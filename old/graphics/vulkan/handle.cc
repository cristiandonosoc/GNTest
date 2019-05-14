// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/handle.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/utils.h"

namespace warhol {
namespace vulkan {

template <>
void Handle<VkInstance>::InternalClear() {
  if (has_value() && context_)
    vkDestroyInstance(handle_, nullptr);
}

template <>
void Handle<VkDebugUtilsMessengerEXT>::InternalClear() {
  if (has_value() && context_)
    DestroyDebugUtilsMessengerEXT(*context_->instance, handle_, nullptr);
}

template <>
void Handle<VkSurfaceKHR>::InternalClear() {
  if (has_value() && context_)
    vkDestroySurfaceKHR(*context_->instance, handle_, nullptr);
}

template <>
void Handle<VkDevice>::InternalClear() {
  if (has_value() && context_)
    vkDestroyDevice(handle_, nullptr);
}

template <>
void Handle<VkSwapchainKHR>::InternalClear() {
  if (has_value() && context_)
    vkDestroySwapchainKHR(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkImage>::InternalClear() {
  if (has_value() && context_)
    vkDestroyImage(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkImageView>::InternalClear() {
  if (has_value() && context_)
    vkDestroyImageView(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkSampler>::InternalClear() {
  if (has_value() && context_)
    vkDestroySampler(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkRenderPass>::InternalClear() {
  if (has_value() && context_)
    vkDestroyRenderPass(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkPipelineLayout>::InternalClear() {
  if (has_value() && context_)
    vkDestroyPipelineLayout(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkPipeline>::InternalClear() {
  if (has_value() && context_)
    vkDestroyPipeline(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkFramebuffer>::InternalClear() {
  if (has_value() && context_)
    vkDestroyFramebuffer(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkCommandPool>::InternalClear() {
  if (has_value() && context_)
    vkDestroyCommandPool(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkCommandBuffer>::InternalClear() {
  if (has_value() && context_ && extra_handle_) {
    vkFreeCommandBuffers(*context_->device, (VkCommandPool)extra_handle_,
                         1, &handle_);
  }
}

template <>
void Handle<VkSemaphore>::InternalClear() {
  if (has_value() && context_)
    vkDestroySemaphore(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkFence>::InternalClear() {
  if (has_value() && context_)
    vkDestroyFence(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkBuffer>::InternalClear() {
  if (has_value() && context_)
    vkDestroyBuffer(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkDeviceMemory>::InternalClear() {
  if (has_value() && context_)
    vkFreeMemory(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkDescriptorSetLayout>::InternalClear() {
  if (has_value() && context_)
    vkDestroyDescriptorSetLayout(*context_->device, handle_, nullptr);
}

template <>
void Handle<VkDescriptorPool>::InternalClear() {
  if (has_value() && context_)
    vkDestroyDescriptorPool(*context_->device, handle_, nullptr);
}

}  // namespace vulkan
}  // namespace warhol
