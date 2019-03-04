// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/renderer_backend_impl.h"

#include <iostream>

#include "warhol/assets/assets.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/image_utils.h"
#include "warhol/graphics/vulkan/renderer_backend.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/assert.h"

#include "warhol/graphics/common/render_command.h"
#include "warhol/graphics/common/image.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/vulkan/memory.h"
#include "warhol/graphics/vulkan/utils.h"


#include "warhol/utils/file.h"
#include "warhol/utils/scope_trigger.h"

namespace warhol {
namespace vulkan {

// InitVulkanRendererBackend ---------------------------------------------------

namespace {

inline void Header(const char* header) {
  std::cout << "\n*** " << header
            << " **************************************************************"
               "**************************\n\n";
  std::flush(std::cout);
}

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

}  // namespace

void VulkanBackendInitImpl(VulkanRendererBackend* vulkan,
                           WindowManager* window) {
  vulkan->context = std::make_unique<vulkan::Context>();
  InitVulkanContext(vulkan->context.get(), window);

  Header("Creating a render pass...");
  CreateRenderPass(vulkan);

  Header("Creating descriptor set layout...");
  CreateDescriptorSetLayout(vulkan);

  Header("Creating the pipeline layout...");
  CreatePipelineLayout(vulkan);

  Header("Creating a graphics pipeline...");
  vulkan->pipeline.vert_shader_path = Assets::VulkanShaderPath("demo.vert.spv");
  vulkan->pipeline.frag_shader_path = Assets::VulkanShaderPath("demo.frag.spv");
  CreateGraphicsPipeline(vulkan);

  Header("Creating frame buffers...");
  CreateFrameBuffers(vulkan);

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

  LoadModel(vulkan, mesh);

  Header("Setting up UBO...");
  SetupUBO(vulkan, sizeof(UBO));

  Image image =
      Image::Create2DImageFromPath(Assets::TexturePath("awesomeface.png"));
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
  CreateTextureBuffers(vulkan, &image);

  Header("Creating texture sampler...");
  CreateTextureSampler(vulkan, image);

  Header("Creating descriptor sets...");
  CreateDescriptorSets(vulkan);

  Header("Creating command buffers....");
  CreateCommandBuffers(vulkan);

  Header("Creating synchronization objects...");
  CreateSyncObjects(vulkan);

  LOG(INFO) << "Vulkan context creation successful!";
}

// VulkanBackendStartFrame -----------------------------------------------------

void VulkanBackendStartFrame(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  ASSERT(context);

  // We wait for the fences that say this image is still in flight.
  int current_frame = vulkan->current_frame;
  VK_CHECK(vkWaitForFences, *context->device, 1,
           &vulkan->pipeline.in_flight_fences[current_frame].value(),
           VK_TRUE, UINT64_MAX);

  // We acquire the next image to render to.
  uint32_t current_swap_index;
  VkSemaphore semaphore =
      *vulkan->pipeline.image_available_semaphores[current_frame];
  VK_CHECK(vkAcquireNextImageKHR, *context->device, *context->swap_chain,
                                  UINT64_MAX,   // No timeout.
                                  semaphore,    // Semaphore to signal
                                  nullptr,      // No fence to wait on.
                                  &current_swap_index);
  vulkan->current_swap_index = current_swap_index;

  // We empty the garbage for the frame and flush any in transit buffers.
  EmptyGarbage(&context->allocator);
  Flush(&context->staging_manager);

  // Reset the fences for this frame.
  VkFence fence = *vulkan->pipeline.in_flight_fences[current_frame];
  VK_CHECK(vkResetFences, *context->device, 1, &fence);

  // Start the command buffer.
  auto command_buffer = vulkan->pipeline.command_buffers[current_frame];
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer, command_buffer, &begin_info);

  /*****************************************************************************
   *
   * TODO(Cristian): Check out this memory barriers here....
   *
   ****************************************************************************/

  // TODO(Cristian): Check query pools

  VkRenderPassBeginInfo render_pass_begin = {};
  render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin.renderPass = *vulkan->pipeline.render_pass;
  render_pass_begin.framebuffer =
      *vulkan->pipeline.frame_buffers[current_swap_index];
  render_pass_begin.renderArea.extent = context->swap_chain_details.extent;

  // TODO(Cristian): Should we handle clearing here?

