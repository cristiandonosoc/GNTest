// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include "vulkan_shaders.h"
#include "vulkan_utils.h"

namespace warhol {

ShaderModuleContext::~ShaderModuleContext() {
  if (handle != VK_NULL_HANDLE) {
    assert(context);
    vkDestroyShaderModule(context->logical_device.handle, handle, nullptr);
  }
}

PipelineContext::PipelineContext(VulkanContext* context) : context(context) {}
PipelineContext::~PipelineContext() {
  if (layout != VK_NULL_HANDLE) {
    assert(context);
    vkDestroyPipelineLayout(context->logical_device.handle, layout, nullptr);
  }
}

Status
CreateShaderModule(VulkanContext* context, const std::string& src,
                   ShaderModuleContext* out) {
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

  out->context = context;
  out->handle = handle;

  return Status::Ok();
}

#if 0

Status
CreateGraphicsPipeline(VulkanContext* context, const std::string& vert_src,
                       const std::string& frag_src) {
  Status status;
  ShaderModuleContext vert_module = {};
  status = CreateShaderModule(context, vert_src, &vert_module);
  if (!status.ok())
    return status;

  VkPipelineShaderStageCreateInfo vert_create_info = {};
  vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_create_info.module = vert_module.handle;
  vert_create_info.pName = "main";

  ShaderModuleContext frag_module = {};
  status = CreateShaderModule(context, frag_src, &frag_module);
  if (!status.ok())
    return status;
  VkPipelineShaderStageCreateInfo frag_create_info = {};
  frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_create_info.module = frag_module.handle;
  frag_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_create_info,
                                                     frag_create_info};


}

#endif

}  // namespace warhol
