// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/context.h"

#include <string.h>

#include <set>

#include "warhol/graphics/vulkan/memory.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/math/vec.h"
#include "warhol/utils/file.h"
#include "warhol/utils/log.h"
#include "warhol/utils/scope_trigger.h"
#include "warhol/utils/types.h"

namespace warhol {
namespace vulkan {

Context::Context() = default;

// CreateContext & -------------------------------------------------------------

bool CreateContext(Context* context) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Warhol Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  create_info.enabledExtensionCount = context->extensions.size();
  create_info.ppEnabledExtensionNames = context->extensions.data();

  create_info.enabledLayerCount = context->validation_layers.size();
  create_info.ppEnabledLayerNames = context->validation_layers.data();

  VkInstance instance;
  if (!VK_CALL(vkCreateInstance, &create_info, nullptr, &instance))
    return false;

  context->instance.Set(context, instance);
  return true;
}

// SetupDebugCall --------------------------------------------------------------

bool SetupDebugCall(Context* context,
                    PFN_vkDebugUtilsMessengerCallbackEXT callback) {
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pUserData = nullptr;
  create_info.pfnUserCallback = callback;

  VkDebugUtilsMessengerEXT debug_messenger;
  if (!CreateDebugUtilsMessengerEXT(
          context->instance.value(), &create_info, nullptr, &debug_messenger)) {
    return false;
  }

  context->debug_messenger.Set(context, debug_messenger);
  return true;
}

// PickPhysicalDevice ----------------------------------------------------------

namespace {

QueueFamilyIndices FindQueueFamilyIndices(const VkPhysicalDevice& device,
                                          const VkSurfaceKHR& surface) {
  QueueFamilyIndices indices = {};

  std::vector<VkQueueFamilyProperties> queue_families;
  VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties,
                    device,
                    queue_families);

  // We look for a GPU that has a graphics queue.
  for (size_t i = 0; i < queue_families.size(); i++) {
    auto& queue_family = queue_families[i];
    if (queue_family.queueCount == 0)
      continue;

    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphics = i;

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support)
      indices.present = i;
  }

  return indices;
}

SwapChainCapabilities QuerySwapChainSupport(const VkPhysicalDevice& device,
                                            const VkSurfaceKHR& surface) {
  SwapChainCapabilities details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  VK_GET_PROPERTIES4(vkGetPhysicalDeviceSurfaceFormatsKHR,
                     device, surface, details.formats);

  LOG(INFO) << "  Got device properties: ";
  for (const auto& format : details.formats)
    LOG(INFO) << "  - Format: " << EnumToString(format.format)
               << ", Color Space: " << EnumToString(format.colorSpace);

  VK_GET_PROPERTIES4(vkGetPhysicalDeviceSurfacePresentModesKHR,
                     device, surface, details.present_modes);
  for (const auto& present_mode : details.present_modes)
    LOG(INFO) << "  - Present Mode: " << EnumToString(present_mode);

  return details;
}

PhysicalDeviceInfo GetPhysicalDeviceInfo(const VkPhysicalDevice& device,
                           const VkSurfaceKHR& surface) {
  PhysicalDeviceInfo info;
  vkGetPhysicalDeviceProperties(device, &info.properties);
  vkGetPhysicalDeviceFeatures(device, &info.features);
  vkGetPhysicalDeviceMemoryProperties(device, &info.memory_properties);
  info.queue_family_indices = FindQueueFamilyIndices(device, surface);
  info.swap_chain_capabilities = QuerySwapChainSupport(device, surface);
  return info;
}

bool IsSuitableDevice(const VkPhysicalDevice& device,
                      const PhysicalDeviceInfo& device_info,
                      const std::vector<const char*>& required_extensions) {
  LOG(INFO) << "Checking GPU: " << device_info.properties.deviceName;

  auto& indices = device_info.queue_family_indices;
  if (indices.graphics == -1 || indices.present == -1)
    return false;

  if (!CheckPhysicalDeviceExtensions(device, required_extensions))
    return false;

  auto& capabilities = device_info.swap_chain_capabilities;
  if (capabilities.formats.empty() || capabilities.present_modes.empty())
    return false;
  return true;
}

}  // namespace

