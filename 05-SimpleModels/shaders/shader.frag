#version 440 
in vec2 TexCoords;
out vec4 color;

//uniform sampler2D texture_diffuse1;
layout (location = 5) uniform sampler2D texture_diffuse1;

void main()
{    
    color = texture(texture_diffuse1, TexCoords);
}

