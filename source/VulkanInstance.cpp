#include "VulkanInstance.h"

std::vector<VkExtensionProperties> VulkanInstance::extensions = getExtensions();
std::vector<VkLayerProperties> VulkanInstance::layers = getLayers();