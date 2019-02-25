// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <vector>

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory_utils.h"
#include "warhol/utils/log.h"
#include "warhol/utils/macros.h"

namespace warhol {

struct Camera;
struct Image;
struct Mesh;
struct RendererBackend;

namespace vulkan {

struct Context;

// Rendering backend in charge of managing Vulkan. There is no interface defined
// here because all the interaction is made through RendererBackend::Interface.
//
// On this .cc is the setup of that interface.
struct VulkanRendererBackend {
  VulkanRendererBackend();
  ~VulkanRendererBackend();
  DELETE_COPY_AND_ASSIGN(VulkanRendererBackend);
  DELETE_MOVE_AND_ASSIGN(VulkanRendererBackend);

  // The current image being rendered.
  uint32_t current_swap_index = 0;
  int current_frame = 0;

  std::unique_ptr<Context> context;

  // Non-constant elements (should be out of the context).

  struct Pipeline {
    Handle<VkRenderPass> render_pass = {};

    Handle<VkDescriptorSetLayout> descriptor_set_layout;
    Handle<VkPipelineLayout> pipeline_layout = {};

    Handle<VkDescriptorPool> descriptor_pool;
    std::vector<VkDescriptorSet>
        descriptor_sets;  // Freed with |descriptor_pool|.

    std::string vert_shader_path;
    std::string frag_shader_path;
    Handle<VkPipeline> pipeline = {};

    Handle<VkFramebuffer> frame_buffers[Definitions::kNumFrames];

    VkCommandBuffer command_buffers[Definitions::kNumFrames];
    VkCommandBuffer new_command_buffers[Definitions::kNumFrames];

    VkDeviceSize ubo_size;
    MemoryBacked<VkBuffer> uniform_buffers[Definitions::kNumFrames];

    // Actual geometry.
    MemoryBacked<VkBuffer> vertices;
    MemoryBacked<VkBuffer> indices;

    MemoryBacked<VkImage> texture;
    Handle<VkImageView> texture_view;
    Handle<VkSampler> texture_sampler;

    Handle<VkSemaphore> image_available_semaphores[Definitions::kNumFrames];
    Handle<VkSemaphore> render_finished_semaphores[Definitions::kNumFrames];
    Handle<VkFence> in_flight_fences[Definitions::kNumFrames];

    // Loaded models.
    struct LoadedMesh {
      MemoryBacked<VkBuffer> vertex_memory;
      MemoryBacked<VkBuffer> index_memory;
    };
  };
  Pipeline pipeline;
};

void CreateRenderPass(VulkanRendererBackend*);

void CreateDescriptorSetLayout(VulkanRendererBackend*);

void CreatePipelineLayout(VulkanRendererBackend*);

// |vert_shader_path| and |frag_shader_path| must be set at this call.
void CreateGraphicsPipeline(VulkanRendererBackend*);

void CreateFrameBuffers(VulkanRendererBackend*);

void LoadModel(VulkanRendererBackend*, const Mesh&);

void SetupUBO(VulkanRendererBackend* , VkDeviceSize ubo_size);

void CreateTextureBuffers(VulkanRendererBackend*, Image*);

void CreateTextureSampler(VulkanRendererBackend*, const Image&);

// This wil also create the descriptor pools.
void CreateDescriptorSets(VulkanRendererBackend*);

void CreateCommandBuffers(VulkanRendererBackend*);

void CreateSyncObjects(VulkanRendererBackend*);

// -----------------------------------------------------------------------------

void RecreateSwapChain(VulkanRendererBackend*, uint32_t width, uint32_t height);

}  // namespace vulkan
}  // namespace warhol