bool PickPhysicalDevice(Context* context) {
  std::vector<VkPhysicalDevice> devices;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, *context->instance, devices);
  if (devices.empty()) {
    LOG(ERROR) << "Could not find GPUs with Vulkan support.";
    return false;
  }

  std::vector<std::pair<VkPhysicalDevice, PhysicalDeviceInfo>> suitable_devices;
  for (const VkPhysicalDevice& device : devices) {
    PhysicalDeviceInfo device_info =
        GetPhysicalDeviceInfo(device, *context->surface);
    if (IsSuitableDevice(device, device_info, context->device_extensions)) {
      suitable_devices.push_back({device, device_info});
      LOG(INFO) << "- Suitable!";
    }
  }

  if (suitable_devices.empty()) {
    LOG(ERROR) << "Could not find a suitable physical device.";
    return false;
  }

  // For now we choose the first device.
  auto& [device, device_info] = suitable_devices.front();
  context->physical_device = device;
  context->physical_device_info = device_info;
  return true;
}

// CreateLogicalDevice ---------------------------------------------------------

bool CreateLogicalDevice(Context* context) {
  // Setup the queues.
  /* auto indices = FindQueueFamilyIndices(context->physical_device, */
  /*                                       *context->surface); */

  // We create one per queue family required. If there are the same, we only
  // create one.
  auto& indices = context->physical_device_info.queue_family_indices;
  int queue_indices[] = { indices.graphics, indices.present };
  VkDeviceQueueCreateInfo queue_create_infos[ARRAY_SIZE(queue_indices)] = {};

  std::set<int> unique_queues;
  unique_queues.insert(indices.graphics);
  unique_queues.insert(indices.present);

  float queue_priority = 1.0f;
  for (size_t i = 0; i < unique_queues.size(); i++) {
    auto& queue_create_info = queue_create_infos[i];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_indices[i];
    queue_create_info.queueCount = 1; // Only need 1 queue of this kind.
    queue_create_info.pQueuePriorities = &queue_priority;
  }

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = unique_queues.size();
  create_info.pQueueCreateInfos = queue_create_infos;
  VkPhysicalDeviceFeatures enabled_features = {};
  create_info.pEnabledFeatures = &enabled_features;
  create_info.enabledExtensionCount = context->device_extensions.size();
  create_info.ppEnabledExtensionNames = context->device_extensions.data();

  VkDevice device;
  if (!VK_CALL(vkCreateDevice, context->physical_device, &create_info, nullptr,
                               &device)) {
    return false;
  }

  context->device.Set(context, device);
  vkGetDeviceQueue(device, indices.graphics, 0, &context->graphics_queue);
  vkGetDeviceQueue(device, indices.present, 0, &context->present_queue);
  return true;
}

// CreateSwapChain -------------------------------------------------------------

namespace {

VkSurfaceFormatKHR
ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  // If vulkan only returns an undefined format, it means that it has no
  // preference and we can choose.
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    VkSurfaceFormatKHR result;
    result.format = VK_FORMAT_B8G8R8A8_UNORM;
    result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    return result;
  }

  for (const VkSurfaceFormatKHR& format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return format;
  }

  // At this point we don't have a prefered one, so we just return the first.
  return formats.front();
}

VkPresentModeKHR
ChooseSwapChainPresentMode(std::vector<VkPresentModeKHR>& present_modes) {
  // We search for mailbox (to do triple buffering).
  // FIFO is assured to exist.
  for (const VkPresentModeKHR& present_mode : present_modes) {
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return present_mode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& cap,
                                 Pair<uint32_t> screen_size) {
  // uint32_t::max means that the window manager lets us pick the extent.
  if (cap.currentExtent.width != Limits::kUint32Max)
    return cap.currentExtent;

  // We clamp our required extent to the bounds given by the GPU.
  VkExtent2D extent = {screen_size.x, screen_size.y};
  extent.width = Clamp(extent.width,
                       cap.minImageExtent.width, cap.maxImageExtent.width);
  extent.height = Clamp(extent.height,
                        cap.minImageExtent.height, cap.maxImageExtent.height);
  return extent;
}

}  // namespace

