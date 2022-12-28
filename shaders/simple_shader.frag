#version 450

layout (location = 0) in vec3 frag_color;

layout (location = 0) out vec4 out_color;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  mat4 inverseView;
  vec3 cameraPositon;
  float dt;
} ubo;

layout(set = 0, binding = 2) uniform sampler2D image;

void main() {
  out_color = vec4(frag_color, 1.f);
}