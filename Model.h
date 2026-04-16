#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include "tool/stb_image.h"
#include "Mesh.h"
#include<limits>

// 記得在最前面 #include <stb_image.h>
// 如果 stb_image.h 是你第一次實作，記得要在某個 .cpp 檔裡面加上 #define STB_IMAGE_IMPLEMENTATION

inline unsigned int TextureFromFile(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID); // 請 OpenGL 產生一個貼圖 ID

    int width, height, nrComponents;
    // 垂直翻轉貼圖 (通常 OpenGL 的 Y 軸是從下到上，但圖片是從上到下)
    stbi_set_flip_vertically_on_load(false); 
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // 自動生成 Mipmap

        // 設定貼圖環繞與過濾方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        // 圖片資料已經傳到 GPU，釋放記憶體
    } else {
        std::cout << "貼圖載入失敗，路徑: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

class Model {
public:
    // 這個模型包含的所有網格
    std::vector<Mesh> meshes;
    bool isKoharu = false;
    std::string target = "Object_3";

    // 建構子：傳入檔案路徑，立刻開始載入
    Model(std::string path) {
        loadModel(path);
        if(path.find("wakamo") != std::string::npos){
            isKoharu = false;
        }else{
            isKoharu = true;
        }
    }

    glm::vec3 localmin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 localmax = glm::vec3(std::numeric_limits<float>::lowest());

    // 繪製模型：迴圈呼叫每一個 Mesh 的 Draw
    void Draw(unsigned int shaderProgram){
        for(unsigned int i = 0; i < meshes.size(); i++) {
            //std::cout << "Drawing Mesh: " << meshes[i].name << std::endl;
            bool ishalo = false;
            bool isHalo = false;
            bool isSingleSided = false;
            if(!isKoharu && meshes[i].name == target){
                continue;
            }
            
            // 檢查這個 Mesh 的貼圖路徑，是否包含 "halo"
            for(unsigned int j = 0; j < meshes[i].textures.size(); j++) {
                if(meshes[i].textures[j].path.find("halo") != std::string::npos) {
                    ishalo = true;
                    break;
                }
                if(meshes[i].textures[j].path.find("Halo") != std::string::npos) {
                    isHalo = true;
                    break;
                }
                if(meshes[i].textures[j].path.find("Trans") != std::string::npos) {
                    isSingleSided = true;
                    break;
                }
            }
            if(!ishalo && !isHalo && !isSingleSided){
                glEnable(GL_CULL_FACE);
            }else{
                glDisable(GL_CULL_FACE);
            }
            meshes[i].Draw(shaderProgram);

            if(ishalo){
                glEnable(GL_CULL_FACE);
            }
            }
        }
    
    // 💡 新增：專門給 Outline Shader 用的 Draw
    void DrawOutlinePass(unsigned int shaderProgram) {
        for(unsigned int i = 0; i < meshes.size(); i++) {
            if(!isKoharu && meshes[i].name == target){
                continue;
            }
            meshes[i].Draw(shaderProgram);
        }
    }

private:
    std::string directory; // 儲存模型所在的資料夾路徑，方便稍後拼接貼圖路徑

    void loadModel(std::string path) {
        Assimp::Importer importer;
        // 載入檔案，並要求 Assimp 幫忙把多邊形轉成三角形、並翻轉 UV 的 Y 軸
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp 讀取錯誤: " << importer.GetErrorString() << std::endl;
            return;
        }
        
        // 擷取資料夾路徑，這樣我們之後在載入貼圖時就知道從哪裡找了
        directory = path.substr(0, path.find_last_of('/'));
        // 從根節點開始遞迴處理
        processNode(scene->mRootNode, scene);
    }

    // 遞迴遍歷所有的節點
    void processNode(aiNode *node, const aiScene *scene) {
        // 處理這個節點包含的所有網格
        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // 繼續處理子節點
        for(unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    // 將 Assimp 的 aiMesh 轉換成我們自己寫的 Mesh 類別
    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        std::string meshName = mesh->mName.C_Str();
        if (meshName.empty()) {
        // 如果 Assimp 沒有解析到名稱，用材質索引當備用名稱
        meshName = "mesh_" + std::to_string(mesh->mMaterialIndex);
        }

        // 1. 在這裡寫迴圈，抓取 vertices (座標、法線、UV)
        // ...
        for(unsigned int i=0;i<mesh->mNumVertices;i++){
            Vertex v;
            glm::vec3 pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            if (pos.x > localmax.x) localmax.x = pos.x;
            if (pos.x < localmin.x) localmin.x = pos.x;
            if (pos.y > localmax.y) localmax.y = pos.y;
            if (pos.y < localmin.y) localmin.y = pos.y;
            if (pos.z > localmax.z) localmax.z = pos.z;
            if (pos.z < localmin.z) localmin.z = pos.z;
            v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            if(mesh->mTextureCoords[0]){
                v.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }else{
                v.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(v);
        }

        // 2. 在這裡寫迴圈，抓取 indices
        // ...
        
        for(unsigned int i=0;i<mesh->mNumFaces;i++){
            for(unsigned int j=0;j<3;j++){
                indices.push_back(mesh->mFaces[i].mIndices[j]);
            }

        }
        
        // 3. 處理材質與貼圖 (動態讀取真正屬於這個網格的貼圖)
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            aiString str;
            aiTextureType typeToUse = aiTextureType_NONE;

            if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
                typeToUse = aiTextureType_BASE_COLOR;
            } else if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                typeToUse = aiTextureType_DIFFUSE;
            } else if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0) { 
            // 💡 新增：檢查發光貼圖 (光環通常在這裡)
                typeToUse = aiTextureType_EMISSIVE;
            } else if (material->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
                typeToUse = aiTextureType_LIGHTMAP;
            }

            // 如果有找到任何一種貼圖，就載入它
            if (typeToUse != aiTextureType_NONE) {
                material->GetTexture(typeToUse, 0, &str);
                Texture t;
                std::string fullPath = directory + "/" + str.C_Str();
                
                t.id = TextureFromFile(fullPath.c_str());
                t.name = str.C_Str();
                t.type = "texture_diffuse";
                // 這裡維持 texture_diffuse，因為你的 Shader 變數是這個名字
                t.path = fullPath;
                textures.push_back(t);
            } else {
                // 如果真的都找不到，印出警告讓我們知道是哪個網格出包
                std::cout << "⚠️ 警告：網格找不到貼圖！Material Index: " << mesh->mMaterialIndex << std::endl;
            }
        }

        return Mesh(meshName, vertices, indices, textures);
    }
};