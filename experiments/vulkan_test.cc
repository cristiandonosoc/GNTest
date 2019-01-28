// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>
#include <optional>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>

#include <vulkan/vulkan.h>

#include <warhol/assets/assets.h>
#include <warhol/debug/timer.h>
#include <warhol/graphics/common/image.h>
#include <warhol/graphics/common/mesh.h>
#include <warhol/graphics/vulkan/context.h>
#include <warhol/graphics/vulkan/utils.h>
#include <warhol/sdl2/sdl_context.h>
#include <warhol/utils/glm_impl.h>
#include <warhol/utils/log.h>
#include <warhol/utils/types.h>

using namespace warhol;

namespace {

struct ApplicationContext {
  bool running = false;
  bool window_size_changed = false;
};

struct UBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
  {{-0.5f, -0.5f,  0.0f},
    {1.0f,  0.0f,  0.0f},
   {0.0f,  0.0f}},
  {{0.5f, -0.5f,  0.0f},
    {0.0f,  1.0f,  0.0f},
    {1.0f,  0.0f}},
  {{0.5f,  0.5f,  0.0f},
    {0.0f,  0.0f,  1.0f},
    {1.0f,  1.0f}},
  {{-0.5f,  0.5f,  0.0f},
    {1.0f,  1.0f,  1.0f},
    {0.0f,  1.0f}},

  {{-0.5f, -0.5f,  -0.5f},
    {1.0f,  0.0f,  0.0f},
    {0.0f,  0.0f}},
  {{0.5f, -0.5f,  -0.5f},
    {0.0f,  1.0f,  0.0f},
    {1.0f,  0.0f}},
  {{0.5f,  0.5f,  -0.5f},
    {0.0f,  0.0f,  1.0f},
    {1.0f,  1.0f}},
  {{-0.5f,  0.5f,  -0.5f},
    {1.0f,  1.0f,  1.0f},
    {0.0f,  1.0f}},
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
};

inline void Header(const char* header) {
  std::cout << "\n*** " << header
            << " **************************************************************"
               "**************************\n\n";
  std::flush(std::cout);
}

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

  Header("Creating context...");
  if (!vulkan::CreateContext(context))
    return false;

  Header("Set debug callback....");
  if (!vulkan::SetupDebugCall(context, VulkanDebugCall))
    return false;

  Header("Creating surface...");
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(
          sdl_context.get_window(), *context->instance, &surface)) {
    LOG(ERROR) << "Could not create surface: " << SDL_GetError();
  }
  context->surface.Set(context, surface);

  context->device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  if (!vulkan::PickPhysicalDevice(context))
    return false;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(context->physical_device, &properties);
  LOG(INFO) << "Picked physical device: " << properties.deviceName;

  Header("Creating a logical device...");
  if (!vulkan::CreateLogicalDevice(context))
    return false;

  Header("Initializing resource management...");
  if (!vulkan::InitResourceManagement(context))
    return false;

  Header("Creating a swap chain...");
  Pair<uint32_t> screen_size = {(uint32_t)sdl_context.width(),
                                (uint32_t)sdl_context.height()};
  if (!vulkan::CreateSwapChain(context, screen_size))
    return false;

  Header("Creating image views...");
  if (!vulkan::CreateImageViews(context))
    return false;

  Header("Creating a command pool for each framebuffer...");
  if (!vulkan::CreateCommandPool(context))
    return false;

  Header("Creating depth buffers...");
  if (!vulkan::CreateDepthResources(context))
    return false;

  Header("Creating a render pass...");
  if (!vulkan::CreateRenderPass(context))
    return false;

  Header("Creating descriptor set layout...");
  if (!vulkan::CreateDescriptorSetLayout(context))
    return false;

  Header("Creating the pipeline layout...");
  if (!vulkan::CreatePipelineLayout(context))
    return false;

  Header("Creating a graphics pipeline...");
  context->vert_shader_path = Assets::VulkanShaderPath("demo.vert.spv");
  context->frag_shader_path = Assets::VulkanShaderPath("demo.frag.spv");
  if (!vulkan::CreateGraphicsPipeline(context))
    return false;

  Header("Creating frame buffers...");
  if (!vulkan::CreateFrameBuffers(context))
    return false;

  Header("Loading model...");
  /* const char* model_name = "chalet.obj"; */
  /* auto model = LoadModel(Assets::ModelPath(model_name)); */
  /* if (!model) { */
  /*   LOG(ERROR) << "COuld not load " << model_name; */
  /*   return false; */
  /* } */

  Mesh mesh = {};
  mesh.vertices = vertices;
  mesh.indices = indices;
  /* const float* begin = vertices.data(); */
  /* const float* end = begin + vertices.size(); */

  /* const float* ptr = begin; */
  /* while (begin < end) { */
  /*   const Vertex* v = (const Vertex*)ptr; */
  /*   mesh.vertices.push_back(*v); */
  /*   ptr += (sizeof(Vertex) / sizeof(float)); */
  /* } */

  /* mesh.vertices.resize(vertices.size() / sizeof(Vertex)); */
  /* float* ptr = (float*)mesh.vertices.data(); */
  /* for (float f : vertices) { */
  /*   *ptr++ = f; */
  /* } */

  LOG(DEBUG) << "Loading model. Size: " << ToKilobytes(mesh.data_size())
             << " KBs.";

  if (!vulkan::LoadModel(context, mesh))
    return false;

  Header("Setting up UBO...");
  if (!vulkan::SetupUBO(context, sizeof(UBO)))
    return false;

  Image image = Image::Create2DImageFromPath(Assets::TexturePath("chalet.jpg"));
  /* image.mip_levels = 2; */