bool CreateSwapChain(Context* context, Pair<uint32_t> screen_size) {
  auto& capabilities = context->physical_device_info.swap_chain_capabilities;
  VkSurfaceFormatKHR format =
      ChooseSwapChainSurfaceFormat(capabilities.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapChainPresentMode(capabilities.present_modes);

  VkExtent2D extent =
      ChooseSwapChainExtent(capabilities.capabilities, screen_size);

  uint32_t min_image_count = capabilities.capabilities.minImageCount;
  uint32_t max_image_count = capabilities.capabilities.maxImageCount;
  uint32_t image_count = min_image_count + 1;
  // We clamp the value if necessary. max == 0 means no limit in image count.
  if (max_image_count > 0 && image_count > max_image_count)
    image_count = max_image_count;

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = *context->surface;

  create_info.minImageCount = image_count;
  create_info.imageFormat = format.format;
  create_info.imageColorSpace = format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // If the graphics and present queues are different, we need to establish
  // sharing mode between them.
  auto& indices = context->physical_device_info.queue_family_indices;
  uint32_t indices_array[2] = {(uint32_t)indices.graphics,
                               (uint32_t)indices.present};
  if (indices.graphics != indices.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = ARRAY_SIZE(indices_array);
    create_info.pQueueFamilyIndices = indices_array;
  } else {
    // If they're the same, we establish no sharing.
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  // Leave the image as it is.
  create_info.preTransform = capabilities.capabilities.currentTransform;

  // We ingore the alpha for blending with the window system.
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swap_chain;
  if (!VK_CALL(vkCreateSwapchainKHR, *context->device, &create_info, nullptr,
               &swap_chain)) {
    return false;
  }

  context->swap_chain.Set(context, swap_chain);
  context->swap_chain_details.format = format;
  context->swap_chain_details.present_mode = present_mode;
  context->swap_chain_details.extent = extent;

  // Retrieve the images.
  VK_GET_PROPERTIES4(vkGetSwapchainImagesKHR,
                     *context->device, swap_chain, context->images);
  return true;
}

// CreateImageViews ------------------------------------------------------------

bool CreateImageViews(Context* context) {
  context->image_views.clear();
  for (size_t i = 0; i < context->images.size(); i++) {
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = context->images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = context->swap_chain_details.format.format;

    // We don't need to swizzle (swap around) any of the color channel.
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // This is an image (color texture).
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (!VK_CALL(vkCreateImageView, *context->device, &create_info, nullptr,
                 &image_view)) {
      return false;
    }
    context->image_views.emplace_back(context, image_view);
  }

  return true;
}

// CreateRenderPass ------------------------------------------------------------

bool CreateRenderPass(Context* context) {
  // Represents how our buffer will relte from/to the frame buffers.
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = context->swap_chain_details.format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // What layouts the data has to be before and after using the render target.
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // References are used by sub-render passes.
  VkAttachmentReference color_attachment_ref = {};
  // Index into the attachment description array.
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  // This is a graphics subpass.
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  // Create a dependency for this render pass.
  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstSubpass = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &color_attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &dependency;

  VkRenderPass render_pass;
  if (!VK_CALL(vkCreateRenderPass, *context->device, &create_info, nullptr,
               &render_pass)) {
    return false;
  }
  context->render_pass.Set(context, render_pass);
  return true;
}

// CreateDescriptorSetLayout ---------------------------------------------------

bool CreateDescriptorSetLayout(Context* context) {
  VkDescriptorSetLayoutBinding ubo_binding = {};
  ubo_binding.binding = 0;
  ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_binding.descriptorCount = 1;
  ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_binding.pImmutableSamplers = nullptr;  // Optional.

  VkDescriptorSetLayoutCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  create_info.bindingCount = 1;
  create_info.pBindings = &ubo_binding;

  VkDescriptorSetLayout descriptor_set_layout;
  if (!VK_CALL(vkCreateDescriptorSetLayout, *context->device, &create_info,
               nullptr, &descriptor_set_layout)) {
    return false;
  }

  context->descriptor_set_layout.Set(context, descriptor_set_layout);
  return true;
}

// CreatePipelineLayout --------------------------------------------------------

bool CreatePipelineLayout(Context* context) {
  VkPipelineLayoutCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = 1;
  create_info.pSetLayouts = &context->descriptor_set_layout.value();
  create_info.pushConstantRangeCount = 0; // Optional
  create_info.pPushConstantRanges = nullptr; // Optional

  VkPipelineLayout pipeline_layout;
  if (!VK_CALL(vkCreatePipelineLayout, *context->device, &create_info, nullptr,
                                       &pipeline_layout)) {
    return false;
  }
  context->pipeline_layout.Set(context, pipeline_layout);
  return true;
}

// CreateGraphicsPipeline ------------------------------------------------------

namespace {

VkShaderModule CreateShaderModule(const VkDevice& device,
                                  const std::vector<char>& data) {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = data.size();
  create_info.pCode = (const uint32_t*)(data.data());

  VkShaderModule shader;
  if (!VK_CALL(vkCreateShaderModule, device, &create_info, nullptr, &shader))
    return VK_NULL_HANDLE;
  return shader;
}

std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindings;
  bindings.reserve(2);

  VkVertexInputBindingDescription binding;
  binding = {};
  binding.binding = 0;
  binding.stride = 3 * sizeof(float);
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  bindings.push_back(binding);

  binding.binding = 1;
  bindings.push_back(binding);
  return bindings;
}

std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> descriptions;
  descriptions.reserve(2);

  VkVertexInputAttributeDescription pos_desc = {};
  pos_desc.binding = 0;
  pos_desc.location = 0;
  pos_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  pos_desc.offset = 0;
  descriptions.push_back(std::move(pos_desc));

  VkVertexInputAttributeDescription color_desc = {};
  color_desc.binding = 1;
  color_desc.location = 1;
  color_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  color_desc.offset = 0;
  descriptions.push_back(std::move(color_desc));

  return descriptions;
}

}  // namespace

