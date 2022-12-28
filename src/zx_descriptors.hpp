#pragma once

#include "defines.hpp"
#include "zx_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace zx {

class ZxDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(ZxDevice &zxDevice) : zxDevice{zxDevice} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<ZxDescriptorSetLayout> build() const;

   private:
    ZxDevice &zxDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  ZxDescriptorSetLayout(
      ZxDevice &zxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~ZxDescriptorSetLayout();
  ZxDescriptorSetLayout(const ZxDescriptorSetLayout &) = delete;
  ZxDescriptorSetLayout &operator=(const ZxDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  ZxDevice &zxDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class ZxDescriptorWriter;
};

class ZxDescriptorPool {
 public:
  class Builder {
   public:
    Builder(ZxDevice &zxDevice) : zxDevice{zxDevice} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<ZxDescriptorPool> build() const;

   private:
    ZxDevice &zxDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  ZxDescriptorPool(
      ZxDevice &zxDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~ZxDescriptorPool();
  ZxDescriptorPool(const ZxDescriptorPool &) = delete;
  ZxDescriptorPool &operator=(const ZxDescriptorPool &) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  ZxDevice &zxDevice;
  VkDescriptorPool descriptorPool;

  friend class ZxDescriptorWriter;
};

class ZxDescriptorWriter {
 public:
  ZxDescriptorWriter(ZxDescriptorSetLayout &setLayout, ZxDescriptorPool &pool);

  ZxDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  ZxDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  ZxDescriptorSetLayout &setLayout;
  ZxDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}
