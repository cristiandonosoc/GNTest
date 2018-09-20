// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string.h>

#include <limits>
#include <set>

#include <SDL2/SDL_vulkan.h>

#include "utils/file.h"
#include "utils/log.h"
#include "utils/string.h"
#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

// TODO: Setup a better debug call.
static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCall(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data) {
  (void)severity;
  (void)type;
  (void)user_data;
  LOG(INFO) << "Validation layer message: " << callback_data->pMessage;

  return VK_FALSE;
}

VulkanContext::~VulkanContext() {
  if (command_pool != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying command pool";
    vkDestroyCommandPool(logical_device.handle, command_pool, nullptr);
  }

  for (auto frame_buffer : frame_buffers) {
    if (frame_buffer != VK_NULL_HANDLE) {
      LOG(INFO) << "Destroying frame buffer";
      vkDestroyFramebuffer(logical_device.handle, frame_buffer, nullptr);
    }
  }

  if (pipeline.pipeline != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying Graphics Pipeline";
    vkDestroyPipeline(logical_device.handle, pipeline.pipeline, nullptr);
  }

  for (const auto shader_module : pipeline.shader_modules) {
    if (shader_module != VK_NULL_HANDLE) {
      LOG(INFO) << "Destroying Shader Module";
      vkDestroyShaderModule(logical_device.handle, shader_module,
                            nullptr);
    }
  }

  // We destroy elements backwards from allocation.
  if (pipeline.layout != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying pipeline layout";
    vkDestroyPipelineLayout(logical_device.handle, pipeline.layout, nullptr);
  }

  if (pipeline.render_pass != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying render pass";
    vkDestroyRenderPass(logical_device.handle, pipeline.render_pass, nullptr);
  }

  for (VkImageView& image_view : swap_chain.image_views) {
    if (image_view != VK_NULL_HANDLE) {
      LOG(INFO) << "Destroying image";
      vkDestroyImageView(logical_device.handle, image_view, nullptr);
    }
  }

  if (swap_chain.handle != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying Swap chain";
    vkDestroySwapchainKHR(logical_device.handle, swap_chain.handle, nullptr);
  }

  if (logical_device.handle != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying logical device";
    vkDestroyDevice(logical_device.handle, nullptr);
  }

  if (surface != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying surface";
    vkDestroySurfaceKHR(instance.handle, surface, nullptr);
  }

  if (debug_messenger != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying debug messenger";
    DestroyDebugUtilsMessengerEXT(instance.handle, debug_messenger, nullptr);
  }

  if (instance.handle != VK_NULL_HANDLE) {
    LOG(INFO) << "Destroying instance";
    vkDestroyInstance(instance.handle, nullptr);
  }
}

// Declare the stages
namespace {

// Utils -----------------------------------------------------------------------
// Adds all the required extensions from SDL and beyond.
Status AddInstanceExtensions(SDL_Window*, VulkanContext*);
Status AddInstanceValidationLayers(VulkanContext*);
Status CheckRequiredLayers(const std::vector<const char*>&);

VulkanContext::SwapChain
GetSwapChainProperties(const VulkanContext&,
                       const VulkanContext::PhysicalDevice&);
bool
IsSuitablePhysicalDevice(const VulkanContext&,
                         const VulkanContext::PhysicalDevice&,
                         const std::vector<const char*>& extensions);

VkSurfaceFormatKHR
GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
VkPresentModeKHR
GetBestPresentMode(const std::vector<VkPresentModeKHR>&);
VkExtent2D
ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

Status
CreateShaderModule(VulkanContext*, const std::vector<char>& src,
                   VkShaderModule* out);

}  // namespace

#define RETURN_IF_ERROR(status, call) \
  LOG(INFO) << "CALLING: " #call;     \
  status = (call);                    \
  if (!status.ok())                   \
    return status;

