#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 WorldPos;
uniform mat4 view;
uniform mat4 projection;

void main(){
    WorldPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec3 WorldPos;
uniform vec3 cameraPos;

void main(){

    vec2 coord = WorldPos.xz;

    vec2 derivative = fwidth(coord);

    vec2 gridDist = abs(fract(coord - 0.5) - 0.5) / derivative;

    float lineDist = min(gridDist.x, gridDist.y);

    float lineAlpha= 1.0-smoothstep(0.0, 3.0, lineDist);

    float distToCamera = length(WorldPos - cameraPos);

    float fadeAlpha = 1.0 - smoothstep(5.0, 30.0, distToCamera);

    vec3 gridColor = vec3(0.8f, 0.6f, 0.98f);

    color = vec4(gridColor, lineAlpha * fadeAlpha);
}