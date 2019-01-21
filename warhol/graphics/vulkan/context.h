// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/graphics/vulkan/allocator.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory_utils.h"

#include "warhol/math/math.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/optional.h"


namespace warhol {

struct Image;
struct Mesh;

namespace vulkan {

// The indices for given queues families within a particular physical device.
struct QueueFamilyIndices {
  int graphics = -1;
  int present = -1;
};

// Represents the capabilities a physical device can have for swap chains.
struct SwapChainCapabilities {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct PhysicalDeviceInfo {
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory_properties;
  QueueFamilyIndices queue_family_indices;
  SwapChainCapabilities swap_chain_capabilities;
};

// The actual formats used by a particular swap chain.
struct SwapChainDetails {
  VkSurfaceFormatKHR format;
  VkPresentModeKHR present_mode;
  VkExtent2D extent;
};

struct Context {
  Context();

  // How many frames we can be processing concurrently.
  int max_frames_in_flight = 2;
  int current_frame = 0;

  DELETE_COPY_AND_ASSIGN(Context);
  DEFAULT_MOVE_AND_ASSIGN(Context);

  Allocator allocator = {};

  Handle<VkInstance> instance = {};
  std::vector<const char*> extensions;
  std::vector<const char*> validation_layers;

  Handle<VkDebugUtilsMessengerEXT> debug_messenger = {};

  Handle<VkSurfaceKHR> surface = {};

  // TODO(Cristian): How to specify features that we want to require.
  std::vector<const char*> device_extensions;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Freed with |instance|.
  PhysicalDeviceInfo physical_device_info;

  Handle<VkDevice> device = {};
  VkQueue graphics_queue = VK_NULL_HANDLE;  // Freed with |device|.
  VkQueue present_queue = VK_NULL_HANDLE;   // Freed with |device|.

  Handle<VkSwapchainKHR> swap_chain = {};
  SwapChainDetails swap_chain_details = {};

  std::vector<VkImage> images;    // Freed with |swap_chain|.
  std::vector<Handle<VkImageView>> image_views;

  VkFormat depth_format = VK_FORMAT_UNDEFINED;
  MemoryBacked<VkImage> depth_image;
  Handle<VkImageView> depth_image_view;

  Handle<VkRenderPass> render_pass = {};

  Handle<VkDescriptorSetLayout> descriptor_set_layout;
  Handle<VkPipelineLayout> pipeline_layout = {};

  Handle<VkDescriptorPool> descriptor_pool;
  std::vector<VkDescriptorSet> descriptor_sets; // Freed with |descriptor_pool|.

  std::string vert_shader_path;
  std::string frag_shader_path;
  Handle<VkPipeline> pipeline = {};

  std::vector<Handle<VkFramebuffer>> frame_buffers;

  Handle<VkCommandPool> command_pool = {};
  std::vector<VkCommandBuffer> command_buffers;   // Freed with |command_pool|.
  VkDeviceSize ubo_size;
  std::vector<MemoryBacked<VkBuffer>> uniform_buffers;

  // Actual geometry.
  MemoryBacked<VkBuffer> vertices;
  MemoryBacked<VkBuffer> indices;

  MemoryBacked<VkImage> texture;
  Handle<VkImageView> texture_view;
  Handle<VkSampler> texture_sampler;

  std::vector<Handle<VkSemaphore>> image_available_semaphores;
  std::vector<Handle<VkSemaphore>> render_finished_semaphores;
  std::vector<Handle<VkFence>> in_flight_fences;
};

bool IsValid(const Context&);

// |extensions| and |validation_layers| must already be set within context.
// If successful, |context| will be correctly filled with a VkInstance.
bool CreateContext(Context* context);

// The Context must be valid from here on...

bool SetupDebugCall(Context*, PFN_vkDebugUtilsMessengerCallbackEXT callback);

// |device_extensions| must already be set for both these calls.

// |swap_chain_details| will be filled for the picked physical device.
bool PickPhysicalDevice(Context*);
bool CreateLogicalDevice(Context*);

bool CreateAllocator(Context*);

// A |device| must be created already.
bool CreateSwapChain(Context*, Pair<uint32_t> screen_size);

bool CreateImageViews(Context*);

bool CreateCommandPool(Context*);

bool CreateDepthResources(Context*);

bool CreateRenderPass(Context*);

bool CreateDescriptorSetLayout(Context*);

bool CreatePipelineLayout(Context*);

// |vert_shader_path| and |frag_shader_path| must be set at this call.
bool CreateGraphicsPipeline(Context*);

bool CreateFrameBuffers(Context*);

/* // Creates vertices, indices and uniforms. */
/* // |ubo_size| is the byte size of the uniform buffer object. */
/* bool CreateDataBuffers(Context*, VkDeviceSize ubo_size); */

bool LoadModel(Context*, const Mesh&);

bool SetupUBO(Context* , VkDeviceSize ubo_size);

bool CreateTextureBuffers(Context*, const Image&);

bool CreateTextureImageView(Context*, const Image&);

bool CreateTextureSampler(Context*, const Image&);

// This wil also create the descriptor pools.
bool CreateDescriptorSets(Context*);

bool CreateCommandBuffers(Context*);

bool CreateSyncObjects(Context*);

// -----------------------------------------------------------------------------

bool RecreateSwapChain(Context*, Pair<uint32_t> screen_size);

}  // namespace vulkan
}  // namespace warhol
