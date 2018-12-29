// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <vulkan/vulkan.h>
#include <warhol/utils/log.h>
#include <warhol/graphics/vulkan/utils.h>

using namespace warhol;

int main() {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Warhol Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instance_create_info = {};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &app_info;

  instance_create_info.enabledExtensionCount = 0;
  instance_create_info.enabledLayerCount = 0;


  VkInstance instance;
  if (auto res = vkCreateInstance(&instance_create_info, nullptr, &instance);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create instance: " << VulkanEnumToString(res);
    exit(1);
  }

  LOG(INFO) << "Success!";
}
