// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>

#include <vulkan/vulkan.h>
#include <warhol/assets/assets.h>
#include <warhol/utils/log.h>
#include <warhol/graphics/vulkan/context.h>
#include <warhol/graphics/vulkan/utils.h>
#include <warhol/sdl2/sdl_context.h>

#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>

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
  context->surface = surface;
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

  if (!vulkan::CreateGraphicsPipeline(
          context,
          Assets::VulkanShaderPath("demo.vert.spv"),
          Assets::VulkanShaderPath("demo.frag.spv"))) {
    return false;
  }
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

  if (!vulkan::CreateSemaphores(context))
    return false;
  LOG(INFO) << "Created the semaphores.";

  return true;
}

bool SubmitCommandBuffer(vulkan::Context* context, uint32_t image_index) {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Which semaphores to wait for.
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &context->image_available.value();
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &context->command_buffers[image_index];

  // Which semaphores to signal after we're done.
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &context->render_finished.value();

  return VK_CALL(vkQueueSubmit, context->graphics_queue, 1, &submit_info,
                 nullptr);
}

bool DrawFrame(vulkan::Context* context) {
  uint32_t image_index = 0;
  vkAcquireNextImageKHR(*context->device, *context->swap_chain,
                        std::numeric_limits<uint64_t>::max(),  // No timeout.
                        *context->image_available,
                        VK_NULL_HANDLE,
                        &image_index);

  if (!SubmitCommandBuffer(context, image_index))
    return false;

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Semaphores to wait for.
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &context->render_finished.value();

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &context->swap_chain.value();
  present_info.pImageIndices = &image_index;

  present_info.pResults = nullptr;  // Optional, as we only submit one image.

  if (!VK_CALL(vkQueuePresentKHR, context->present_queue, &present_info))
    return false;

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

    SDL_Delay(100);
  }

  if (!VK_CALL(vkDeviceWaitIdle, *context.device)) {
    LOG(ERROR) << "Could not wait on device. Aborting.";
    exit(1);
  }

}
