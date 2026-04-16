#shader vertex
#version 330 core 

layout(location = 0) in vec3 vertices;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
   gl_Position = projection* view* model*vec4(vertices, 1.0);
   TexCoords = texCoords;
   FragPos = vec3(model * vec4(vertices, 1.0));
   //解決不等比縮放,對法線向量產生的影響
   Normal = mat3(transpose(inverse(model))) * normal;

}
#shader fragment
#version 330 core 

layout(location = 0) out vec4 color;
layout(location=1) out vec4 NormalColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform float ambientStrength;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightDir;
uniform bool u_useRim;
uniform float u_rimPower;
float gamma = 2.2;

void main()
{

   vec4 texColor = texture(texture_diffuse1, TexCoords);
   
   if(texColor.a < 1.0) {
       discard;
   }

   vec3 lightColor = vec3(1.0, 1.0, 1.0); // 白色光源
   
   // 4. 漫反射 (Diffuse)
   vec3 norm = normalize(Normal);
   vec3 lightDirNorm = normalize(-lightDir); // 注意這裡是 -lightDir，因為我們想要從光源指向片段的方向
   float diff = max(dot(norm, lightDirNorm), 0.0);

   float toonIntensity;

   toonIntensity = smoothstep(0.35, 0.55, diff);
   
   vec3 viewDir = normalize(viewPos - FragPos);
   
   vec3 baseColor = texColor.rgb;

   vec3 shadowTint = baseColor * vec3(0.7, 0.7, 0.8); 
   vec3 darkColor = texColor.rgb * shadowTint * 0.75;

   vec3 result = mix(darkColor, baseColor, toonIntensity);

   if(u_useRim){

      float rim = 1.0 -max(dot(viewDir,norm),0.0);

      float rimIntensity = pow(rim, u_rimPower);

      vec3 rimColor = vec3(1.0f, 0.7f, 0.97f);

      result += rimIntensity * rimColor;
   }

   float gamma = 2.2;
   result = pow(result, vec3(1.0/gamma));

   // 7. 輸出最終顏色，並帶上它原本的透明度
   color = vec4(result, texColor.a);

   NormalColor = vec4(norm * 0.5 + 0.5, 1.0); // 將法線從 [-1, 1] 映射到 [0, 1]
}
