// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/utils.h"

#include <string.h>

#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

bool CheckExtensions(const std::vector<const char*>& extensions) {
  std::vector<VkExtensionProperties> properties;
  VK_GET_PROPERTIES(vkEnumerateInstanceExtensionProperties, nullptr, properties);

  for (const char* extension : extensions) {
    bool found = false;
    for (const VkExtensionProperties& property : properties) {
      if (strcmp(extension, property.extensionName) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG(ERROR) << "Could not find extension " << extension;
      return false;
    }
  }

  return true;
}

void AddDebugExtensions(std::vector<const char*>* out) {
  out->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool CheckValidationLayers(const std::vector<const char*>& layers) {
  std::vector<VkLayerProperties> properties;
  VK_GET_PROPERTIES_NC(vkEnumerateInstanceLayerProperties, properties);

  for (const char* layer : layers) {
    bool found = false;
    for (const VkLayerProperties& property : properties) {
      if (strcmp(layer, property.layerName) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG(ERROR) << "Could not find layer " << layer;
      return false;
    }
  }

  return true;
}

bool CheckPhysicalDeviceExtensions(
    const VkPhysicalDevice& device,
    const std::vector<const char*>& required_extensions) {
  std::vector<VkExtensionProperties> extensions;
  VK_GET_PROPERTIES4(
      vkEnumerateDeviceExtensionProperties, device, nullptr, extensions);

  for (const char* required : required_extensions) {
    bool found = false;
    for (const VkExtensionProperties& extension : extensions) {
      if (strcmp(required, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG(WARNING) << "Could not find physical device extension: " << required;
      return false;
    }
  }

  return true;
}

template <>
const char* EnumToString(VkResult res) {
  switch (res) {
    case VK_SUCCESS: return "SUCCESS";
    case VK_NOT_READY: return "NOT_READY";
    case VK_TIMEOUT: return "TIMEOUT";
    case VK_EVENT_SET: return "EVENT_SET";
    case VK_EVENT_RESET: return "EVENT_RESET";
    case VK_INCOMPLETE: return "INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return "ERROR_FRAGMENTED_POOL";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return "ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_SURFACE_LOST_KHR: return "ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "ERROR_INVALID_SHADER_NV";
    case VK_ERROR_FRAGMENTATION_EXT: return "ERROR_FRAGMENTATION_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
      return "ERROR_NOT_PERMITTED_EXT";
    /* case VK_ERROR_OUT_OF_POOL_MEMORY_KHR: */
    /*   return "ERROR_OUT_OF_POOL_MEMORY_KHR"; */
    /* case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR: */
    /*   return "ERROR_INVALID_EXTERNAL_HANDLE_KHR"; */
    default: break;
  }

  ASSERT(!"Unknown option");
  return "";
}

template <>
const char* EnumToString(VkPhysicalDeviceType type) {
  switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
    default: break;
  }

  ASSERT(!"Unknown option");
  return nullptr;
}

template <>
const char* EnumToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
  switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      return "Verbose";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      return "Info";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      return "Warning";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      return "Error";
    default: break;
  }

  ASSERT("Unknown severity");
  return nullptr;
}

template <>
const char* EnumToString(VkDebugUtilsMessageTypeFlagsEXT type) {
  switch (type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: return "General";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: return "Validation";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance";
    default: break;
  }

  ASSERT("Unknown");
  return nullptr;
}

template <>
const char* EnumToString(VkFormat format) {
  switch (format) {
    case VK_FORMAT_UNDEFINED: return "UNDEFINED";
    case VK_FORMAT_R4G4_UNORM_PACK8: return "R4G4_UNORM_PACK8";
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "R4G4B4A4_UNORM_PACK16";
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "B4G4R4A4_UNORM_PACK16";
    case VK_FORMAT_R5G6B5_UNORM_PACK16: return "R5G6B5_UNORM_PACK16";
    case VK_FORMAT_B5G6R5_UNORM_PACK16: return "B5G6R5_UNORM_PACK16";
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "R5G5B5A1_UNORM_PACK16";
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "B5G5R5A1_UNORM_PACK16";
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "A1R5G5B5_UNORM_PACK16";
    case VK_FORMAT_R8_UNORM: return "R8_UNORM";
    case VK_FORMAT_R8_SNORM: return "R8_SNORM";
    case VK_FORMAT_R8_USCALED: return "R8_USCALED";
    case VK_FORMAT_R8_SSCALED: return "R8_SSCALED";
    case VK_FORMAT_R8_UINT: return "R8_UINT";
    case VK_FORMAT_R8_SINT: return "R8_SINT";
    case VK_FORMAT_R8_SRGB: return "R8_SRGB";
    case VK_FORMAT_R8G8_UNORM: return "R8G8_UNORM";
    case VK_FORMAT_R8G8_SNORM: return "R8G8_SNORM";
    case VK_FORMAT_R8G8_USCALED: return "R8G8_USCALED";
    case VK_FORMAT_R8G8_SSCALED: return "R8G8_SSCALED";
    case VK_FORMAT_R8G8_UINT: return "R8G8_UINT";
    case VK_FORMAT_R8G8_SINT: return "R8G8_SINT";
    case VK_FORMAT_R8G8_SRGB: return "R8G8_SRGB";
    case VK_FORMAT_R8G8B8_UNORM: return "R8G8B8_UNORM";
    case VK_FORMAT_R8G8B8_SNORM: return "R8G8B8_SNORM";
    case VK_FORMAT_R8G8B8_USCALED: return "R8G8B8_USCALED";
    case VK_FORMAT_R8G8B8_SSCALED: return "R8G8B8_SSCALED";
    case VK_FORMAT_R8G8B8_UINT: return "R8G8B8_UINT";
    case VK_FORMAT_R8G8B8_SINT: return "R8G8B8_SINT";
    case VK_FORMAT_R8G8B8_SRGB: return "R8G8B8_SRGB";
    case VK_FORMAT_B8G8R8_UNORM: return "B8G8R8_UNORM";
    case VK_FORMAT_B8G8R8_SNORM: return "B8G8R8_SNORM";
    case VK_FORMAT_B8G8R8_USCALED: return "B8G8R8_USCALED";
    case VK_FORMAT_B8G8R8_SSCALED: return "B8G8R8_SSCALED";
    case VK_FORMAT_B8G8R8_UINT: return "B8G8R8_UINT";
    case VK_FORMAT_B8G8R8_SINT: return "B8G8R8_SINT";
    case VK_FORMAT_B8G8R8_SRGB: return "B8G8R8_SRGB";
    case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
    case VK_FORMAT_R8G8B8A8_USCALED: return "R8G8B8A8_USCALED";
    case VK_FORMAT_R8G8B8A8_SSCALED: return "R8G8B8A8_SSCALED";
    case VK_FORMAT_R8G8B8A8_UINT: return "R8G8B8A8_UINT";
    case VK_FORMAT_R8G8B8A8_SINT: return "R8G8B8A8_SINT";
    case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
    case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
    case VK_FORMAT_B8G8R8A8_SNORM: return "B8G8R8A8_SNORM";
    case VK_FORMAT_B8G8R8A8_USCALED: return "B8G8R8A8_USCALED";
    case VK_FORMAT_B8G8R8A8_SSCALED: return "B8G8R8A8_SSCALED";
    case VK_FORMAT_B8G8R8A8_UINT: return "B8G8R8A8_UINT";
    case VK_FORMAT_B8G8R8A8_SINT: return "B8G8R8A8_SINT";
    case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "A8B8G8R8_UNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "A8B8G8R8_SNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "A8B8G8R8_USCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "A8B8G8R8_SSCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "A8B8G8R8_UINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "A8B8G8R8_SINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "A8B8G8R8_SRGB_PACK32";
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "A2R10G10B10_SNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      return "A2R10G10B10_USCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      return "A2R10G10B10_SSCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "A2R10G10B10_UINT_PACK32";
    case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "A2R10G10B10_SINT_PACK32";
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "A2B10G10R10_SNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      return "A2B10G10R10_USCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      return "A2B10G10R10_SSCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "A2B10G10R10_UINT_PACK32";
    case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "A2B10G10R10_SINT_PACK32";
    case VK_FORMAT_R16_UNORM: return "R16_UNORM";
    case VK_FORMAT_R16_SNORM: return "R16_SNORM";
    case VK_FORMAT_R16_USCALED: return "R16_USCALED";
    case VK_FORMAT_R16_SSCALED: return "R16_SSCALED";
    case VK_FORMAT_R16_UINT: return "R16_UINT";
    case VK_FORMAT_R16_SINT: return "R16_SINT";
    case VK_FORMAT_R16_SFLOAT: return "R16_SFLOAT";
    case VK_FORMAT_R16G16_UNORM: return "R16G16_UNORM";
    case VK_FORMAT_R16G16_SNORM: return "R16G16_SNORM";
    case VK_FORMAT_R16G16_USCALED: return "R16G16_USCALED";
    case VK_FORMAT_R16G16_SSCALED: return "R16G16_SSCALED";
    case VK_FORMAT_R16G16_UINT: return "R16G16_UINT";
    case VK_FORMAT_R16G16_SINT: return "R16G16_SINT";
    case VK_FORMAT_R16G16_SFLOAT: return "R16G16_SFLOAT";
    case VK_FORMAT_R16G16B16_UNORM: return "R16G16B16_UNORM";
    case VK_FORMAT_R16G16B16_SNORM: return "R16G16B16_SNORM";
    case VK_FORMAT_R16G16B16_USCALED: return "R16G16B16_USCALED";
    case VK_FORMAT_R16G16B16_SSCALED: return "R16G16B16_SSCALED";
    case VK_FORMAT_R16G16B16_UINT: return "R16G16B16_UINT";
    case VK_FORMAT_R16G16B16_SINT: return "R16G16B16_SINT";
    case VK_FORMAT_R16G16B16_SFLOAT: return "R16G16B16_SFLOAT";
    case VK_FORMAT_R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
    case VK_FORMAT_R16G16B16A16_SNORM: return "R16G16B16A16_SNORM";
    case VK_FORMAT_R16G16B16A16_USCALED: return "R16G16B16A16_USCALED";
    case VK_FORMAT_R16G16B16A16_SSCALED: return "R16G16B16A16_SSCALED";
    case VK_FORMAT_R16G16B16A16_UINT: return "R16G16B16A16_UINT";
    case VK_FORMAT_R16G16B16A16_SINT: return "R16G16B16A16_SINT";
    case VK_FORMAT_R16G16B16A16_SFLOAT: return "R16G16B16A16_SFLOAT";
    case VK_FORMAT_R32_UINT: return "R32_UINT";
    case VK_FORMAT_R32_SINT: return "R32_SINT";
    case VK_FORMAT_R32_SFLOAT: return "R32_SFLOAT";
    case VK_FORMAT_R32G32_UINT: return "R32G32_UINT";
    case VK_FORMAT_R32G32_SINT: return "R32G32_SINT";
    case VK_FORMAT_R32G32_SFLOAT: return "R32G32_SFLOAT";
    case VK_FORMAT_R32G32B32_UINT: return "R32G32B32_UINT";
    case VK_FORMAT_R32G32B32_SINT: return "R32G32B32_SINT";
    case VK_FORMAT_R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
    case VK_FORMAT_R32G32B32A32_UINT: return "R32G32B32A32_UINT";
    case VK_FORMAT_R32G32B32A32_SINT: return "R32G32B32A32_SINT";
    case VK_FORMAT_R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
    case VK_FORMAT_R64_UINT: return "R64_UINT";
    case VK_FORMAT_R64_SINT: return "R64_SINT";
    case VK_FORMAT_R64_SFLOAT: return "R64_SFLOAT";
    case VK_FORMAT_R64G64_UINT: return "R64G64_UINT";
    case VK_FORMAT_R64G64_SINT: return "R64G64_SINT";
    case VK_FORMAT_R64G64_SFLOAT: return "R64G64_SFLOAT";
    case VK_FORMAT_R64G64B64_UINT: return "R64G64B64_UINT";
    case VK_FORMAT_R64G64B64_SINT: return "R64G64B64_SINT";
    case VK_FORMAT_R64G64B64_SFLOAT: return "R64G64B64_SFLOAT";
    case VK_FORMAT_R64G64B64A64_UINT: return "R64G64B64A64_UINT";
    case VK_FORMAT_R64G64B64A64_SINT: return "R64G64B64A64_SINT";
    case VK_FORMAT_R64G64B64A64_SFLOAT: return "R64G64B64A64_SFLOAT";
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "B10G11R11_UFLOAT_PACK32";
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "E5B9G9R9_UFLOAT_PACK32";
    case VK_FORMAT_D16_UNORM: return "D16_UNORM";
    case VK_FORMAT_X8_D24_UNORM_PACK32: return "X8_D24_UNORM_PACK32";
    case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
    case VK_FORMAT_S8_UINT: return "S8_UINT";
    case VK_FORMAT_D16_UNORM_S8_UINT: return "D16_UNORM_S8_UINT";
    case VK_FORMAT_D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "BC1_RGB_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "BC1_RGB_SRGB_BLOCK";
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "BC1_RGBA_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "BC1_RGBA_SRGB_BLOCK";
    case VK_FORMAT_BC2_UNORM_BLOCK: return "BC2_UNORM_BLOCK";
    case VK_FORMAT_BC2_SRGB_BLOCK: return "BC2_SRGB_BLOCK";
    case VK_FORMAT_BC3_UNORM_BLOCK: return "BC3_UNORM_BLOCK";
    case VK_FORMAT_BC3_SRGB_BLOCK: return "BC3_SRGB_BLOCK";
    case VK_FORMAT_BC4_UNORM_BLOCK: return "BC4_UNORM_BLOCK";
    case VK_FORMAT_BC4_SNORM_BLOCK: return "BC4_SNORM_BLOCK";
    case VK_FORMAT_BC5_UNORM_BLOCK: return "BC5_UNORM_BLOCK";
    case VK_FORMAT_BC5_SNORM_BLOCK: return "BC5_SNORM_BLOCK";
    case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "BC6H_UFLOAT_BLOCK";
    case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "BC6H_SFLOAT_BLOCK";
    case VK_FORMAT_BC7_UNORM_BLOCK: return "BC7_UNORM_BLOCK";
    case VK_FORMAT_BC7_SRGB_BLOCK: return "BC7_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "ETC2_R8G8B8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "ETC2_R8G8B8_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
      return "ETC2_R8G8B8A1_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "ETC2_R8G8B8A1_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
      return "ETC2_R8G8B8A8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "ETC2_R8G8B8A8_SRGB_BLOCK";
    case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "EAC_R11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "EAC_R11_SNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "EAC_R11G11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "EAC_R11G11_SNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "ASTC_4x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "ASTC_4x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "ASTC_5x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "ASTC_5x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "ASTC_5x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "ASTC_5x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "ASTC_6x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "ASTC_6x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "ASTC_6x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "ASTC_6x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "ASTC_8x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "ASTC_8x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "ASTC_8x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "ASTC_8x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "ASTC_8x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "ASTC_8x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "ASTC_10x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "ASTC_10x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "ASTC_10x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "ASTC_10x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "ASTC_10x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "ASTC_10x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "ASTC_10x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "ASTC_10x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "ASTC_12x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "ASTC_12x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "ASTC_12x12_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "ASTC_12x12_SRGB_BLOCK";
    case VK_FORMAT_G8B8G8R8_422_UNORM: return "G8B8G8R8_422_UNORM";
    case VK_FORMAT_B8G8R8G8_422_UNORM: return "B8G8R8G8_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      return "G8_B8_R8_3PLANE_420_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: return "G8_B8R8_2PLANE_420_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
      return "G8_B8_R8_3PLANE_422_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: return "G8_B8R8_2PLANE_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
      return "G8_B8_R8_3PLANE_444_UNORM";
    case VK_FORMAT_R10X6_UNORM_PACK16: return "R10X6_UNORM_PACK16";
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: return "R10X6G10X6_UNORM_2PACK16";
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return "R10X6G10X6B10X6A10X6_UNORM_4PACK16";
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_R12X4_UNORM_PACK16: return "R12X4_UNORM_PACK16";
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: return "R12X4G12X4_UNORM_2PACK16";
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return "R12X4G12X4B12X4A12X4_UNORM_4PACK16";
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_G16B16G16R16_422_UNORM: return "G16B16G16R16_422_UNORM";
    case VK_FORMAT_B16G16R16G16_422_UNORM: return "B16G16R16G16_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
      return "G16_B16_R16_3PLANE_420_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
      return "G16_B16R16_2PLANE_420_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
      return "G16_B16_R16_3PLANE_422_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      return "G16_B16R16_2PLANE_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
      return "G16_B16_R16_3PLANE_444_UNORM";
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
    /* case VK_FORMAT_G8B8G8R8_422_UNORM_KHR: return "G8B8G8R8_422_UNORM_KHR"; */
    /* case VK_FORMAT_B8G8R8G8_422_UNORM_KHR: return "B8G8R8G8_422_UNORM_KHR"; */
    /* case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR: */
    /*   return "G8_B8_R8_3PLANE_420_UNORM_KHR"; */
    /* case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR: */
    /*   return "G8_B8R8_2PLANE_420_UNORM_KHR"; */
    /* case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR: */
    /*   return "G8_B8_R8_3PLANE_422_UNORM_KHR"; */
    /* case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR: */
    /*   return "G8_B8R8_2PLANE_422_UNORM_KHR"; */
    /* case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR: */
    /*   return "G8_B8_R8_3PLANE_444_UNORM_KHR"; */
    /* case VK_FORMAT_R10X6_UNORM_PACK16_KHR: return "R10X6_UNORM_PACK16_KHR"; */
    /* case VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR: */
    /*   return "R10X6G10X6_UNORM_2PACK16_KHR"; */
    /* case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR: */
    /*   return "R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR: */
    /*   return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR: */
    /*   return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR: */
    /*   return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR: */
    /*   return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR: */
    /*   return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR: */
    /*   return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR: */
    /*   return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_R12X4_UNORM_PACK16_KHR: return "R12X4_UNORM_PACK16_KHR"; */
    /* case VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR: */
    /*   return "R12X4G12X4_UNORM_2PACK16_KHR"; */
    /* case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR: */
    /*   return "R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR: */
      /* return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR: */
    /*   return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR"; */
    /* case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR: */
    /*   return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR: */
    /*   return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR: */
    /*   return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR: */
    /*   return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR: */
    /*   return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR"; */
    /* case VK_FORMAT_G16B16G16R16_422_UNORM_KHR: */
    /*   return "G16B16G16R16_422_UNORM_KHR"; */
    /* case VK_FORMAT_B16G16R16G16_422_UNORM_KHR: */
    /*   return "B16G16R16G16_422_UNORM_KHR"; */
    /* case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR: */
    /*   return "G16_B16_R16_3PLANE_420_UNORM_KHR"; */
    /* case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR: */
    /*   return "G16_B16R16_2PLANE_420_UNORM_KHR"; */
    /* case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR: */
    /*   return "G16_B16_R16_3PLANE_422_UNORM_KHR"; */
    /* case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR: */
    /*   return "G16_B16R16_2PLANE_422_UNORM_KHR"; */
    /* case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR: */
    /*   return "G16_B16_R16_3PLANE_444_UNORM_KHR"; */
    default: break;
  }

  ASSERT("Unknown");
  return nullptr;
}

template <>
const char* EnumToString(VkColorSpaceKHR color_space) {
  switch (color_space) {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return "SRGB_NONLINEAR_KHR";
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
      return "DISPLAY_P3_NONLINEAR_EXT";
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
      return "EXTENDED_SRGB_LINEAR_EXT";
    case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT: return "DCI_P3_LINEAR_EXT";
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return "DCI_P3_NONLINEAR_EXT";
    case VK_COLOR_SPACE_BT709_LINEAR_EXT: return "BT709_LINEAR_EXT";
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return "BT709_NONLINEAR_EXT";
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return "BT2020_LINEAR_EXT";
    case VK_COLOR_SPACE_HDR10_ST2084_EXT: return "HDR10_ST2084_EXT";
    case VK_COLOR_SPACE_DOLBYVISION_EXT: return "DOLBYVISION_EXT";
    case VK_COLOR_SPACE_HDR10_HLG_EXT: return "HDR10_HLG_EXT";
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return "ADOBERGB_LINEAR_EXT";
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return "ADOBERGB_NONLINEAR_EXT";
    case VK_COLOR_SPACE_PASS_THROUGH_EXT: return "PASS_THROUGH_EXT";
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
      return "EXTENDED_SRGB_NONLINEAR_EXT";
    default: break;
  }

  ASSERT("Unknown");
  return nullptr;
}

template <>
const char* EnumToString(VkPresentModeKHR present_mode) {
  switch (present_mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: return "IMMEDIATE_KHR";
    case VK_PRESENT_MODE_MAILBOX_KHR: return "MAILBOX_KHR";
    case VK_PRESENT_MODE_FIFO_KHR: return "FIFO_KHR";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return "FIFO_RELAXED_KHR";
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
      return "SHARED_DEMAND_REFRESH_KHR";
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
      return "SHARED_CONTINUOUS_REFRESH_KHR";
    default: break;
  }

  ASSERT("Unknown");
  return nullptr;
}

template<> const char* EnumToString(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED: return "VK_IMAGE_LAYOUT_UNDEFINED";
    case VK_IMAGE_LAYOUT_GENERAL: return "VK_IMAGE_LAYOUT_GENERAL";
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
      return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return "VK_IMAGE_LAYOUT_PREINITIALIZED";
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
      return "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
    case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
      return "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV";
    /* case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR: */
    /*   return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR"; */
    /* case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR: */
    /*   return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR"; */
    /* case VK_IMAGE_LAYOUT_BEGIN_RANGE: return "VK_IMAGE_LAYOUT_BEGIN_RANGE"; */
    /* case VK_IMAGE_LAYOUT_END_RANGE: return "VK_IMAGE_LAYOUT_END_RANGE"; */
    case VK_IMAGE_LAYOUT_RANGE_SIZE: return "VK_IMAGE_LAYOUT_RANGE_SIZE";
    case VK_IMAGE_LAYOUT_MAX_ENUM: return "VK_IMAGE_LAYOUT_MAX_ENUM";
    default: break;
  }

  NOT_REACHED();
  return nullptr;
}

}  // namespace vulkan
}  // namespace warhol
