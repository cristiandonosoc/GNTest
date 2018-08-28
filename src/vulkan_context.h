// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "macros.h"
#include "status.h"

struct SDL_Window;

namespace warhol {

struct InstanceContext;
struct PhysicalDeviceContext;
struct LogicalDeviceContext;
struct SwapChainContext;
struct ImageViewContext;

// Reference holder for the current selected state.
// These references must outlive the holder.
struct SelectedContext {
  InstanceContext* instance;
  PhysicalDeviceContext* physical_device;
  LogicalDeviceContext* logical_device;
  SwapChainContext* swap_chain;
};

// InstanceContext -------------------------------------------------------------

// Overall context to run a vulkan app.
// Will destroy the managed resources (instance, messengers) on destruction.
struct InstanceContext {
  InstanceContext();
  ~InstanceContext();

  // Handles
  VkInstance handle = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_handle = VK_NULL_HANDLE;

  // Extensions to be added to the VkInstance
  std::vector<const char*> extensions;
  // List of validation layers.
  std::vector<const char*> validation_layers;

  // The surface to where we will be drawing.

  // Store by unique ptr so that the move is trivial.
  // Otherwise care would be needed about moving the handles, as some compilers
  // won't bother invalidating the handle.
  // This is more relevant for the PhysicalDeviceContext though.
  std::vector<std::unique_ptr<PhysicalDeviceContext>> physical_devices;

  DELETE_COPY_AND_ASSIGN(InstanceContext);
};

// Creates an instance. The extensions and validation layers should be already
// set at this point.
Status SetupSDLVulkanInstance(InstanceContext*);

// PhysicalDevice --------------------------------------------------------------

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilites;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct PhysicalDeviceContext {
  PhysicalDeviceContext(InstanceContext*);
  ~PhysicalDeviceContext();

  InstanceContext* instance;   // Not owning, must outline;

  VkPhysicalDevice handle = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;

  SwapChainProperties swap_chain_properties;

  // Queues
  int graphics_queue_index = -1;
  int present_queue_index = -1;
  std::vector<VkQueueFamilyProperties> qf_properties;

  // Surface.
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  // Store by unique ptr so that the move is trivial.
  // Otherwise care would be needed about moving the handles, as some compilers
  // won't bother invalidating the handle.
  std::vector<std::unique_ptr<LogicalDeviceContext>> logical_devices;

  DELETE_COPY_AND_ASSIGN(PhysicalDeviceContext);
};

// Setups the logical devices and bionds the first one to the context.
Status SetupVulkanPhysicalDevices(SDL_Window*, InstanceContext*);

// Validate that the requested extensions are provided by the vulkan
// implementation.
bool
CheckPhysicalDeviceRequiredExtensions(
    const PhysicalDeviceContext&,
    const std::vector<const char*>& requested_extensions);

Status
CreateSurface(SDL_Window*, InstanceContext*, PhysicalDeviceContext*);

PhysicalDeviceContext*
FindSuitablePhysicalDevice(
    InstanceContext*, const std::vector<const char*>& requested_extensions);

// LogicalDevice ---------------------------------------------------------------

struct LogicalDeviceContext {
  LogicalDeviceContext(PhysicalDeviceContext*);
  ~LogicalDeviceContext();

  PhysicalDeviceContext* physical_device;   // Not owning, must outline.

  VkDevice handle = VK_NULL_HANDLE;

  // Queue in charge of the graphical commands.
  VkQueue graphics_queue = VK_NULL_HANDLE;

  // Queue in charge of outputting to the system surface/window.
  VkQueue present_queue = VK_NULL_HANDLE;

  // Swapchain
  std::unique_ptr<SwapChainContext> swap_chain;
  std::unique_ptr<ImageViewContext> image_view;

  DELETE_COPY_AND_ASSIGN(LogicalDeviceContext);
};

Status
SetupVulkanLogicalDevices(InstanceContext*,
                          PhysicalDeviceContext*,
                          const std::vector<const char*>& extensions);

// SwapChain -------------------------------------------------------------------

struct SwapChainContext {
  SwapChainContext(LogicalDeviceContext*);
  ~SwapChainContext();

  SwapChainProperties properties;

  // Direct handles
  VkSurfaceFormatKHR surface_format;
  VkPresentModeKHR present_mode;
  VkExtent2D extent;

  VkSwapchainKHR handle = VK_NULL_HANDLE;
  LogicalDeviceContext* logical_device;   // Not owning, must outlive.

  std::vector<VkImage> images;
  std::vector<ImageViewContext> image_views;

  DELETE_COPY_AND_ASSIGN(SwapChainContext);
};

Status
SetupSwapChain(PhysicalDeviceContext*, LogicalDeviceContext*);

// ImageView -------------------------------------------------------------------

struct ImageViewContext {
  ImageViewContext(SwapChainContext*);
  ~ImageViewContext();

  SwapChainContext* swap_chain;   // Not owning, must outlive.

  VkImageView handle = VK_NULL_HANDLE;

  DELETE_COPY_AND_ASSIGN(ImageViewContext);
  DECLARE_MOVE_AND_ASSIGN(ImageViewContext);
};

Status
CreateImageViews(SwapChainContext*);

// Misc ------------------------------------------------------------------------

// Gets the extensions SDL needs to hook up correctly with vulkan.
Status
GetSDLExtensions(SDL_Window*, InstanceContext*);

// Validate that the requested layers are provided by the vulkan implementation.
bool CheckRequiredLayers(const std::vector<const char*>& requested_layers);

}  // namespace warhol