  vkCmdBeginRenderPass(command_buffer, &render_pass_begin,
                       VK_SUBPASS_CONTENTS_INLINE);
}

// VulkanBackendDrawMesh -------------------------------------------------------

void VulkanBackendDrawMesh(VulkanRendererBackend* vulkan, RenderCommand* cmd) {
  VkCommandBuffer command_buffer =
      vulkan->pipeline.new_command_buffers[vulkan->current_frame];

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *vulkan->pipeline.pipeline);

  Mesh* mesh = cmd->mesh;
  ASSERT(mesh->loaded());

  auto it = vulkan->loaded_meshes.find(mesh->loaded_token);
  ASSERT(it != vulkan->loaded_meshes.end());
  VulkanRendererBackend::LoadedMesh& loaded_mesh = it->second;

  VkBuffer vertex_buffers[] = {
    *loaded_mesh.vertices.handle,
  };
  VkDeviceSize offsets[] = {
      0,
  };

  vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(command_buffer, *loaded_mesh.indices.handle, 0,
                       VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(command_buffer, (uint32_t)mesh->indices.size(), 1, 0, 0, 0);
}

// VulkanBackendEndFrame -------------------------------------------------------

void VulkanBackendEndFrame(VulkanRendererBackend*) {
  NOT_IMPLEMENTED();
}

// **** Impl *******************************************************************


// CreateRenderPass ------------------------------------------------------------

void CreateRenderPass(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  // Represents how our buffer will relate from/to the frame buffers.
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

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = context->depth_format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // What layouts the data has to be before and after using the render target.
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_ref = {};
  depth_ref.attachment = 1;
  depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  // This is a graphics subpass.
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_ref;

  // Create a dependency for this render pass.
  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[] = {
    color_attachment,
    depth_attachment,
  };

  VkRenderPassCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = ARRAY_SIZE(attachments);
  create_info.pAttachments = attachments;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &dependency;

  VkRenderPass render_pass;
  VK_CHECK(vkCreateRenderPass, *context->device, &create_info, nullptr,
                               &render_pass);
  vulkan->pipeline.render_pass.Set(context, render_pass);
}

// CreateDescriptorSetLayout ---------------------------------------------------

void CreateDescriptorSetLayout(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  VkDescriptorSetLayoutBinding bindings[2] = {};

  // UBO binding
  auto& ubo_binding = bindings[0];
  ubo_binding.binding = 0;
  ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_binding.descriptorCount = 1;
  ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_binding.pImmutableSamplers = nullptr;  // Optional.

  auto& sampler_binding = bindings[1];
  sampler_binding.binding = 1;
  sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_binding.descriptorCount = 1;
  sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  ubo_binding.pImmutableSamplers = nullptr;  // Optional.

  VkDescriptorSetLayoutCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  create_info.bindingCount = ARRAY_SIZE(bindings);
  create_info.pBindings = bindings;

  VkDescriptorSetLayout descriptor_set_layout;
  VK_CHECK(vkCreateDescriptorSetLayout, *context->device, &create_info,
                                        nullptr, &descriptor_set_layout);

  vulkan->pipeline.descriptor_set_layout.Set(context, descriptor_set_layout);
}

// CreatePipelineLayout --------------------------------------------------------

void CreatePipelineLayout(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();

  VkPipelineLayoutCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = 1;
  create_info.pSetLayouts = &vulkan->pipeline.descriptor_set_layout.value();
  create_info.pushConstantRangeCount = 0; // Optional
  create_info.pPushConstantRanges = nullptr; // Optional

  VkPipelineLayout pipeline_layout;
  VK_CHECK(vkCreatePipelineLayout, *context->device, &create_info, nullptr,
                                   &pipeline_layout);
  vulkan->pipeline.pipeline_layout.Set(context, pipeline_layout);
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

  VkVertexInputBindingDescription binding = {};
  binding.binding = 0;
  binding.stride = sizeof(Vertex);
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  bindings.emplace_back(std::move(binding));

  return bindings;
}

std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> descriptions;
  descriptions.reserve(3);

  VkVertexInputAttributeDescription pos_desc = {};
  pos_desc.binding = 0;
  pos_desc.location = 0;
  pos_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  pos_desc.offset = 0;
  descriptions.push_back(std::move(pos_desc));

