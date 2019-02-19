// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/renderer_backend.h"

#include <iostream>


#include "warhol/assets/assets.h"
#include "warhol/scene/camera.h"
#include "warhol/graphics/common/image.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/renderer_backend.h"
#include "warhol/graphics/renderer.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/renderer_backend_impl.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/window/window_manager.h"
#include "warhol/utils/glm_impl.h"

namespace warhol {
namespace vulkan {

VulkanRendererBackend::VulkanRendererBackend() = default;
VulkanRendererBackend::~VulkanRendererBackend() = default;

namespace {

void InitRendererBackend(RendererBackend*);
void ShutdownRendererBackend(RendererBackend*);
void ExecuteCommands(RendererBackend*);
void DrawFrame(RendererBackend*, Camera*);

// Declaration of RendererBackend::Interface -----------------------------------

struct SetupInterface {
  SetupInterface() {
    RendererBackend::Interface interface;

    interface.Init = InitRendererBackend;
    interface.Shutdown = ShutdownRendererBackend;
    interface.ExecuteCommands = ExecuteCommands;
    interface.DrawFrame = DrawFrame;
    SetRendererBackendInterfaceTemplate(RendererBackend::Type::kVulkan,
                                        std::move(interface));
  }
};

// Setup the renderer backend once.
SetupInterface setup_interface;

// InitRendererBackend ---------------------------------------------------------

void InitRendererBackend(RendererBackend* backend) {
  ASSERT(!backend->valid());
  VulkanRendererBackend* vulkan = new VulkanRendererBackend();
  backend->data = vulkan;
  WindowManager* window = backend->renderer->window;
  InitVulkanRendererBackend(vulkan, window);
}

// ExecuteCommands -------------------------------------------------------------

void ExecuteCommands(RendererBackend* backend) {
  ASSERT(backend->valid());
  NOT_IMPLEMENTED();
}

// ShutdownRendererBackend -----------------------------------------------------

void ShutdownRendererBackend(RendererBackend* backend) {
  ASSERT(backend->valid());

  auto* vulkan = (VulkanRendererBackend*)backend->data;
  vulkan::Context* vk_context = vulkan->context.get();
  ASSERT(vk_context);

  if (!VK_CALL(vkDeviceWaitIdle, *vk_context->device))
    NOT_REACHED("Could not wait on device. Aborting.");

  // Reset vulkan renderer. This will free all resources.
  vulkan->context.reset();
  delete vulkan;

  Clear(backend);
}

// DrawFrame -------------------------------------------------------------------

void SubmitCommandBuffer(VulkanRendererBackend* vulkan, uint32_t image_index) {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Which semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *vulkan->image_available_semaphores[vulkan->current_frame],
  };
  submit_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  submit_info.pWaitSemaphores = wait_semaphores;
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.pWaitDstStageMask = wait_stages;

  VkCommandBuffer command_buffers[] = {
    vulkan->command_buffers[image_index],
  };
  submit_info.commandBufferCount = ARRAY_SIZE(command_buffers);
  submit_info.pCommandBuffers = command_buffers;

  // Which semaphores to signal after we're done.
  VkSemaphore signal_semaphores[] = {
    *vulkan->render_finished_semaphores[vulkan->current_frame],
  };
  submit_info.signalSemaphoreCount = ARRAY_SIZE(signal_semaphores);
  submit_info.pSignalSemaphores = signal_semaphores;

  VK_CHECK(vkQueueSubmit, vulkan->context->graphics_queue, 1, &submit_info,
           *vulkan->in_flight_fences[vulkan->current_frame]);
}

void PresentQueue(WindowManager* window, VulkanRendererBackend* vulkan,
                  uint32_t image_index) {
  Context* context = vulkan->context.get();

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *vulkan->render_finished_semaphores[vulkan->current_frame],
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
    RecreateSwapChain(vulkan, screen_size);
  } else if (res != VK_SUCCESS) {
    // Suboptimal is considered a success state, as rendering can continue.
    LOG(ERROR) << "Error presenting the queue: " << vulkan::EnumToString(res);
    NOT_REACHED("See logs.");
  }
}

void DrawFrame(RendererBackend* backend, Camera* camera) {
  ASSERT(backend->valid());
  WindowManager* window= backend->renderer->window;
  auto* vulkan = (VulkanRendererBackend*)backend->data;
  Context* context = vulkan->context.get();

  int current_frame = vulkan->current_frame;
  VK_CHECK(vkWaitForFences, *context->device, 1,
           &vulkan->in_flight_fences[current_frame].value(), VK_TRUE,
           UINT64_MAX);

  uint32_t image_index = 0;
  VK_CHECK(vkAcquireNextImageKHR, *context->device, *context->swap_chain,
           UINT64_MAX,  // No timeout.
           *vulkan->image_available_semaphores[current_frame],
           nullptr,
           &image_index);

  EmptyGarbage(&context->allocator);
  Flush(&context->staging_manager);

  VK_CHECK(vkResetFences, *context->device, 1,
           &vulkan->in_flight_fences[current_frame].value());

  // Copy the UBO
  float time = window->seconds;
  UBO* vk_ubo = (UBO*)vulkan->uniform_buffers[image_index].data();
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

}  // namespace

}  // namespace vulkan
}  // namespace warhol
