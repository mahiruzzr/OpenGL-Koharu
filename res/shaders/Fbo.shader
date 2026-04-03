#shader vertex
#version 330 core
layout (location = 0) in vec3 vertices;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    
    gl_Position = vec4(vertices.x, vertices.y, 0.0, 1.0); 
    TexCoords = texCoords;
}

#shader fragment
#version 330 core
layout(location=0) out vec4 FragColor;
layout(location=1)out vec4 BrightColor;

in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D blurTexture;

void main()
{
    vec3 texColor = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(texColor, 1.0);

}