  VkVertexInputAttributeDescription color_desc = {};
  color_desc.binding = 0;
  color_desc.location = 1;
  color_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  color_desc.offset = offsetof(Vertex, color);
  descriptions.push_back(std::move(color_desc));

  VkVertexInputAttributeDescription uv_desc = {};
  uv_desc.binding = 0;
  uv_desc.location = 2;
  uv_desc.format = VK_FORMAT_R32G32_SFLOAT;
  uv_desc.offset = offsetof(Vertex, uv);
  descriptions.push_back(std::move(uv_desc));

  return descriptions;
}

}  // namespace

void CreateGraphicsPipeline(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  std::vector<char> vert_data, frag_data;
  if (!ReadWholeFile(vulkan->pipeline.vert_shader_path, &vert_data, false) ||
      !ReadWholeFile(vulkan->pipeline.frag_shader_path, &frag_data, false)) {
    NOT_REACHED("Unable to read shaders.");
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
    NOT_REACHED("Null shader modules.");

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
  LOG(DEBUG) << "Bindings size: " << bindings.size();
  vertex_input.vertexBindingDescriptionCount = bindings.size();
  vertex_input.pVertexBindingDescriptions = bindings.data();
  LOG(DEBUG) << "Attributes size: " << attributes.size();
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
  //
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

  // ****** Depth Buffer ******

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
  depth_stencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.minDepthBounds = 0.0f;  // Optional.
  depth_stencil.maxDepthBounds = 1.0f;  // Optional.

  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.front = {};   // Optional.
  depth_stencil.back = {};    // Optional.

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
  create_info.pDepthStencilState = &depth_stencil;
  create_info.pColorBlendState = &color_blend_state;
  create_info.pDynamicState = nullptr; // Optional

  // Pipeline layout.
  create_info.layout = *vulkan->pipeline.pipeline_layout;

  // Render pass.
  create_info.renderPass = *vulkan->pipeline.render_pass;
  create_info.subpass = 0;  // Index of the subpass to use.

  // These are used to create pipelines from previous ones.
  create_info.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  create_info.basePipelineIndex = -1;  // Optional

  VkPipeline pipeline;
  VK_CHECK(vkCreateGraphicsPipelines, *context->device, nullptr,
                                      1, &create_info, nullptr, &pipeline);
  vulkan->pipeline.pipeline.Set(context, pipeline);
}

// CreateFrameBuffers ----------------------------------------------------------

void CreateFrameBuffers(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  for (size_t i = 0; i < ARRAY_SIZE(vulkan->pipeline.frame_buffers); i++) {
    // A framebuffer references image views for input data.
    VkImageView attachments[] = {
      *context->image_views[i],
      *context->depth_image_view,
    };

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = *vulkan->pipeline.render_pass;
    create_info.attachmentCount = ARRAY_SIZE(attachments);
    create_info.pAttachments = attachments;
    create_info.width = context->swap_chain_details.extent.width;
    create_info.height = context->swap_chain_details.extent.height;
    create_info.layers = 1;

    VkFramebuffer frame_buffer;
    VK_CHECK(vkCreateFramebuffer, *context->device, &create_info, nullptr,
                                  &frame_buffer);

    vulkan->pipeline.frame_buffers[i].Set(context, frame_buffer);
  }
}

// Load Model ------------------------------------------------------------------

namespace {

void CreateVertexBuffers(VulkanRendererBackend* vulkan, const Mesh& mesh) {
  Context* context = vulkan->context.get();
  VkDeviceSize size = mesh.vertices.size() * sizeof(mesh.vertices[0]);

  StageToken stage_token = Stage(&context->staging_manager, size, 1);
  CopyIntoStageToken(&stage_token, (void*)mesh.vertices.data(), size);

  // Create the local memory and copy the memory to it.
  AllocBufferConfig alloc_config = {};
  alloc_config.size = size;
  alloc_config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  alloc_config.memory_usage = MemoryUsage::kGPUOnly;
  MemoryBacked<VkBuffer> vertices_memory = AllocBuffer(context, &alloc_config);
  ASSERT(vertices_memory.has_value());

  CopyStageTokenToBuffer(&stage_token, *vertices_memory.handle, 0);
  vulkan->pipeline.vertices = std::move(vertices_memory);
}

static size_t indices_count = 0;

void CreateIndicesBuffers(VulkanRendererBackend* vulkan, const Mesh& mesh) {
  Context* context = vulkan->context.get();
  LOG(DEBUG) << "Creating index buffer.";
  VkDeviceSize size = mesh.indices.size() * sizeof(mesh.indices[0]);

  StageToken stage_token = Stage(&context->staging_manager, size, 1);
  CopyIntoStageToken(&stage_token, (void*)mesh.indices.data(), size);

  // Create the local memory and copy the memory to it.
  AllocBufferConfig alloc_config = {};
  alloc_config.size = size;
  alloc_config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  alloc_config.memory_usage = MemoryUsage::kGPUOnly;
  MemoryBacked<VkBuffer> indices_memory = AllocBuffer(context, &alloc_config);
  ASSERT(indices_memory.has_value());

  CopyStageTokenToBuffer(&stage_token, *indices_memory.handle, 0);

  indices_count = mesh.indices.size();

  // The stating buffers will be freed by Handle<>
  vulkan->pipeline.indices = std::move(indices_memory);
}

}  // namespace

void LoadModel(VulkanRendererBackend* vulkan, const Mesh& mesh) {
  CreateVertexBuffers(vulkan, mesh);
  CreateIndicesBuffers(vulkan, mesh);
  Flush(&vulkan->context->staging_manager);
}

// SetupUBO --------------------------------------------------------------------

void SetupUBO(VulkanRendererBackend* vulkan, VkDeviceSize ubo_size) {
  Context* context = vulkan->context.get();
  vulkan->pipeline.ubo_size = ubo_size;
  for (size_t i = 0; i < ARRAY_SIZE(context->images); i++) {
    LOG(DEBUG) << "Allocating UBO " << i
               << ", size: " << BytesToString(ubo_size);

    AllocBufferConfig alloc_config = {};
    alloc_config.size = ubo_size;
    alloc_config.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    alloc_config.memory_usage = MemoryUsage::kCPUToGPU;
    MemoryBacked<VkBuffer> ubo = AllocBuffer(context, &alloc_config);
    ASSERT(ubo.has_value());

    vulkan->pipeline.uniform_buffers[i] = std::move(ubo);
  }
}

// CreateTextureBuffers --------------------------------------------------------

void CreateTextureBuffers(VulkanRendererBackend* vulkan, Image* image) {
  Context* context = vulkan->context.get();
  StageToken token = Stage(&context->staging_manager, image->data_size, 1);
  CopyIntoStageToken(&token, (void*)image->data.value, image->data_size);

  // Allocate and image.
  AllocImageConfig alloc_image_config = {};
  VkImageCreateInfo& image_info = alloc_image_config.create_info;
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.format = ToVulkan(image->format);
  image_info.imageType = ToVulkan(image->type);
  image_info.extent = { (uint32_t)image->width, (uint32_t)image->height, 1 };
  image_info.mipLevels = image->mip_levels;
  image_info.arrayLayers = 1;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                      VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  alloc_image_config.memory_usage = MemoryUsage::kGPUOnly;
  // If we're creating mip levels, this image will also be a source for copy
  // operations.
  if (alloc_image_config.create_info.mipLevels > 1)
    alloc_image_config.create_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  CreateImageViewConfig image_view_config = {};
  /* image_view_config.image = *context->texture.handle; */
  image_view_config.format = ToVulkan(image->format);
  image_view_config.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_config.mip_levels = image->mip_levels;

  TransitionImageLayoutConfig transition_config = {};
  transition_config.format = VK_FORMAT_B8G8R8A8_UNORM;
  transition_config.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  transition_config.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  transition_config.mip_levels = image->mip_levels;

  CreateImageConfig create_image_config = {};
  create_image_config.alloc_config = std::move(alloc_image_config);
  create_image_config.view_config = std::move(image_view_config);
  create_image_config.transition_config = std::move(transition_config);

  auto created_image = CreateImage(context, &create_image_config);
  ASSERT(created_image.valid());

  CopyStageTokenToImage(&token, image, *created_image.image.handle);
  Flush(&context->staging_manager);

  LOG(DEBUG) << "Generating mip maps.";

  // Generate all the mipmaps for this texture.
  // Generating the mipmaps will take care of the transition to
  // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for us.
  GenerateMipmapsConfig mipmap_config = {};
  mipmap_config.image = *created_image.image.handle;
  mipmap_config.format = ToVulkan(image->format);
  mipmap_config.width = image->width;
  mipmap_config.height = image->height;
  mipmap_config.mip_levels = image->mip_levels;
  ASSERT(GenerateMipmaps(context, &mipmap_config));

  vulkan->pipeline.texture = std::move(created_image.image);
  vulkan->pipeline.texture_view = std::move(created_image.image_view);
}

// CreateTextureSampler --------------------------------------------------------

void CreateTextureSampler(VulkanRendererBackend* vulkan, const Image& image) {
  Context* context = vulkan->context.get();
  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.magFilter = VK_FILTER_LINEAR;

  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = 16;

  sampler_info.unnormalizedCoordinates = VK_FALSE;

  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = (float)image.mip_levels;

  VkSampler sampler_handle;
  VK_CHECK(vkCreateSampler, *context->device, &sampler_info, nullptr,
                            &sampler_handle);

  vulkan->pipeline.texture_sampler = {context, sampler_handle};
}

// CreateDescriptorSets  -------------------------------------------------------

namespace {

void CreateDescriptorPool(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();
  VkDescriptorPoolSize pool_sizes[2] = {};
  auto& uniform_size = pool_sizes[0];
  uniform_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniform_size.descriptorCount = ARRAY_SIZE(context->images);

  auto& sampler_size = pool_sizes[1];
  sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_size.descriptorCount = ARRAY_SIZE(context->images);

  VkDescriptorPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  create_info.poolSizeCount = ARRAY_SIZE(pool_sizes);
  create_info.pPoolSizes = pool_sizes;
  create_info.maxSets = ARRAY_SIZE(context->images);

  VkDescriptorPool pool;
  VK_CHECK(vkCreateDescriptorPool, *context->device, &create_info, nullptr,
                                   &pool);
  vulkan->pipeline.descriptor_pool.Set(context, pool);
}

}  // namespace

void CreateDescriptorSets(VulkanRendererBackend* vulkan) {
  CreateDescriptorPool(vulkan);
  Context* context = vulkan->context.get();

  // The layout could be different for each descriptor set we're allocating.
  // We point to the same layout everytime.
  std::vector<VkDescriptorSetLayout> layouts(
      ARRAY_SIZE(context->images), *vulkan->pipeline.descriptor_set_layout);

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = *vulkan->pipeline.descriptor_pool;
  alloc_info.descriptorSetCount = layouts.size();
  alloc_info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> descriptor_sets(layouts.size());
  VK_CHECK(vkAllocateDescriptorSets, *context->device, &alloc_info,
                                     descriptor_sets.data());

  // We now associate our descriptor sets to a uniform buffer.
  for (size_t i = 0; i < layouts.size(); i++) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = *vulkan->pipeline.uniform_buffers[i].handle;
    buffer_info.offset = 0;
    buffer_info.range = vulkan->pipeline.ubo_size;

    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = *vulkan->pipeline.texture_view;
    image_info.sampler = *vulkan->pipeline.texture_sampler;

    VkWriteDescriptorSet descriptor_writes[2] = {};

    auto& buffer_write = descriptor_writes[0];
    buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    buffer_write.dstSet = descriptor_sets[i];
    buffer_write.dstBinding = 0;
    buffer_write.dstArrayElement = 0;
    buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    buffer_write.descriptorCount = 1;
    buffer_write.pBufferInfo = &buffer_info;

    auto& texture_write = descriptor_writes[1];
    texture_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    texture_write.dstSet = descriptor_sets[i];
    texture_write.dstBinding = 1;
    texture_write.dstArrayElement = 0;
    texture_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_write.descriptorCount = 1;
    texture_write.pImageInfo = &image_info;

    // We could also do one big call with an array of descriptor writes.
    vkUpdateDescriptorSets(*context->device, 2, descriptor_writes, 0, nullptr);
  }

  vulkan->pipeline.descriptor_sets = descriptor_sets;
}

