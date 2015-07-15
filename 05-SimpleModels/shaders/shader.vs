#version 440 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) uniform mat4 mvp;


out vec2 TexCoords;

void main()
{
   // gl_Position = projection * view * model * vec4(position, 1.0f);
	gl_Position = mvp* vec4(position, 1.0f);
    TexCoords = texCoords;
}