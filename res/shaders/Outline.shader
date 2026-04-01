#shader vertex
#version 330 core 

layout(location = 0) in vec3 vertices;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float Worldy;


void main()
{
    TexCoords = texCoords;
    float outlineWidth = 0.1; // 調整輪廓寬度

    vec3 fatterPos = vertices + (normal * outlineWidth);
    gl_Position = projection* view* model*vec4(fatterPos, 1.0);
}
#shader fragment
#version 330 core 

layout(location = 0) out vec4 Outlinecolor;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;;
void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    if(texColor.a < 0.5) {
        discard;
    }
   Outlinecolor = vec4(0.0f, 0.0f, 0.0f,1.0);
}

