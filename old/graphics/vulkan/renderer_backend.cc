// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/renderer_backend.h"

#include <iostream>


#include "warhol/assets/assets.h"
#include "warhol/scene/camera.h"
#include "warhol/graphics/common/image.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/render_command.h"
#include "warhol/graphics/renderer.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/renderer_backend_impl.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/window/window_manager.h"
#include "warhol/utils/glm_impl.h"

namespace warhol {
namespace vulkan {

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<RendererBackend> CreateSDLVulkanRenderer() {
  return std::make_unique<VulkanRendererBackend>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeRendererBackendFactory(RendererBackend::Type::kVulkan,
                                   CreateSDLVulkanRenderer);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

// VulkanRendererBackend -------------------------------------------------------

VulkanRendererBackend::VulkanRendererBackend()
    : RendererBackend(Type::kVulkan) {}
VulkanRendererBackend::~VulkanRendererBackend() {
  if (valid())
    Shutdown();
}

namespace {

// VulkanBackendInit -----------------------------------------------------------

void VulkanBackendInit(VulkanRendererBackend* vulkan) {
  WindowManager* window = vulkan->renderer->window;
  VulkanBackendInitImpl(vulkan, window);
}

// VulkanBackendShutdown -------------------------------------------------------

void VulkanBackendShutdown(VulkanRendererBackend* vulkan) {
  ASSERT(vulkan->valid());

  vulkan::Context* context = vulkan->context.get();
  ASSERT(context);

  VK_CHECK(vkDeviceWaitIdle, *context->device);

  // Reset vulkan renderer. This will free all resources.
  vulkan->pipeline = {};
  vulkan->context.reset();
  vulkan->renderer = nullptr;
}

// VulkanBackendExecuteCommands ------------------------------------------------

void VulkanBackendExecuteCommands(VulkanRendererBackend* vulkan,
                                  RenderCommand* commands,
                                  size_t command_count) {
  ASSERT(vulkan->valid());
  ASSERT(command_count > 0);
  ASSERT(commands);

  VulkanBackendStartFrame(vulkan);

  RenderCommand* current_command = commands;
  for (size_t i = 0; i < command_count; i++) {
    switch (current_command->type) {
      case RenderCommand::Type::kMesh:
        VulkanBackendDrawMesh(vulkan, current_command);
        break;
      case RenderCommand::Type::kLast:
        NOT_REACHED("Invalid RenderCommand Type (Last).");
        break;
    }

    current_command++;
  }

  VulkanBackendEndFrame(vulkan);

  NOT_IMPLEMENTED();
}

// VulkanBackendDrawFrame ------------------------------------------------------

void SubmitCommandBuffer(VulkanRendererBackend* vulkan, uint32_t image_index) {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Which semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *vulkan->pipeline.image_available_semaphores[vulkan->current_frame],
  };
  submit_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  submit_info.pWaitSemaphores = wait_semaphores;
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.pWaitDstStageMask = wait_stages;

  VkCommandBuffer command_buffers[] = {
    vulkan->pipeline.command_buffers[image_index],
  };
  submit_info.commandBufferCount = ARRAY_SIZE(command_buffers);
  submit_info.pCommandBuffers = command_buffers;

  uint32_t current_frame = vulkan->current_frame;

  // Which semaphores to signal after we're done.
  VkSemaphore signal_semaphores[] = {
    *vulkan->pipeline.render_finished_semaphores[current_frame],
  };
  submit_info.signalSemaphoreCount = ARRAY_SIZE(signal_semaphores);
  submit_info.pSignalSemaphores = signal_semaphores;

  VK_CHECK(vkQueueSubmit, vulkan->context->graphics_queue, 1, &submit_info,
           *vulkan->pipeline.in_flight_fences[current_frame]);
}

void PresentQueue(WindowManager* window, VulkanRendererBackend* vulkan,
                  uint32_t image_index) {
  Context* context = vulkan->context.get();

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  uint32_t current_frame = vulkan->current_frame;

  // Semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *vulkan->pipeline.render_finished_semaphores[current_frame],
  };
  present_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  present_info.pWaitSemaphores = wait_semaphores;

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &context->swap_chain.value();
  present_info.pImageIndices = &image_index;

  present_info.pResults = nullptr;  // Optional, as we only submit one image.

  // When presenting a queue, we can be notified that the queue is out of date
  // (eg. the surface changed size) and we need to recreate the swapchain for
  // this.
  VkResult res = vkQueuePresentKHR(context->present_queue, &present_info);
  if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
    Pair<uint32_t> screen_size = {(uint32_t)window->width,
                                  (uint32_t)window->height};
    LOG(INFO) << "Recreating swap chain to " << screen_size.ToString();
    VulkanBackendRecreateSwapChain(vulkan, screen_size);
  } else if (res != VK_SUCCESS) {
    // Suboptimal is considered a success state, as rendering can continue.
    LOG(ERROR) << "Error presenting the queue: " << vulkan::EnumToString(res);
    NOT_REACHED("See logs.");
  }
}

void VulkanBackendDrawFrame(VulkanRendererBackend* vulkan, Camera* camera) {
  ASSERT(vulkan->valid());
  WindowManager* window = vulkan->renderer->window;
  Context* context = vulkan->context.get();
  ASSERT(context);

  int current_frame = vulkan->current_frame;
  VK_CHECK(vkWaitForFences, *context->device, 1,
           &vulkan->pipeline.in_flight_fences[current_frame].value(), VK_TRUE,
           UINT64_MAX);

  uint32_t image_index = 0;
  VK_CHECK(vkAcquireNextImageKHR, *context->device, *context->swap_chain,
           UINT64_MAX,  // No timeout.
           *vulkan->pipeline.image_available_semaphores[current_frame],
           nullptr,
           &image_index);

  EmptyGarbage(&context->allocator);
  Flush(&context->staging_manager);

  VK_CHECK(vkResetFences, *context->device, 1,
           &vulkan->pipeline.in_flight_fences[current_frame].value());

  // Copy the UBO
  float time = window->seconds;
  UBO* vk_ubo = (UBO*)vulkan->pipeline.uniform_buffers[image_index].data();
  *vk_ubo = {};
  vk_ubo->model = glm::rotate(
      glm::mat4{1.0f}, time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
  vk_ubo->view = camera->view;
  vk_ubo->proj = camera->projection;
  // GLM was thought with OpenGL in mind.
  vk_ubo->proj[1][1] *= -1;

  SubmitCommandBuffer(vulkan, image_index);
  PresentQueue(window, vulkan, image_index);

  vulkan->current_frame++;
  vulkan->current_frame %= vulkan::Definitions::kMaxFramesInFlight;
}

// LoadMesh --------------------------------------------------------------------

MemoryBacked<VkBuffer> CreateDeviceBuffer(Context* context, VkDeviceSize size) {
  // Create the device local memory and copy the memory to it.
  AllocBufferConfig alloc_config = {};
  alloc_config.size = size;
  alloc_config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  alloc_config.memory_usage = MemoryUsage::kGPUOnly;
  return AllocBuffer(context, &alloc_config);
}

void VulkanBackendLoadMesh(VulkanRendererBackend* vulkan, Mesh* mesh) {
  Context* context = vulkan->context.get();
  ASSERT(context);

  ASSERT(!mesh->loaded());

  VulkanRendererBackend::LoadedMesh loaded_mesh = {};

  // **** Vertices ****
  {
    VkDeviceSize size = mesh->vertices.size() * sizeof(mesh->vertices[0]);
    StageToken token = Stage(&context->staging_manager, size);
    CopyIntoStageToken(&token, (void*)mesh->vertices.data(), size);

    // Create the device local memory and copy the memory to it.
    MemoryBacked<VkBuffer> vertices_memory = CreateDeviceBuffer(context, size);
    ASSERT(vertices_memory.has_value());
    CopyStageTokenToBuffer(&token, *vertices_memory.handle, 0);

    loaded_mesh.vertices = std::move(vertices_memory);
  }

  // **** Indices ****
  {
    VkDeviceSize size = mesh->indices.size() * sizeof(mesh->indices[0]);
    StageToken token = Stage(&context->staging_manager, size);
    CopyIntoStageToken(&token, (void*)mesh->indices.data(), size);

    // Create the device local memory and copy the memory to it.
    MemoryBacked<VkBuffer> indices_memory = CreateDeviceBuffer(context, size);
    ASSERT(indices_memory.has_value());
    CopyStageTokenToBuffer(&token, *indices_memory.handle, 0);

    loaded_mesh.indices = std::move(indices_memory);
  }

  uint64_t mesh_id = vulkan->next_loaded_mesh_id++;
  mesh->loaded_token = mesh_id;
  loaded_mesh.mesh = mesh;
  vulkan->loaded_meshes[mesh_id] = std::move(loaded_mesh);
}

void VulkanBackendUnloadMesh(VulkanRendererBackend* vulkan, Mesh* mesh) {
  ASSERT(mesh->loaded());

  auto it = vulkan->loaded_meshes.find(mesh->loaded_token);
  ASSERT(it != vulkan->loaded_meshes.end());

  auto& loaded_mesh = it->second;
  ASSERT(loaded_mesh.mesh == mesh);

  vulkan->loaded_meshes.erase(it);
}

}  // namespace

// Calling the implementations -------------------------------------------------

void VulkanRendererBackend::Init(Renderer* renderer) {
  this->renderer = renderer;
  VulkanBackendInit(this);
}

void VulkanRendererBackend::Shutdown() {
  if (valid())
    VulkanBackendShutdown(this);
}

void VulkanRendererBackend::ExecuteCommands(RenderCommand* commands,
                                            size_t count) {
  VulkanBackendExecuteCommands(this, commands, count);
}

void VulkanRendererBackend::DrawFrame(Camera* camera) {
  VulkanBackendDrawFrame(this, camera);
}

void VulkanRendererBackend::LoadMesh(Mesh* mesh) {
  VulkanBackendLoadMesh(this, mesh);
}
void VulkanRendererBackend::UnloadMesh(Mesh* mesh) {
  VulkanBackendUnloadMesh(this, mesh);
}

}  // namespace vulkan
}  // namespace warhol
