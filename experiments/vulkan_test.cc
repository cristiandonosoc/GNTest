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

struct ApplicationContext {
  bool running = false;
  bool window_size_changed = false;
};

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

bool PresentQueue(const SDLContext& sdl_context,
                  vulkan::Context* vk_context,
                  uint32_t image_index) {
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Semaphores to wait for.
  VkSemaphore wait_semaphores[] = {
    *vk_context->render_finished_semaphores[vk_context->current_frame],
  };
  present_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
  present_info.pWaitSemaphores = wait_semaphores;

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &vk_context->swap_chain.value();
  present_info.pImageIndices = &image_index;

  present_info.pResults = nullptr;  // Optional, as we only submit one image.

  // When presenting a queue, we can be notified that the queue is out of date
  // (eg. the surface changed size) and we need to recreate the swapchain for
  // this.
  VkResult res = vkQueuePresentKHR(vk_context->present_queue, &present_info);
  if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
    Pair<uint32_t> screen_size = {(uint32_t)sdl_context.width(),
                                  (uint32_t)sdl_context.height()};

    LOG(INFO) << "Recreating swap chain to " << screen_size.ToString();
    return RecreateSwapChain(vk_context, screen_size);
  } else if (res != VK_SUCCESS) {
    // Suboptimal is considered a success state, as rendering can continue.
    LOG(ERROR) << "Error presenting the queue: " << vulkan::EnumToString(res);
    return false;
  }
  return true;
}

bool DrawFrame(const SDLContext& sdl_context,
               ApplicationContext* app_context,
               vulkan::Context* vk_context) {
  int current_frame = vk_context->current_frame;
  if (!VK_CALL(vkWaitForFences, *vk_context->device, 1,
               &vk_context->in_flight_fences[current_frame].value(),
               VK_TRUE, Limits::kUint64Max)) {
    return false;
  }

  uint32_t image_index = 0;
  VkResult res = vkAcquireNextImageKHR(
      *vk_context->device,
      *vk_context->swap_chain,
      Limits::kUint64Max,  // No timeout.
      *vk_context->image_available_semaphores[current_frame],
      VK_NULL_HANDLE,
      &image_index);

  // If vulkan or our app tells us the window/surface size has changed, we
  // recreate the swap chain.
  if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR ||
      app_context->window_size_changed) {
    app_context->window_size_changed = false;
    Pair<uint32_t> screen_size = {(uint32_t)sdl_context.width(),
                                  (uint32_t)sdl_context.height()};
    LOG(INFO) << "Recreating swap chain to " << screen_size.ToString();
    return RecreateSwapChain(vk_context, screen_size);
  } else if (res != VK_SUCCESS) {
    // Suboptimal is considered a success state, as rendering can continue.
    LOG(ERROR) << "Error presenting the queue: " << vulkan::EnumToString(res);
    return false;
  }

  if (!VK_CALL(vkResetFences, *vk_context->device, 1,
              &vk_context->in_flight_fences[current_frame].value())) {
    return false;
  }

  if (!SubmitCommandBuffer(vk_context, image_index))
    return false;

  if (!PresentQueue(sdl_context, vk_context, image_index))
    return false;

  vk_context->current_frame++;
  vk_context->current_frame %= vk_context->max_frames_in_flight;
  return true;
}

void HandleSDLEvents(ApplicationContext* app_context,
                     vulkan::Context* vk_context,
                     SDLContext::Event* events,
                     size_t event_count) {
  (void)vk_context;
  for (size_t i = 0; i < event_count; i++) {
    SDLContext::Event& event = events[i];
    if (event == SDLContext::Event::kQuit) {
      app_context->running = false;
      break;
    }
  }
}

}  // namespace

int main() {
  ApplicationContext app_context = {};

  SDLContext sdl_context = {};
  if (!sdl_context.InitVulkan(SDL_WINDOW_RESIZABLE))
    return 1;
  LOG(INFO) << "Created SDL context.";
  LOG(INFO) << "Window size. WIDTH: " << sdl_context.width()
            << ", HEIGHT: " << sdl_context.height();

  vulkan::Context vk_context;
  if (!SetupVulkan(sdl_context, &vk_context)) {
    LOG(ERROR) << "Could not setup vulkan. Exiting.";
    return 1;
  }

  InputState input = InputState::Create();
  app_context.running = true;
  while (app_context.running) {
    if (auto [events, event_count] = sdl_context.NewFrame(&input);
        events != nullptr) {
      HandleSDLEvents(&app_context, &vk_context, events, event_count);
    }

    if (input.keys_up[GET_KEY(Escape)])
      break;

    if (!DrawFrame(sdl_context, &app_context, &vk_context)) {
      LOG(ERROR) << "Error drawing with vulkan. Exiting.";
      break;
    }

    SDL_Delay(16);
  }

  if (!VK_CALL(vkDeviceWaitIdle, *vk_context.device)) {
    LOG(ERROR) << "Could not wait on device. Aborting.";
    exit(1);
  }

}
