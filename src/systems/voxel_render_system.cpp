#include "voxel_render_system.hpp"

#include "chunk.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace zx {

struct VoxelPushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

VoxelRenderSystem::VoxelRenderSystem(
    ZxDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : zxDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

VoxelRenderSystem::~VoxelRenderSystem() {
  vkDestroyPipelineLayout(zxDevice.device(), pipelineLayout, nullptr);
}

void VoxelRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(VoxelPushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(zxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    panic("Failed to create pipeline layout!");
  }
}

void VoxelRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

  PipelineConfigInfo pipelineConfig{};
  ZxPipeline::defaultPipelineConfigInfo(pipelineConfig, Chunk::Vertex::getBindingDescriptions(), Chunk::Vertex::getAttributeDescriptions());
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  zxPipeline = std::make_unique<ZxPipeline>(
      zxDevice,
      "shaders/voxel_shader.vert.spv",
      "shaders/voxel_shader.frag.spv",
      pipelineConfig);
}

void VoxelRenderSystem::renderChunks(FrameInfo& frameInfo) {
  zxPipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr);

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.chunk == nullptr) continue;
    VoxelPushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(VoxelPushConstantData),
        &push);
    obj.chunk->bind(frameInfo.commandBuffer);
    obj.chunk->draw(frameInfo.commandBuffer);
  }
}
}