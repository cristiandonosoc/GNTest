// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <vulkan/vulkan.h>

int main() {

  uint32_t extension_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

  printf("Hello Vulkan!\n");
  printf("Extensions supported: %d\n", extension_count);
}