bool CreateGraphicsPipeline(Context* context) {
  std::vector<char> vert_data, frag_data;
  if (!ReadWholeFile(context->vert_shader_path, &vert_data, false) ||
      !ReadWholeFile(context->frag_shader_path, &frag_data, false)) {
    return false;
  }

  VkShaderModule vert_module = CreateShaderModule(*context->device, vert_data);
  ScopeTrigger vert_trigger([&vert_module, &device = *context->device]() {
    if (vert_module != VK_NULL_HANDLE)
      vkDestroyShaderModule(device, vert_module, nullptr);
  });

  VkShaderModule frag_module = CreateShaderModule(*context->device, frag_data);
  ScopeTrigger frag_trigger([&frag_module, &device = *context->device]() {
    if (frag_module != VK_NULL_HANDLE)
      vkDestroyShaderModule(device, frag_module, nullptr);
  });

  if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE)
    return false;

  VkPipelineShaderStageCreateInfo vert_create_info = {};
  vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_create_info.module = vert_module;
  vert_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_create_info = {};
  frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_create_info.module = frag_module;
  frag_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_create_info,
                                                     frag_create_info};

  // ******* Vertex input ******
  //
  // Describes the format the vertex data will be passed to the vertex shader.
  // Bindings: Whether the spacing between data and if it's per vertex or per
  //           instance.
  // Attribute Descriptions: Information about the attribute locations, etc.

  auto bindings = GetBindingDescriptions();
  auto attributes = GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertex_input = {};
  vertex_input.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input.vertexBindingDescriptionCount = bindings.size();
  vertex_input.pVertexBindingDescriptions = bindings.data();
  vertex_input.vertexAttributeDescriptionCount = attributes.size();
  vertex_input.pVertexAttributeDescriptions = attributes.data();

  // ******  Input Assembly ******
  //
  // What kind of primitive to use and if primitive restart is enabled.

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  // ****** Viewport & Scissor ******
  //
  // Viewport: Size of the framebuffer the image will be transformed to.
  // Scissor: Region of pixels which will be actually stored.

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)context->swap_chain_details.extent.width;
  viewport.height = (float)context->swap_chain_details.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = context->swap_chain_details.extent;

  // The actual vulkan structure that holds the viewport & scissor info.
  // Using multiple viewports/scissors requires enabling a GPU feature.

  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  // ****** Rasterizer ******
  //
  // Rasterizing is the process of taking geometry and tarnsforming into screen
  // fragments that can be evaluated by a fragment shader. It performs depth
  // testing, face culling, scissor test, wireframe rendering, etc.

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // Whether to clamp instead of discard for depth test. Requires GPU feature.
  rasterizer.depthClampEnable = VK_FALSE;
  // Disable the rasterizer stage. Basically discard output to the framebuffer.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // Any other mode than fill requires a GPU feature.
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  // Requires enabling the "wideLines" GPU feature.
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;  // Cull back faces.
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  // Useful for solving z-fighting.
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  // ****** Multisampling ******
  //
  // Combines the fragment shader results of many polygons that affect the same
  // fragment. Useful for fighting anti-aliasing. Requires a GPU feature.

  // Disabled for now.
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  // ****** Color Blending ******
  //
  // Determines how the color of a new fragment combines with the existing one
  // in the framebuffer. There are two ways:
  //
  // 1. Mix the old and new.
  // 2. Combine old and new with bitwise operation.


  // A Color Blend Attachment describes blending *per* framebuffer.

  VkPipelineColorBlendAttachmentState color_attachment = {};
  color_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  // blendEnable means that the blending equation parametized in the factors
  // will be used. If not the color will be overwritten according to the
  // colorWriteMask.
  color_attachment.blendEnable = VK_FALSE;
  color_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  color_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  color_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  color_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  color_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  color_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

  // Color Blend State represents the global state that is used and overwritten
  // by the blend attachments.

  VkPipelineColorBlendStateCreateInfo color_blend_state = {};
  color_blend_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state.logicOpEnable = VK_FALSE;
  color_blend_state.logicOp = VK_LOGIC_OP_COPY;  // Optional
  color_blend_state.attachmentCount = 1;
  color_blend_state.pAttachments = &color_attachment;
  color_blend_state.blendConstants[0] = 0.0f; // Optional
  color_blend_state.blendConstants[1] = 0.0f; // Optional
  color_blend_state.blendConstants[2] = 0.0f; // Optional
  color_blend_state.blendConstants[3] = 0.0f; // Optional

  // ****** Dynamic State ******
  //
  // Represents which aspects of the graphics pipeline can be changed without
  // re-constructing the pipeline again.
  // Setting these means that the configuration for these will be ignored and
  // will be required to be set on drawing time.

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_LINE_WIDTH,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = ARRAY_SIZE(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  // ----------------------
  // Create the actual pipeline object.

  VkGraphicsPipelineCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  // Shader stages.
  create_info.stageCount = 2;
  create_info.pStages = shader_stages;

  // Fixed function configuration.
  create_info.pVertexInputState = &vertex_input;
  create_info.pInputAssemblyState = &input_assembly;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterizer;
  create_info.pMultisampleState = &multisampling;
  create_info.pDepthStencilState = nullptr; // Optional
  create_info.pColorBlendState = &color_blend_state;
  create_info.pDynamicState = nullptr; // Optional

  // Pipeline layout.
  create_info.layout = *context->pipeline_layout;

  // Render pass.
  create_info.renderPass = *context->render_pass;
  create_info.subpass = 0;  // Index of the subpass to use.

  // These are used to create pipelines from previous ones.
  create_info.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  create_info.basePipelineIndex = -1;  // Optional

  VkPipeline pipeline;
  VkResult res = vkCreateGraphicsPipelines(
      *context->device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
  if (res != VK_SUCCESS) {
    LOG(ERROR) << "Creating graphics pipeline: " << EnumToString(res);
    return false;
  }

  context->pipeline.Set(context, pipeline);
  return true;
}

// CreateFrameBuffers ----------------------------------------------------------

bool CreateFrameBuffers(Context* context) {
  context->frame_buffers.clear();
  for (size_t i = 0; i < context->image_views.size(); i++) {
    // A framebuffer references image views for input data.
    VkImageView attachments[] = { *context->image_views[i] };

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = *context->render_pass;
    create_info.attachmentCount = 1;
    create_info.pAttachments = attachments;
    create_info.width = context->swap_chain_details.extent.width;
    create_info.height = context->swap_chain_details.extent.height;
    create_info.layers = 1;

    VkFramebuffer frame_buffer;
    if (!VK_CALL(vkCreateFramebuffer, *context->device, &create_info, nullptr,
                                      &frame_buffer)) {
      return false;
    }

    context->frame_buffers.emplace_back(context, frame_buffer);
  }

  return true;
}

// CreateCommandPool -----------------------------------------------------------

bool CreateCommandPool(Context* context) {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  // Each command pool can only allocate commands for one particular queue
  // family.
  create_info.queueFamilyIndex =
      context->physical_device_info.queue_family_indices.graphics;
  create_info.flags = 0;  // Optional

  VkCommandPool command_pool;
  if (!VK_CALL(vkCreateCommandPool, *context->device, &create_info, nullptr,
                                    &command_pool)) {
    return false;
  }

  context->command_pool.Set(context, std::move(command_pool));
  return true;
}

// CreateVertexBuffers ---------------------------------------------------------

namespace {

#if 0
std::vector<float> vertices = {
  // Positions
   0.0f, -0.5f,  0.0f,
   0.5f,  0.5f,  0.0f,
  -0.5f,  0.5f,  0.0f,
  // Colors
   1.0f,  0.0f,  0.0f,
   0.0f,  1.0f,  1.0f,
   0.0f,  0.0f,  1.0f,
};
#endif

constexpr size_t kColorOffset = 12;
const std::vector<float> vertices = {
  // Positions
  -0.5f, -0.5f,  0.0f,
   0.5f, -0.5f,  0.0f,
   0.5f,  0.5f,  0.0f,
  -0.5f,  0.5f,  0.0f,

  // Colors
   1.0f,  0.0f,  0.0f,
   0.0f,  1.0f,  0.0f,
   0.0f,  0.0f,  1.0f,
   1.0f,  1.0f,  1.0f
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

bool CreateVertexBuffers(Context* context) {
  VkDeviceSize size = vertices.size() * sizeof(vertices[0]);

  // Create a staging buffer.
  DeviceBackedMemory staging_handles;
  VkBufferUsageFlags staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags staging_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  if (!CreateBuffer(context, size, staging_usage, staging_flags,
                    &staging_handles)) {
    return false;
  }

  // Map the vertex data to the staging buffer.
  void* memory;
  if (!VK_CALL(vkMapMemory, *context->device, *staging_handles.memory, 0, size,
               0, &memory)) {
    return false;
  }
  memcpy(memory, vertices.data(), size);
  vkUnmapMemory(*context->device, *staging_handles.memory);

  // Create the local memory and copy the memory to it.
  DeviceBackedMemory handles;
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  VkMemoryPropertyFlags desired_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (!CreateBuffer(context, size, usage, desired_flags, &handles) ||
      !CopyBuffer(context, *staging_handles.buffer, *handles.buffer, size)) {
    return false;
  }

  // The stating buffers will be freed by Handle<>
  context->vertices = std::move(handles);
  return true;
}

bool CreateIndicesBuffers(Context* context) {
  VkDeviceSize size = indices.size() * sizeof(indices[0]);

  // Create a staging buffer.
  DeviceBackedMemory staging_handles;
  VkBufferUsageFlags staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags staging_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  if (!CreateBuffer(context, size, staging_usage, staging_flags,
                    &staging_handles)) {
    return false;
  }

  // Map the vertex data to the staging buffer.
  void* memory;
  if (!VK_CALL(vkMapMemory, *context->device, *staging_handles.memory, 0, size,
               0, &memory)) {
    return false;
  }
  memcpy(memory, indices.data(), size);
  vkUnmapMemory(*context->device, *staging_handles.memory);

  // Create the local memory and copy the memory to it.
  DeviceBackedMemory handles;
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  VkMemoryPropertyFlags desired_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (!CreateBuffer(context, size, usage, desired_flags, &handles) ||
      !CopyBuffer(context, *staging_handles.buffer, *handles.buffer, size)) {
    return false;
  }

  // The stating buffers will be freed by Handle<>
  context->indices = std::move(handles);
  return true;
}

bool CreateUniformBuffers(Context* context, VkDeviceSize size) {
  context->ubo_size = size;
  context->uniform_buffers.reserve(context->images.size());
  for (size_t i = 0; i < context->images.size(); i++) {
    DeviceBackedMemory ubo;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!CreateBuffer(context, size, usage, flags, &ubo))
      return false;

    ubo.device = *context->device;
    if (!VK_CALL(vkMapMemory, *context->device, *ubo.memory, 0,
                              size, 0, &ubo.data)) {
      return false;
    }

    context->uniform_buffers.emplace_back(std::move(ubo));
  }
  return true;
}

}  // namespace

bool CreateDataBuffers(Context* context, VkDeviceSize ubo_size) {
  if (!CreateVertexBuffers(context) ||
      !CreateIndicesBuffers(context) ||
      !CreateUniformBuffers(context, ubo_size)) {
    return false;
  }
  return true;
}

// CreateDescriptorSets  --------------------------------------------------------

namespace {

bool CreateDescriptorPool(Context* context) {
  VkDescriptorPoolSize pool_size = {};
  pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = context->images.size();

  VkDescriptorPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  create_info.poolSizeCount = 1;
  create_info.pPoolSizes = &pool_size;
  create_info.maxSets = context->images.size();

  VkDescriptorPool pool;
  if (!VK_CALL(vkCreateDescriptorPool, *context->device, &create_info, nullptr,
                                       &pool)) {
    return false;
  }
  context->descriptor_pool.Set(context, pool);
  return true;
}

}  // namespace

bool CreateDescriptorSets(Context* context) {
  if (!CreateDescriptorPool(context))
    return false;

  LOG(INFO) << "Created descriptor pool.";

  // The layout could be different for each descriptor set we're allocating.
  // We point to the same layout everytime.
  std::vector<VkDescriptorSetLayout> layouts(context->images.size(),
                                             *context->descriptor_set_layout);

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = *context->descriptor_pool;
  alloc_info.descriptorSetCount = layouts.size();
  alloc_info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> descriptor_sets(layouts.size());
  if (!VK_CALL(vkAllocateDescriptorSets, *context->device, &alloc_info,
                                         descriptor_sets.data())) {
    return false;
  }


  // We now associate our descriptor sets to a uniform buffer.
  for (size_t i = 0; i < layouts.size(); i++) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = *context->uniform_buffers[i].buffer;
    buffer_info.offset = 0;
    buffer_info.range = context->ubo_size;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_sets[i];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;  // Optional.
    descriptor_write.pTexelBufferView = nullptr;  // Optional.

    // We could also do one big call with an array of descriptor writes.
    vkUpdateDescriptorSets(*context->device, 1, &descriptor_write, 0, nullptr);
  }

  context->descriptor_sets = descriptor_sets;

  return true;
}

