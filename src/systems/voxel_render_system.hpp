#pragma once

#include "../defines.hpp"
#include "../zx_camera.hpp"
#include "../zx_device.hpp"
#include "../zx_frame_info.hpp"
#include "../zx_game_object.hpp"
#include "../zx_pipeline.hpp"

#include <memory>
#include <vector>

namespace zx {
class VoxelRenderSystem {
 public:
  VoxelRenderSystem(
      ZxDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~VoxelRenderSystem();

  VoxelRenderSystem(const VoxelRenderSystem &) = delete;
  VoxelRenderSystem &operator=(const VoxelRenderSystem &) = delete;

  void renderChunks(FrameInfo& frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  ZxDevice &zxDevice;

  std::unique_ptr<ZxPipeline> zxPipeline;
  VkPipelineLayout pipelineLayout;
};
}