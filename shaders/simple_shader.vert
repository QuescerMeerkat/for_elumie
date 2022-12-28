#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 frag_color;

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

float rand(vec2 seed){
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
  vec4 positionWorld = push.modelMatrix /*to world space*/ * vec4(position, 1.f) /*NDC space*/;
  //               finally                        <--  then                      <--   first
  gl_Position = ubo.projection /*to screen space*/ * ubo.view /*to camera space*/ * positionWorld /*world space*/;
  frag_color = vec3(color);
}

                              /*          NDC Space
                              (-1,-1)-------------------(1,1)               -y up z+ fowards x+ to the right
                              |                             |
                              |                             |
                              |                             |
                              |            (0,0)            |
                              |                             |
                              |                             |
                              |                             |
                              (-1,1)--------------------(1,1)
                              */
