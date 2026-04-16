#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 NormalColor;

in vec3 FragPos;

uniform vec3 u_bgColor;
uniform float u_radius;
uniform float u_feather;
uniform float u_shadowRadius;

void main()
{
    float dist = length(FragPos.xz);

    float mask = smoothstep(u_radius - u_feather, u_radius, dist);
    float alpha = 1.0 - mask;

    if(alpha < 0.01) discard;

    float shadowIntensity = smoothstep(0.0, u_shadowRadius, dist);
    vec3 shadowColor = u_bgColor * 0.8;
    vec3 finalColor = mix(shadowColor, u_bgColor, shadowIntensity);

    color = vec4(finalColor, alpha);
    NormalColor = vec4(0.5, 1.0, 0.5, 1.0);
}