Status
VulkanContext::Init(SDL_Window* window) {
  Status status;
  // Instance.
  RETURN_IF_ERROR(status, SetupInstance(window));
  RETURN_IF_ERROR(status, SetupDebugMessenger());
  RETURN_IF_ERROR(status, SetupSurface(window));
  RETURN_IF_ERROR(status, SetupPhysicalDevice());
  RETURN_IF_ERROR(status, SetupLogicalDevice());
  RETURN_IF_ERROR(status, SetupSwapChain());
  RETURN_IF_ERROR(status, SetupImages());
  RETURN_IF_ERROR(status, SetupRenderPass());
  RETURN_IF_ERROR(status, SetupPipelineLayout());
  RETURN_IF_ERROR(status, SetupGraphicsPipeline());
  RETURN_IF_ERROR(status, SetupFrameBuffers());
  RETURN_IF_ERROR(status, SetupCommandPool());

  return Status::Ok();
}

Status
VulkanContext::SetupInstance(SDL_Window* window) {
  // TODO: The extensions and validation layers should be exposed.
  Status status;
  RETURN_IF_ERROR(status, AddInstanceExtensions(window, this));
  RETURN_IF_ERROR(status, AddInstanceValidationLayers(this));

  // Vulkan application info.
  VkApplicationInfo app_info = {};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName        = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_1;

  // The creation info.
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount =
      (uint32_t)instance.extensions.size();
  create_info.ppEnabledExtensionNames = instance.extensions.data();

  Status res = CheckRequiredLayers(instance.validation_layers);
  if (!res.ok())
    return res;

  create_info.enabledLayerCount =
      (uint32_t)instance.validation_layers.size();
  create_info.ppEnabledLayerNames = instance.validation_layers.data();

  // Finally create the VkInstance.
  VkResult result =
      vkCreateInstance(&create_info, nullptr, &instance.handle);
  VK_RETURN_IF_ERROR(result);
  return Status::Ok();
}

Status
VulkanContext::SetupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT messenger_info = {};
  messenger_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messenger_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  messenger_info.pfnUserCallback = VulkanDebugCall;
  messenger_info.pUserData = nullptr;

  Status status = CreateDebugUtilsMessengerEXT(instance.handle, &messenger_info,
                                               nullptr, &debug_messenger);
  return status;
}

Status
VulkanContext::SetupSurface(SDL_Window* window) {
  if (!SDL_Vulkan_CreateSurface(window, instance.handle, &surface))
    return Status("Could not create surface: %s\n", SDL_GetError());
  return Status::Ok();
}

Status
VulkanContext::SetupPhysicalDevice() {
  std::vector<VkPhysicalDevice> device_handles;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, instance.handle,
                    device_handles);

  // Enumarate device properties.
  std::vector<VulkanContext::PhysicalDevice> devices;
  devices.reserve(device_handles.size());
  LOG(INFO) << StringPrintf("Found %zu physical devices: ",
                            device_handles.size());
  for (auto& device_handle : device_handles) {
    VulkanContext::PhysicalDevice device;
    device.handle = device_handle;
    vkGetPhysicalDeviceProperties(device.handle, &device.properties);
    vkGetPhysicalDeviceFeatures(device.handle, &device.features);

    printf("--------------------------------------------\n");
    printf("Device Name: %s\n", device.properties.deviceName);
    printf("Type: %s\n", VulkanEnumToString(device.properties.deviceType));
    printf("API Version: %u\n", device.properties.apiVersion);
    printf("Driver Version: %u\n", device.properties.driverVersion);
    printf("Vendor ID: %x\n", device.properties.vendorID);
    printf("Device ID: %x\n", device.properties.deviceID);
    fflush(stdout);

    // We setup the queue families data for each device.
    VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties, device.handle,
                      (device.qf_properties));

    // Get the queues
    int i = 0;
    for (auto& qfp : device.qf_properties) {
      if (qfp.queueCount == 0)
        continue;

      // Get the graphical queue.
      if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        device.graphics_queue_index = i;

      // Get the present queue.
      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device.handle, i, surface,
                                           &present_support);
      if (present_support)
        device.present_queue_index = i;

      i++;
    }

    swap_chain = GetSwapChainProperties(*this, device);

    // We check if this is a suitable device
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!IsSuitablePhysicalDevice(*this, device, extensions)) {
      LOG(INFO) << "Device " << device.properties.deviceName
                << " is not suitable",
          device.properties.deviceName;
      continue;
    }

    device.extensions = std::move(extensions);
    devices.push_back(std::move(device));
  }

  if (devices.empty())
    return Status("No suitable device found");

  // TODO: Find a better heuristic to get the device.
  //       For now we get the first.
  physical_device = devices.front();
  LOG(INFO) << "Selected device: " << physical_device.properties.deviceName;

  return Status::Ok();
}

