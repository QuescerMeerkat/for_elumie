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
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(
      ZxDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  ZxDevice &zxDevice;

  std::unique_ptr<ZxPipeline> zxPipeline;
  VkPipelineLayout pipelineLayout;
};
}