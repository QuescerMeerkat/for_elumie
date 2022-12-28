#include "zx_renderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

namespace zx {

ZxRenderer::ZxRenderer(ZxWindow& window, ZxDevice& device)
    : zxWindow{window}, zxDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
}

ZxRenderer::~ZxRenderer() { freeCommandBuffers(); }

void ZxRenderer::recreateSwapChain() {
  auto extent = zxWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = zxWindow.getExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(zxDevice.device());

  if (zxSwapChain == nullptr) {
    zxSwapChain = std::make_unique<ZxSwapChain>(zxDevice, extent);
  } else {
    std::shared_ptr<ZxSwapChain> oldSwapChain = std::move(zxSwapChain);
    zxSwapChain = std::make_unique<ZxSwapChain>(zxDevice, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*zxSwapChain.get())) {
      panic("Swap chain image(or depth) format has changed!");
    }
  }
}

void ZxRenderer::createCommandBuffers() {
  commandBuffers.resize(ZxSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = zxDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(zxDevice.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    panic("Failed to allocate command buffers!");
  }
}

void ZxRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(
      zxDevice.device(),
      zxDevice.getCommandPool(),
      static_cast<uint32_t>(commandBuffers.size()),
      commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer ZxRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = zxSwapChain->acquireNextImage(&currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    panic("Failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    panic("Failed to begin recording command buffer!");
  }
  return commandBuffer;
}

void ZxRenderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    panic("Failed to record command buffer!");
  }

  auto result = zxSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      zxWindow.wasWindowResized()) {
    zxWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    panic("Failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % ZxSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void ZxRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = zxSwapChain->getRenderPass();
  renderPassInfo.framebuffer = zxSwapChain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = zxSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(zxSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(zxSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, zxSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void ZxRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't end render pass on command buffer from a different frame");
  vkCmdEndRenderPass(commandBuffer);
}

}