Status
VulkanContext::SetupLogicalDevice() {
  // The device queues to set.
  float queue_priority = 1.0f;

  // We onlu create unique device queues
  std::set<int> queue_indices = {physical_device.graphics_queue_index,
                                 physical_device.present_queue_index};
  std::vector<VkDeviceQueueCreateInfo> qcreate_infos;
  for (int queue_family : queue_indices) {
    VkDeviceQueueCreateInfo qcreate_info = {};
    qcreate_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qcreate_info.queueFamilyIndex = queue_family;
    qcreate_info.queueCount = 1;
    qcreate_info.pQueuePriorities = &queue_priority;
    qcreate_infos.push_back(std::move(qcreate_info));
  }

  // Setup the logical device features.
  VkDeviceCreateInfo dci = {};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = (uint32_t)qcreate_infos.size();
  dci.pQueueCreateInfos = qcreate_infos.data();
  // Extensions.
  dci.enabledExtensionCount =
      (uint32_t)physical_device.extensions.size();
  dci.ppEnabledExtensionNames = physical_device.extensions.data();
  // Features.
  // For now physical features are disabled.
  /* dci.pEnabledFeatures = &physical_device->features; */
  VkPhysicalDeviceFeatures features = {};
  dci.pEnabledFeatures = &features;
  // Validation layers.
  dci.enabledLayerCount = (uint32_t)instance.validation_layers.size();
  dci.ppEnabledLayerNames = instance.validation_layers.data();

  // Finally create the device.
  VkResult res = vkCreateDevice(physical_device.handle, &dci, nullptr,
                                &logical_device.handle);
  VK_RETURN_IF_ERROR(res);

  // Get the graphics queue.
  vkGetDeviceQueue(logical_device.handle, physical_device.graphics_queue_index,
                   0, &logical_device.graphics_queue);
  // Get the present queue.
  vkGetDeviceQueue(logical_device.handle, physical_device.present_queue_index,
                   0, &logical_device.present_queue);

  return Status::Ok();
}

Status
VulkanContext::SetupSwapChain() {
  uint32_t image_min = swap_chain.capabilites.minImageCount;
  uint32_t image_max = swap_chain.capabilites.maxImageCount;
  uint32_t image_count = image_min + 1;
  if (image_max > 0 && image_count > image_max)
    image_count = image_max;

  swap_chain.format = GetBestSurfaceFormat(swap_chain.formats);
  swap_chain.present_mode = GetBestPresentMode(swap_chain.present_modes);
  swap_chain.extent = ChooseSwapExtent(swap_chain.capabilites);

  VkSwapchainCreateInfoKHR scci = {};
  scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  scci.surface = surface;

  scci.minImageCount = image_count;
  scci.imageFormat = swap_chain.format.format;
  scci.imageColorSpace = swap_chain.format.colorSpace;
  scci.imageExtent = swap_chain.extent;
  scci.imageArrayLayers = 1;
  scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queue_family_indices[] = {
      (uint32_t)physical_device.graphics_queue_index,
      (uint32_t)physical_device.present_queue_index};

  // We check which sharing mode is needed between the command queues.
  if (physical_device.graphics_queue_index !=
      physical_device.present_queue_index) {
    // If graphics and present are different, we have a concurrent management
    // that is simpler.
    scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    scci.queueFamilyIndexCount = 2;
    scci.pQueueFamilyIndices = queue_family_indices;
  } else {
    // If the queues are the same, we use exclusive as there are no queues
    // to coordinate.
    scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    scci.queueFamilyIndexCount = 0;      // Optional
    scci.pQueueFamilyIndices = nullptr;  // Optional
  }

  scci.preTransform = swap_chain.capabilites.currentTransform;
  // Don't blend alpha between images of the swap chain.
  scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  scci.presentMode = swap_chain.present_mode;
  scci.clipped = VK_TRUE;   // Ignore pixels that are ignored.

  // If we need to create a new swap chain, this is a reference to the one we
  // came from.
  scci.oldSwapchain = VK_NULL_HANDLE;

  VkResult res = vkCreateSwapchainKHR(logical_device.handle, &scci, nullptr,
                                      &swap_chain.handle);
  if (res != VK_SUCCESS)
    return Status("Cannot create swap chain: %s", VulkanEnumToString(res));
  return Status::Ok();
}

