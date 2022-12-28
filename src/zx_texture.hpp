#pragma once

#include "defines.hpp"
#include "zx_device.hpp"

#include <string>

namespace zx{
  class Texture{
    public:
      Texture(ZxDevice& device, const std::string& filepath);
      ~Texture();

      Texture(const Texture &) = delete;
      Texture &operator=(const Texture &) = delete;
      Texture(Texture &&) = delete;
      Texture &operator=(Texture &&) = delete;

      VkSampler getSampler() { return sampler; }
      VkImageView getImageView() { return imageView; }
      VkImageLayout getImageLayout() { return imageLayout; }

    private:
      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
      void generateMipMaps();

      int width, height, mipLevels;
      ZxDevice& zxDevice;
      VkImage image;
      VkDeviceMemory imageMemory;
      VkImageView imageView;
      VkSampler sampler;
      VkFormat imageFormat;
      VkImageLayout imageLayout;
  };
}