#if 0
  Image image = {};
  image.width = 1;
  image.height = 1;
  image.channels = 4;
  image.data_size = 1 * 1 * 4;
  image.data = (uint8_t*)malloc(image.data_size);
  image.free_function = free;
  image.type = Image::Type::k2D;
  image.format = Image::Format::kRGBA8;

  // Only red.
  (*image.data)[0] = 0;
  (*image.data)[1] = 0;
  (*image.data)[2] = 0xff;
  (*image.data)[3] = 0xff;
#endif
  LOG(DEBUG) << "Loading image. Size: " << ToKilobytes(image.data_size)
             << " KBs.";

  Header("Creating texture buffers...");
  if (!vulkan::CreateTextureBuffers(context, image))
    return false;

  /* Header("Creating imate view..."); */
  /* if (!vulkan::CreateTextureImageView(context, image)) */
  /*   return false; */

  Header("Creating texture sampler...");
  if (!vulkan::CreateTextureSampler(context, image))
    return false;

  Header("Creating descriptor sets...");
  if (!vulkan::CreateDescriptorSets(context))
    return false;

  Header("Creating command buffers....");
  if (!vulkan::CreateCommandBuffers(context))
    return false;

  Header("Creagin synchronization objects...");
  if (!vulkan::CreateSyncObjects(context))
    return false;

  LOG(INFO) << "Vulkan context creation successful!";
  return true;
}

bool Update(const SDLContext& context, UBO* ubo) {
  float time = context.seconds();
  ubo->model = glm::rotate(
      glm::mat4{1.0f}, time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
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
               const UBO& ubo,
               ApplicationContext* app_context,
               vulkan::Context* vk_context) {
  int current_frame = vk_context->current_frame;
  if (!VK_CALL(vkWaitForFences, *vk_context->device, 1,
               &vk_context->in_flight_fences[current_frame].value(),
               VK_TRUE, UINT64_MAX)) {
    return false;
  }

  uint32_t image_index = 0;
  VkResult res = vkAcquireNextImageKHR(
      *vk_context->device,
      *vk_context->swap_chain,
      UINT64_MAX,  // No timeout.
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

  // Copy the UBO
  UBO* vk_ubo = (UBO*)vk_context->uniform_buffers[image_index].data();
  *vk_ubo = ubo;

  if (!SubmitCommandBuffer(vk_context, image_index))
    return false;

  if (!PresentQueue(sdl_context, vk_context, image_index))
    return false;

  vk_context->current_frame++;
  vk_context->current_frame %= vulkan::Definitions::kMaxFramesInFlight;
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
  vulkan::Context vk_context = {};

  {
    Timer timer = Timer::ManualTimer();

    if (!sdl_context.InitVulkan(SDL_WINDOW_RESIZABLE))
      return 1;

    float timing = timer.End();
    LOG(INFO) << "Created SDL context: " << timing << " ms.";
  }

  {
    Timer timer = Timer::ManualTimer();

    if (!SetupVulkan(sdl_context, &vk_context)) {
      LOG(ERROR) << "Could not setup vulkan. Exiting.";
      return 1;
    }

    float timing = timer.End();
    LOG(INFO) << "Initialized vulkan: " << timing << " ms.";
  }

  LOG(INFO) << "Window size. WIDTH: " << sdl_context.width()
            << ", HEIGHT: " << sdl_context.height();

  // We know that the uniform memory object is mapped.
  UBO ubo = {};
  ubo.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0.0f, 0.0f, 0.1f});
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              sdl_context.width() / (float)sdl_context.height(),
                              0.1f, 100.f);

  // GLM was thought with OpenGL in mind.
  ubo.proj[1][1] *= -1;

  LOG(INFO) << "Framebuffer count: " << vk_context.frame_buffers.size();

  InputState input = InputState::Create();
  app_context.running = true;
  while (app_context.running) {
    if (auto [events, event_count] = sdl_context.NewFrame(&input);
        events != nullptr) {
      HandleSDLEvents(&app_context, &vk_context, events, event_count);
    }

    if (input.keys_up[GET_KEY(Escape)])
      break;

    if (!Update(sdl_context, &ubo))
      break;

    if (!DrawFrame(sdl_context, ubo, &app_context, &vk_context)) {
      LOG(ERROR) << "Error drawing with vulkan. Exiting.";
      break;
    }

    SDL_Delay(10);
  }

  if (!VK_CALL(vkDeviceWaitIdle, *vk_context.device)) {
    LOG(ERROR) << "Could not wait on device. Aborting.";
    exit(1);
  }

}