Status
VulkanContext::SetupImages() {
  // Retrieve the images.
  uint32_t sc_image_count;
  vkGetSwapchainImagesKHR(logical_device.handle, swap_chain.handle,
                          &sc_image_count, nullptr);
  swap_chain.images.resize(sc_image_count);
  vkGetSwapchainImagesKHR(logical_device.handle, swap_chain.handle,
                          &sc_image_count, swap_chain.images.data());

  swap_chain.image_views.reserve(swap_chain.images.size());
  for (size_t i = 0; i < swap_chain.images.size(); i++) {
    swap_chain.image_views.emplace_back();

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = swap_chain.images[i];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = swap_chain.format.format;

    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(logical_device.handle, &ivci,
                                     nullptr, &swap_chain.image_views[i]);
    if (res != VK_SUCCESS)
      return Status("Could not create image view: %s", VulkanEnumToString(res));
  }

  return Status::Ok();
}

Status
VulkanContext::SetupRenderPass() {
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = swap_chain.format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  VkResult res = vkCreateRenderPass(logical_device.handle, &render_pass_info,
                                    nullptr, &pipeline.render_pass);
  if (res != VK_SUCCESS)
    return Status("Could not create render pass: %s", VulkanEnumToString(res));
  return Status::Ok();
}

Status
VulkanContext::SetupPipelineLayout() {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0; // Optional
  pipeline_layout_info.pSetLayouts = nullptr; // Optional
  pipeline_layout_info.pushConstantRangeCount = 0; // Optional
  pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

  VkResult res = vkCreatePipelineLayout(
      logical_device.handle, &pipeline_layout_info, nullptr, &pipeline.layout);

  if (res != VK_SUCCESS) {
    return Status("Could not create pipeline layout: %s",
                  VulkanEnumToString(res));
  }
  return Status::Ok();
}