// CreateCommandBuffers --------------------------------------------------------

bool CreateCommandBuffers(Context* context) {

  // Command buffers can get multiple allocated at once with one call.
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = *context->command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)context->frame_buffers.size();

  context->command_buffers.clear();
  context->command_buffers.resize(context->frame_buffers.size());
  if (!VK_CALL(vkAllocateCommandBuffers, *context->device, &alloc_info,
               context->command_buffers.data())) {
    return false;
  }

  // Start a command buffer recording.
  for (size_t i = 0; i < context->command_buffers.size(); i++) {
    VkCommandBuffer& command_buffer = context->command_buffers[i];

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    begin_info.pInheritanceInfo = nullptr;  // Optional

    if (!VK_CALL(vkBeginCommandBuffer, command_buffer, &begin_info))
      return false;

    VkRenderPassBeginInfo render_pass_begin = {};
    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.renderPass = *context->render_pass;
    render_pass_begin.framebuffer = *context->frame_buffers[i];
    render_pass_begin.renderArea.offset = {0, 0};
    render_pass_begin.renderArea.extent = context->swap_chain_details.extent;
    VkClearValue clear_value = {{{0.7f, 0.3f, 0.5f, 1.0f}}};
    render_pass_begin.clearValueCount = 1;
    render_pass_begin.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer,
                         &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    {
      vkCmdBindPipeline(command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS, *context->pipeline);

      // We are binding the same buffer in two different points.
      VkBuffer vertex_buffers[] = {
          *context->vertices.buffer,
          *context->vertices.buffer,
      };
      VkDeviceSize offsets[] = {0, kColorOffset * sizeof(float)};
      vkCmdBindVertexBuffers(command_buffer, 0, 2, vertex_buffers, offsets);
      vkCmdBindIndexBuffer(command_buffer, *context->indices.buffer, 0,
                           VK_INDEX_TYPE_UINT16);

      // We bind the descriptor sets.
      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              *context->pipeline_layout, 0, 1,
                              &context->descriptor_sets[i], 0, nullptr);

      vkCmdDrawIndexed(command_buffer, (uint32_t)indices.size(), 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(command_buffer);

    if (!VK_CALL(vkEndCommandBuffer, command_buffer))
      return false;
  }

  return true;
}

