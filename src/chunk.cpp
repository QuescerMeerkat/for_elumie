#include "chunk.hpp"

#include "zx_utils.hpp"
#include "zx_model.hpp"

#include <glm/gtc/noise.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

using Vertex = zx::Chunk::Vertex;

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    size_t seed = 0;
    zx::hashCombine(seed, vertex.position.x, vertex.position.y, vertex.position.z, vertex.color.x, vertex.color.y, vertex.color.z, vertex.normal.x, vertex.normal.y, vertex.normal.z);
    return seed;
  }
};
}

namespace zx {
Chunk::Chunk(ZxDevice& zxDevice) : zxDevice{zxDevice} {}

Chunk::~Chunk() {}

void Chunk::createVertexBuffers() {
  vertexCount = static_cast<uint32_t>(vertices.size());
  info(std::to_string(vertexCount), 1);
  assert(vertexCount >= 3 && "Vertex count must be at least 3!");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  ZxBuffer stagingBuffer{
      zxDevice,
      vertexSize,
      vertexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<ZxBuffer>(
      zxDevice,
      vertexSize,
      vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  zxDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void Chunk::createIndexBuffers() {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  ZxBuffer stagingBuffer{
      zxDevice,
      indexSize,
      indexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<ZxBuffer>(
      zxDevice,
      indexSize,
      indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  (zxDevice).copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}



void Chunk::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void Chunk::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

std::vector<VkVertexInputBindingDescription> Chunk::Vertex::getBindingDescriptions() {
std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
bindingDescriptions[0].binding = 0;
bindingDescriptions[0].stride = sizeof(Vertex);
bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Chunk::Vertex::getAttributeDescriptions() {
std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

return attributeDescriptions;
}

void Chunk::create_mesh(glm::vec2 pos){
  vertices.clear();
  indices.clear();

  static const float voxel_vertices[] = {
  0, 0, 0,
  1, 0, 0,
  1, 1, 0,
  0, 1, 0,

  0, 0, 1,
  1, 0, 1,
  1, 1, 1,
  0, 1, 1
  };

  static int sz_vv = sizeof(voxel_vertices)/sizeof(voxel_vertices[0]);

  static const uint32_t voxel_indices[] = {
  1, 0, 3, 1, 3, 2, // north (-z)
  4, 5, 6, 4, 6, 7, // south (+z)
  5, 1, 2, 5, 2, 6, // east (+x)
  0, 4, 7, 0, 7, 3, // west (-x)
  2, 3, 7, 2, 7, 6, // top (+y)
  5, 4, 0, 5, 0, 1, // bottom (-y)
  };  

  static const glm::vec3 voxel_normals[] = { {0.f, 0.f, -1.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, -1.f, 0.f} };
  static int sz_vn = sizeof(voxel_normals)/sizeof(voxel_normals[0]);
  
  static int sz_vi = sizeof(voxel_indices)/sizeof(voxel_indices[0]);

  int j = 0;
  for(int y = 0; y < CHUNK_SIZE; y++) {
    for(int z = 0; z < CHUNK_SIZE; z++) {
      for(int x = 0; x < CHUNK_SIZE; x++) {
        for(int i = 0, k = 0; i < sz_vv; i+=3, i%6==0 ? k++ : k=k){
          Vertex vertex;

          float xx = voxel_vertices[i]+x;
          float yy = voxel_vertices[i+1]+y;
          float zz = voxel_vertices[i+2]+z;
          vertex.position = { xx, yy, zz};
          
          glm::vec3 red = { 0.8f, 0.2f, 0.25f };
          glm::vec3 gray = { 0.4f, 0.4f, 0.4f };
          glm::vec3 green = { 0.1f, 0.9f, 0.2f };
          glm::vec3 yellow = { 0.8f, 0.8f, 0.1f };
          glm::vec3 border = { 0.2, 0.9f, 0.3f };
          Voxel voxel = voxels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
          voxel = voxels[j];
          vertex.color = voxel == stone ? gray : voxel == grass ? green : voxel == sand ? yellow : red;
          if((x == 0 || x == (CHUNK_SIZE-1) || z == 0 || z == (CHUNK_SIZE-1)) && (vertex.color != red)){
            vertex.color = border;
          }

          glm::vec3 vn = voxel_normals[k];
          vertex.normal = vn;
          
          vertices.push_back(vertex);
        }
        for(int i = 0; i < sz_vi; i++){
          indices.push_back(voxel_indices[i]+8*j);
        }
        j++;
      } // x
    } // z
  } // y
  
  createVertexBuffers();
  createIndexBuffers();
}
}