Status
VulkanContext::SetupGraphicsPipeline() {
  Status status;

  // Vertex shader.
  std::vector<char> vertex_src;
  status = ReadWholeFile("out/simple.vert.spv", &vertex_src);
  if (!status.ok())
    return status;

  pipeline.shader_modules.emplace_back();
  auto& vertex_module = pipeline.shader_modules.back();
  status = CreateShaderModule(this, vertex_src, &vertex_module);
  if (!status.ok())
    return status;

  VkPipelineShaderStageCreateInfo vert_create_info = {};
  vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_create_info.module = vertex_module;
  vert_create_info.pName = "main";

  // Fragment shader.

  std::vector<char> fragment_src;
  status = ReadWholeFile("out/simple.frag.spv", &fragment_src);
  if (!status.ok())
    return status;

  pipeline.shader_modules.emplace_back();
  auto& fragment_module = pipeline.shader_modules.back();

  status = CreateShaderModule(this, fragment_src, &fragment_module);
  if (!status.ok())
    return status;
  VkPipelineShaderStageCreateInfo frag_create_info = {};
  frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_create_info.module = fragment_module;
  frag_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_create_info,
                                                     frag_create_info};

  // Vertex input state
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;

  // Assembly: How the geometry is interpreted.
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  // Viewport: The extent of the window we're going to draw to.
  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swap_chain.extent.width;
  viewport.height = (float)swap_chain.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Scissor: The sub-window we will actually keep.
  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain.extent;

  // The Viewport state.
  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  // Rasterizer configuration.
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // Whether to clamp instead of discard for depth test.
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;  // Disable the rasterizer stage.
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  // Multisampling. (Disabled for now).
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.blendEnable = VK_TRUE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f; // Optional
  color_blending.blendConstants[1] = 0.0f; // Optional
  color_blending.blendConstants[2] = 0.0f; // Optional
  color_blending.blendConstants[3] = 0.0f; // Optional

  // Create the graphics pipeline.

  VkGraphicsPipelineCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.stageCount = 2;
  create_info.pStages = shader_stages;

  create_info.pVertexInputState = &vertex_input_info;
  create_info.pInputAssemblyState = &input_assembly;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterizer;
  create_info.pMultisampleState = &multisampling;
  create_info.pDepthStencilState = nullptr; // Optional
  create_info.pColorBlendState = &color_blending;
  create_info.pDynamicState = nullptr; // Optional

  create_info.layout = pipeline.layout;
  create_info.renderPass = pipeline.render_pass;
  create_info.subpass = 0;

  create_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  create_info.basePipelineIndex = -1; // Optional

  VkResult res =
      vkCreateGraphicsPipelines(logical_device.handle, VK_NULL_HANDLE, 1,
                                &create_info, nullptr, &pipeline.pipeline);
  if (res != VK_SUCCESS) {
    return Status("Error creating graphics pipeline: %s",
                  VulkanEnumToString(res));
  }
  return Status::Ok();
}

Status
VulkanContext::SetupFrameBuffers() {
  frame_buffers.reserve(swap_chain.image_views.size());

  for (size_t i = 0; i < frame_buffers.size(); i++) {
    frame_buffers.emplace_back();
    frame_buffers[i] = VK_NULL_HANDLE;

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = pipeline.render_pass;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &swap_chain.image_views[i];
    create_info.width = swap_chain.extent.width;
    create_info.height = swap_chain.extent.height;
    create_info.layers = 1;

    VkResult res = vkCreateFramebuffer(logical_device.handle,
                                       &create_info,
                                       nullptr,
                                       &frame_buffers[i]);
    if (res != VK_SUCCESS)
      return Status("Error creating frame buffer: %s", VulkanEnumToString(res));
  }

  return Status::Ok();
}

Status
VulkanContext::SetupCommandPool() {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = physical_device.graphics_queue_index;
  create_info.flags = 0;  // Optional.

  VkResult res = vkCreateCommandPool(logical_device.handle,
                                     &create_info,
                                     nullptr,
                                     &command_pool);
  if (res != VK_SUCCESS)
    return Status("Could not create Command Pool: %s", VulkanEnumToString(res));
  return Status::Ok();
}

// Utils -----------------------------------------------------------------------

namespace {

Status
AddInstanceExtensions(SDL_Window* window, VulkanContext* context) {
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions, window,
                    (context->instance.extensions));
  if (context->instance.extensions.empty())
    return Status("Could not get SDL required extensions");
  // Add the debug extensions.
  context->instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return Status::Ok();
}

Status
AddInstanceValidationLayers(VulkanContext* context) {
  // Add validation layers
  context->instance.validation_layers.push_back(
      "VK_LAYER_LUNARG_standard_validation");
  return Status::Ok();
}

