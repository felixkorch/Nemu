R"(
#shader vertex
#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

vec3 f_position;
vec2 f_uv;

uniform mat4 ortho;

void main() {
	gl_Position = ortho * vec4(position, 1.0);
	f_position = position;
	f_uv = uv;
}

#shader fragment
#version 300 es

precision mediump float;


uniform sampler2D sampler;

vec3 f_position;
vec2 f_uv;

layout (location = 0) out vec4 pixel;

void main() {
    pixel = texture(sampler, f_uv);
}

)"