// CreateSemaphores ------------------------------------------------------------

bool CreateSyncObjects(Context* context) {
  context->image_available_semaphores.clear();
  context->render_finished_semaphores.clear();
  context->in_flight_fences.clear();

  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  // Start this fence in the signaled state.
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int i = 0; i < context->max_frames_in_flight; i++) {
    VkSemaphore semaphores[2];
    VkFence fence;
    if (!VK_CALL(vkCreateSemaphore, *context->device, &semaphore_create_info,
                 nullptr, &semaphores[0]) ||
        !VK_CALL(vkCreateSemaphore, *context->device, &semaphore_create_info,
                 nullptr, &semaphores[1]) ||
        !VK_CALL(vkCreateFence, *context->device, &fence_create_info, nullptr,
                               &fence)) {
      return false;
    }

    context->image_available_semaphores.emplace_back(context, semaphores[0]);
    context->render_finished_semaphores.emplace_back(context, semaphores[1]);
    context->in_flight_fences.emplace_back(context, fence);
  }

  return true;
}

// RecreateSwapChain -----------------------------------------------------------

namespace {

// We free the resources in the right order before recreating.
void ClearOldSwapChain(Context* context) {
  context->frame_buffers.clear();

  // We don't recreate the command pool, so we just need to free the command
  // buffers.
  vkFreeCommandBuffers(*context->device,
                       *context->command_pool,
                       (uint32_t)context->command_buffers.size(),
                       context->command_buffers.data());
  context->pipeline.Clear();
  context->pipeline_layout.Clear();
  context->render_pass.Clear();
  context->image_views.clear();
  context->swap_chain.Clear();
}

}  // namespace

bool RecreateSwapChain(Context* context, Pair<uint32_t> screen_size) {
  if (!VK_CALL(vkDeviceWaitIdle, *context->device))
      return false;

  ClearOldSwapChain(context);

  // We re-query the swapchain capabilities, as the surface could have changed.

  auto& cap =
      context->physical_device_info.swap_chain_capabilities.capabilities;
  if (!VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
               context->physical_device,
               *context->surface,
               &cap)) {
    return false;
  }

  // Finally we recreate the whole infrastructure again.
  if (!CreateSwapChain(context, screen_size) ||
      !CreateImageViews(context) ||
      !CreateRenderPass(context) ||
      !CreatePipelineLayout(context) ||
      !CreateGraphicsPipeline(context) ||
      !CreateFrameBuffers(context) ||
      !CreateCommandBuffers(context)) {
    return false;
  }

  return true;
}

// Vertex Buffers --------------------------------------------------------------



}  // namespace vulkan
}  // namespace warhol
