// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>


#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>

#include <vulkan/vulkan.h>

#include <warhol/assets/assets.h>
#include <warhol/graphics/vulkan/context.h>
#include <warhol/graphics/vulkan/utils.h>
#include <warhol/sdl2/sdl_context.h>
#include <warhol/utils/log.h>
#include <warhol/utils/types.h>

using namespace warhol;

namespace {

// TODO: Setup a better debug call.
static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCall(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data) {
  (void)user_data;
  if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    return VK_FALSE;

  // Message is important enough to show
  std::cout << "*VULKAN* [" << vulkan::EnumToString(severity) << "]["
            << vulkan::EnumToString(type) << "] " << callback_data->pMessage
            << std::endl;
  std::flush(std::cout);

  return VK_FALSE;
}

bool SetupVulkan(const SDLContext& sdl_context, vulkan::Context* context) {
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions,
                    sdl_context.get_window(),
                    context->extensions);
#ifndef NDEBUG
  vulkan::AddDebugExtensions(&context->extensions);
#endif
  if (!vulkan::CheckExtensions(context->extensions))
    return false;

#ifndef NDEBUG
  context->validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
  if (!vulkan::CheckValidationLayers(context->validation_layers))
    return false;

  if (!vulkan::CreateContext(context))
    return false;
  LOG(INFO) << "Created context.";

  if (!vulkan::SetupDebugCall(context, VulkanDebugCall))
    return false;
  LOG(INFO) << "Set debug callback.";

  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(
          sdl_context.get_window(), *context->instance, &surface)) {
    LOG(ERROR) << "Could not create surface: " << SDL_GetError();
  }
  context->surface.Set(context, surface);
  LOG(INFO) << "Created a surface.";

  context->device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  if (!vulkan::PickPhysicalDevice(context))
    return false;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(context->physical_device, &properties);
  LOG(INFO) << "Picked physical device: " << properties.deviceName;

  if (!vulkan::CreateLogicalDevice(context))
    return false;
  LOG(INFO) << "Created a logical device.";

  Pair<uint32_t> screen_size = {(uint32_t)sdl_context.width(),
                                (uint32_t)sdl_context.height()};
  if (!vulkan::CreateSwapChain(context, screen_size))
    return false;
  LOG(INFO) << "Created a swap chain.";

  if (!vulkan::CreateImageViews(context))
    return false;
  LOG(INFO) << "Created image views.";

  if (!vulkan::CreateRenderPass(context))
    return false;
  LOG(INFO) << "Created a render pass.";

  if (!vulkan::CreatePipelineLayout(context))
    return false;
  LOG(INFO) << "Created the pipeline layout.";

  context->vert_shader_path = Assets::VulkanShaderPath("demo.vert.spv");
  context->frag_shader_path = Assets::VulkanShaderPath("demo.frag.spv");
  if (!vulkan::CreateGraphicsPipeline(context))
    return false;
  LOG(INFO) << "Created a graphics pipeline.";

  if (!vulkan::CreateFrameBuffers(context))
    return false;
  LOG(INFO) << "Created frame buffers.";

  if (!vulkan::CreateCommandPool(context))
    return false;
  LOG(INFO) << "Created a command pool for each framebuffer.";

  if (!vulkan::CreateCommandBuffers(context))
    return false;
  LOG(INFO) << "Created some command buffers.";

  if (!vulkan::CreateSyncObjects(context))
    return false;
  LOG(INFO) << "Created synchronization objects.";

  return true;
}

bool SubmitCommandBuffer(vulkan::Context* context, uint32_t image_index) {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Which semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *context->image_available_semaphores[context->current_frame],
  };
  submit_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  submit_info.pWaitSemaphores = wait_semaphores;
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.pWaitDstStageMask = wait_stages;

  VkCommandBuffer command_buffers[] = {
    context->command_buffers[image_index],
  };
  submit_info.commandBufferCount = ARRAY_SIZE(command_buffers);
  submit_info.pCommandBuffers = command_buffers;

  // Which semaphores to signal after we're done.
  VkSemaphore signal_semaphores[] = {
    *context->render_finished_semaphores[context->current_frame],
  };
  submit_info.signalSemaphoreCount = ARRAY_SIZE(signal_semaphores);
  submit_info.pSignalSemaphores = signal_semaphores;

  return VK_CALL(vkQueueSubmit, context->graphics_queue, 1, &submit_info,
                 *context->in_flight_fences[context->current_frame]);
}

bool DrawFrame(vulkan::Context* context) {
  if (!VK_CALL(vkWaitForFences, *context->device, 1,
               &context->in_flight_fences[context->current_frame].value(),
               VK_TRUE, Limits::kUint64Max) ||
      !VK_CALL(vkResetFences, *context->device, 1,
               &context->in_flight_fences[context->current_frame].value())) {
    return false;
  }

  uint32_t image_index = 0;
  vkAcquireNextImageKHR(
      *context->device,
      *context->swap_chain,
      Limits::kUint64Max,  // No timeout.
      *context->image_available_semaphores[context->current_frame],
      VK_NULL_HANDLE,
      &image_index);

  if (!SubmitCommandBuffer(context, image_index))
    return false;

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *context->render_finished_semaphores[context->current_frame],
  };
  present_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  present_info.pWaitSemaphores = wait_semaphores;

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &context->swap_chain.value();
  present_info.pImageIndices = &image_index;

  present_info.pResults = nullptr;  // Optional, as we only submit one image.

  if (!VK_CALL(vkQueuePresentKHR, context->present_queue, &present_info))
    return false;

  context->current_frame++;
  context->current_frame %= context->max_frames_in_flight;
  return true;
}

}  // namespace

int main() {
  SDLContext sdl_context;
  if (!sdl_context.InitVulkan(0))
    return 1;
  LOG(INFO) << "Created SDL context.";

  LOG(INFO) << "Window size. WIDTH: " << sdl_context.width()
            << ", HEIGHT: " << sdl_context.height();

  vulkan::Context context;
  if (!SetupVulkan(sdl_context, &context)) {
    LOG(ERROR) << "Could not setup vulkan. Exiting.";
    return 1;
  }

  InputState input = InputState::Create();
  bool running = true;
  while (running) {
    SDLContext::EventAction action = sdl_context.NewFrame(&input);
    if (action == SDLContext::EventAction::kQuit)
      break;

    if (input.keys_up[GET_KEY(Escape)])
      break;

    if (!DrawFrame(&context)) {
      LOG(ERROR) << "Error drawing with vulkan. Exiting.";
      break;
    }

    SDL_Delay(16);
  }

  if (!VK_CALL(vkDeviceWaitIdle, *context.device)) {
    LOG(ERROR) << "Could not wait on device. Aborting.";
    exit(1);
  }

}