// CreateCommandBuffers --------------------------------------------------------

void CreateCommandBuffers(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();

  // Command buffers can get multiple allocated at once with one call.
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = *context->command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = ARRAY_SIZE(vulkan->pipeline.command_buffers);

  VK_CHECK(vkAllocateCommandBuffers, *context->device, &alloc_info,
                                     vulkan->pipeline.command_buffers);

  VK_CHECK(vkAllocateCommandBuffers,
           *context->device,
           &alloc_info,
           vulkan->pipeline.new_command_buffers);

  // Start a command buffer recording.
  for (size_t i = 0; i < ARRAY_SIZE(vulkan->pipeline.command_buffers); i++) {
    VkCommandBuffer& command_buffer = vulkan->pipeline.command_buffers[i];

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    begin_info.pInheritanceInfo = nullptr;  // Optional

    VK_CHECK(vkBeginCommandBuffer, command_buffer, &begin_info);

    VkRenderPassBeginInfo render_pass_begin = {};
    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.renderPass = *vulkan->pipeline.render_pass;
    render_pass_begin.framebuffer = *vulkan->pipeline.frame_buffers[i];
    render_pass_begin.renderArea.offset = {0, 0};
    render_pass_begin.renderArea.extent = context->swap_chain_details.extent;

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0.7f, 0.3f, 0.5f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_begin.clearValueCount = ARRAY_SIZE(clear_values);
    render_pass_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer,
                         &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    {
      vkCmdBindPipeline(command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        *vulkan->pipeline.pipeline);

      // We are binding the same buffer in two different points.
      VkBuffer vertex_buffers[] = {
          *vulkan->pipeline.vertices.handle,
      };
      VkDeviceSize offsets[] = {
        0,
      };
      vkCmdBindVertexBuffers(command_buffer, 0,
                             ARRAY_SIZE(vertex_buffers), vertex_buffers, offsets);
      vkCmdBindIndexBuffer(command_buffer, *vulkan->pipeline.indices.handle, 0,
                           VK_INDEX_TYPE_UINT32);

      // We bind the descriptor sets.
      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              *vulkan->pipeline.pipeline_layout, 0, 1,
                              &vulkan->pipeline.descriptor_sets[i], 0, nullptr);

      vkCmdDrawIndexed(command_buffer, (uint32_t)indices_count, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(command_buffer);

    VK_CHECK(vkEndCommandBuffer, command_buffer);
  }
}

// CreateSyncObjects -----------------------------------------------------------

void CreateSyncObjects(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();

  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  // Start this fence in the signaled state.
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < Definitions::kMaxFramesInFlight; i++) {
    VkSemaphore semaphores[2];
    VkFence fence;
    VK_CHECK(vkCreateSemaphore, *context->device, &semaphore_create_info,
                                nullptr, &semaphores[0]);
    VK_CHECK(vkCreateSemaphore, *context->device, &semaphore_create_info,
                                nullptr, &semaphores[1]);
    VK_CHECK(vkCreateFence, *context->device, &fence_create_info, nullptr,
                            &fence);

    vulkan->pipeline.image_available_semaphores[i].Set(context, semaphores[0]);
    vulkan->pipeline.render_finished_semaphores[i].Set(context, semaphores[1]);
    vulkan->pipeline.in_flight_fences[i].Set(context, fence);
  }
}

