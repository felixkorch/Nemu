R"(
#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out DATA
{
	vec3 position;
	vec2 uv;
} vs_out;

uniform mat4 ortho;

void main() {
	gl_Position = ortho * vec4(position, 1.0);
	vs_out.position = position;
	vs_out.uv = uv;
}

#shader fragment
#version 330 core

uniform sampler2D sampler;

in DATA
{
	vec3 position;
	vec2 uv;
} fs_in;

layout (location = 0) out vec4 pixel;

void main() {
	pixel = texture(sampler, fs_in.uv);
}

)"