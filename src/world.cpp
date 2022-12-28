#include "world.hpp"

#include <iostream>

namespace zx{

World::World(ZxDevice& zxDevice) : zxDevice{zxDevice}{}
World::~World(){}

float rounded(const glm::vec2& coord){
    auto bump = [](float t) { return glm::max(0.0f, 1.0f - std::pow(t, 6.0f)); };
    float b = bump(coord.x) * bump(coord.y);
    return b * 0.9f;
}

float getNoiseAt(const glm::vec2& voxel_position, const glm::vec2& chunk_position,
                  const NoiseSettings& settings, int seed){
  float voxel_x = voxel_position.x + chunk_position.x * CHUNK_SIZE;
  float voxel_z = voxel_position.y + chunk_position.y * CHUNK_SIZE;

  float value = 0;
  float accumulated_amplitudes = 0;
  for (int i = 0; i < settings.octaves; i++) {
      float frequency = glm::pow(2.0f, i);
      float amplitude = glm::pow(settings.roughness, i);

      float x = voxel_x * frequency / settings.smoothness;
      float y = voxel_z * frequency / settings.smoothness;

      float noise = glm::simplex(glm::vec3{seed + x, seed + y, seed});
      noise = (noise + 1.0f) / 2.0f;
      value += noise * amplitude;
      accumulated_amplitudes += amplitude;
  }
  return value / accumulated_amplitudes;
}

void World::createChunkHeightMap(const glm::vec3& position, int worldSize, int seed){

    const float world_size = static_cast<float>(world_size) * CHUNK_SIZE;

    NoiseSettings firstNoise;
    firstNoise.amplitude = 105;
    firstNoise.octaves = 6;
    firstNoise.smoothness = 205.f;
    firstNoise.roughness = 0.58f;
    firstNoise.offset = 18;

    NoiseSettings secondNoise;
    secondNoise.amplitude = 20;
    secondNoise.octaves = 4;
    secondNoise.smoothness = 200;
    secondNoise.roughness = 0.45f;
    secondNoise.offset = 0;

    glm::vec2 chunkXZ = {position.x, position.z};

    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            float bx = static_cast<float>(x + position.x * CHUNK_SIZE);
            float bz = static_cast<float>(z + position.z * CHUNK_SIZE);

            glm::vec2 coord =
                (glm::vec2{bx, bz} - world_size / 2.0f) / world_size * 2.0f;

            auto noise = getNoiseAt({x, z}, chunkXZ, firstNoise, seed);
            auto noise2 =
                getNoiseAt({x, z}, {position.x, position.z}, secondNoise, seed);
            auto island = rounded(coord) * 1.25;
            float result = noise * noise2;
            height_map[z * CHUNK_SIZE + x] =
                static_cast<int>((result * firstNoise.amplitude + firstNoise.offset) *
                                  island) - 5;                        
        }
    }
}

float generateSeed(const std::string& input){
    std::hash<std::string> strhash;

    float seed_float;
    uint32_t hash = strhash(input);
    std::memcpy(&seed_float, &hash, sizeof(float));
    return seed_float;
}

void World::createTerrain(const glm::vec2& chunk_pos){
  for (int z = 0; z < CHUNK_SIZE; z++) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        int height = height_map[z * CHUNK_SIZE + x];
        for (int y = 0; y < CHUNK_SIZE; y++) {
            int voxel_y = chunk_pos.y * CHUNK_SIZE + y;
            Voxel voxel = air;

            if(voxel_y <= height){
              if(voxel_y > WATER_LEVEL){
                voxel = stone;
              }
              else if(voxel_y == WATER_LEVEL){
                voxel = grass;
              }
              else{
                voxel = stone;
              }
            }
            chunks[chunk_pos.x + chunk_pos.y * CHUNK_SIZE].voxels.push_back(voxel);
        }
    }
  }
  chunks[chunk_pos.x + chunk_pos.y * CHUNK_SIZE].create_mesh(chunk_pos);
}

void World::generateTerrain(glm::vec2& chunk_pos, uint32_t worldSize){
    glm::vec3 position{chunk_pos.x, 0, chunk_pos.y}; // y is actually z LOL
    createChunkHeightMap(position, worldSize, generateSeed("my seed"));

    std::cout << "Creating terrain..." << std::endl;
    for (int y = 0; y < 1; y++) {
      createTerrain(chunk_pos);
    }
}
}