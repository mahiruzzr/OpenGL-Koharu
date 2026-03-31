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
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
uniform float ambientStrength;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightDir;

void main()
{

   vec4 texColor = texture(texture_diffuse1, TexCoords);
   
   // 2. 丟棄透明像素：如果是透明背景，直接跳過不畫！(解決黑色嘴巴塊)
   if(texColor.a < 0.01) {
       discard;
   }

   vec3 lightColor = vec3(1.0, 1.0, 1.0); // 白色光源
   
   // 3. 環境光 (Ambient)
   vec3 ambient = ambientStrength * lightColor;
   
   // 4. 漫反射 (Diffuse)
   vec3 norm = normalize(Normal);
   vec3 lightDirNorm = normalize(-lightDir); // 注意這裡是 -lightDir，因為我們想要從光源指向片段的方向
   float diff = max(dot(norm, lightDirNorm), 0.0);
   vec3 diffuse = diff * lightColor;
   
   // 5. 注意這裡！一定要用 texColor.rgb (只取前三個顏色通道) 來跟 vec3 相乘
   vec3 result = (ambient + diffuse) * texColor.rgb;
   
   // 6. 鏡面光 (Specular)
   float specularStrength = 0.5f;
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 reflectDir = reflect(-lightDirNorm, norm);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
   vec3 specular = specularStrength * spec * lightColor;
   
   result += specular;
   
   // 7. 輸出最終顏色，並帶上它原本的透明度
   color = vec4(result, texColor.a);
}

