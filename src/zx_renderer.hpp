#pragma once

#include "defines.hpp"
#include "zx_device.hpp"
#include "zx_swap_chain.hpp"
#include "zx_window.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace zx {
class ZxRenderer {
 public:
  ZxRenderer(ZxWindow &window, ZxDevice &device);
  ~ZxRenderer();

  ZxRenderer(const ZxRenderer &) = delete;
  ZxRenderer &operator=(const ZxRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const { return zxSwapChain->getRenderPass(); }
  float getAspectRatio() const { return zxSwapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  ZxWindow &zxWindow;
  ZxDevice &zxDevice;
  std::unique_ptr<ZxSwapChain> zxSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
}