Status
CheckRequiredLayers(const std::vector<const char*>& requested_layers) {
  if (requested_layers.empty())
    return Status::Ok();

  // Check available validation layers.
  std::vector<VkLayerProperties> available_layers;
  VK_GET_PROPERTIES_NC(vkEnumerateInstanceLayerProperties, available_layers);

  if (available_layers.empty())
    return Status("No layers available");

  // We check that the requested layers exist.
  for (const char* requested_layer : requested_layers) {
    bool layer_found = false;

    for (const auto& layer : available_layers) {
      if (strcmp(requested_layer, layer.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found)
      return Status("Layer %s not found", requested_layer);
  }

  return Status::Ok();
};

VulkanContext::SwapChain
GetSwapChainProperties(const VulkanContext& context,
                       const VulkanContext::PhysicalDevice& device) {
  // Setup the swap chain.
  VulkanContext::SwapChain swap_chain;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.handle, context.surface,
                                            &swap_chain.capabilites);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.handle, context.surface,
                                       &format_count, nullptr);
  swap_chain.formats.resize(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.handle, context.surface, &format_count, swap_chain.formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.handle, context.surface,
                                            &present_mode_count, nullptr);

  swap_chain.present_modes.resize(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.handle, context.surface,
                                            &present_mode_count,
                                            swap_chain.present_modes.data());

  return swap_chain;
}


bool
IsSuitablePhysicalDevice(const VulkanContext& context,
                         const VulkanContext::PhysicalDevice& device,
                         const std::vector<const char*>& extensions) {
  // Queues.
  if (device.graphics_queue_index < 0 ||
      device.present_queue_index < 0)
    return false;


  if (extensions.empty())
    return true;

  // Get the the extensions the physical device actually offers.
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device.handle,
                                       nullptr,
                                       &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> available_extensions;
  available_extensions.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(device.handle,
                                       nullptr,
                                       &extension_count,
                                       available_extensions.data());

  // All extensions should be present.
  for (const char* extension : extensions) {
    bool found = false;
    for (const auto& available_extension : available_extensions) {
      if (strcmp(available_extension.extensionName,
                 extension) == 0) {
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  // Swap chain properties.
  if (context.swap_chain.formats.empty() ||
      context.swap_chain.present_modes.empty()) {
    return false;
  }

  return true;
}

VkSurfaceFormatKHR
GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  // When undefined, vulkan lets us decided whichever we want.
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  // We try to find the RGBA SRGB format.
  for (const auto& format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace ==  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }

  // Otherwise return the first one.
  return formats.front();
}

static inline bool
HasPresentMode(const std::vector<VkPresentModeKHR>& present_modes,
               const VkPresentModeKHR& target) {
  for (const auto& present_mode : present_modes) {
    if (present_mode == target)
      return true;
  }
  return false;
}

VkPresentModeKHR
GetBestPresentMode(const std::vector<VkPresentModeKHR>& present_modes) {
  if (HasPresentMode(present_modes, VK_PRESENT_MODE_MAILBOX_KHR))
      return VK_PRESENT_MODE_MAILBOX_KHR;

  // Sometimes FIFO is not well implemented, so prefer immediate.
  if (HasPresentMode(present_modes, VK_PRESENT_MODE_IMMEDIATE_KHR))
    return VK_PRESENT_MODE_IMMEDIATE_KHR;

  // FIFO is assured to be there.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    return capabilities.maxImageExtent;

#if 0
    // Do the clamping.
    VkExtent2D extent = {WIDTH, HEIGHT};
    // TODO: Use MAX, MIN macros.
    extent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, extent.height));

    return extent;
#endif
  }
}

Status
CreateShaderModule(VulkanContext* context, const std::vector<char>& src,
                   VkShaderModule* out) {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = src.size();
  create_info.pCode = (const uint32_t*)src.data();

  VkShaderModule handle = VK_NULL_HANDLE;
  VkResult res = vkCreateShaderModule(context->logical_device.handle,
                                      &create_info, nullptr, &handle);
  if (res != VK_SUCCESS) {
    return Status("Could not create shader module: %s",
                  VulkanEnumToString(res));
  }

  *out = handle;
  return Status::Ok();
}

}  // namespace

}  // namespace warhol
