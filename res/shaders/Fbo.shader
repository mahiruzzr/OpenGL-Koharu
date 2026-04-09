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

in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D normalTexture;

const vec2 texOffset = vec2(0.5/800.0, 0.5/600.0);
void main()
{
    vec3 texColor = texture(screenTexture, TexCoords).rgb;

    //vec3 n =texture(normalTexture, TexCoords+vec2(0.0, texOffset.y)).rgb;
    //vec3 s =texture(normalTexture, TexCoords-vec2(0.0, texOffset.y)).rgb;
    //vec3 e =texture(normalTexture, TexCoords+vec2(texOffset.x, 0.0)).rgb;
    //vec3 w =texture(normalTexture, TexCoords-vec2(texOffset.x, 0.0)).rgb;

    //vec3 edge = abs(n - s) + abs(e - w);

    //float edgeStrength = length(edge);

    //edgeStrength = smoothstep(0.6, 0.85, edgeStrength);

    //vec3 lineColor = vec3(0.0, 0.0, 0.0);
    //vec3 result = mix(texColor, lineColor, edgeStrength);
    
    //float gamma = 2.2;
    //result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(texColor, 1.0);

}
