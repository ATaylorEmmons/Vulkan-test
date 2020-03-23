#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 attr_Position;
layout(location = 0) out vec3 out_Color;

layout(location = 1) in vec3 attr_Color;

layout(push_constant) uniform _PushConstants {
    mat4 data;
} PushConstant;

layout(binding = 0) uniform _View {
    vec4 data; //x, y => translation, z => zoom, w => unitRatio
    vec4 resolution;
} View;

void main() {
    mat3 model = mat3(PushConstant.data);
    vec3 tint = transpose(PushConstant.data)[3].xyz;
    vec2 res = vec2(View.resolution);
    
    vec3 worldSpace = vec3(attr_Position, 1.0) * model * View.data.z;
    vec3 clipSpace = vec3((worldSpace*View.data.w).xy/(.5f*res.xy), 1.0);
    
    gl_Position = vec4(vec3(clipSpace.x, -clipSpace.y, clipSpace.z), 1.0);
    out_Color = attr_Color * tint;
    
}