// VulkanBackendRecreateSwapChain ----------------------------------------------

namespace {

// We free the resources in the right order before recreating.
void ClearOldSwapChain(VulkanRendererBackend* vulkan) {
  Context* context = vulkan->context.get();

  // We don't recreate the command pool, so we just need to free the command
  // buffers.
  vkFreeCommandBuffers(*context->device,
                       *context->command_pool,
                       (uint32_t)ARRAY_SIZE(vulkan->pipeline.command_buffers),
                       vulkan->pipeline.command_buffers);

  context->swap_chain.Clear();
  for (size_t i = 0; i < ARRAY_SIZE(context->image_views); i++)
    context->image_views[i].Clear();

  vulkan->pipeline.pipeline.Clear();
  vulkan->pipeline.pipeline_layout.Clear();
  vulkan->pipeline.render_pass.Clear();
}

}  // namespace

void VulkanBackendRecreateSwapChain(VulkanRendererBackend* vulkan,
                                    Pair<uint32_t> screen_size) {
  Context* context = vulkan->context.get();
  VK_CHECK(vkDeviceWaitIdle, *context->device);

  ClearOldSwapChain(vulkan);

  RecreateSwapChain(context, screen_size);

  CreateRenderPass(vulkan);
  CreatePipelineLayout(vulkan);
  CreateGraphicsPipeline(vulkan);
  CreateFrameBuffers(vulkan);
  CreateCommandBuffers(vulkan);
}

}  // namespace vulkan
}  // namespace warhol
