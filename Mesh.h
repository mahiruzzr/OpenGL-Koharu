#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include<iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};
class Mesh {
public:
    // 網格資料
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;  // EBO 用的索引值
    std::vector<Texture>      textures;

    // 建構子
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 建立物件時，立刻設定好 VAO, VBO, EBO
        setupMesh();
    }

    // 繪製這個網格
    void Draw(unsigned int shaderProgram) {
        // 1. 綁定這個網格擁有的所有貼圖
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for(unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // 啟動對應的 Texture Unit
            
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);

            // 把 Texture Unit 告訴 Shader
            glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 2. 畫出網格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 3. 重置狀態
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // 渲染數據
    unsigned int VAO, VBO, EBO;

    // 初始化 Buffer (把你現在 main.cpp 裡的 glGenBuffers 搬來這裡)
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        // 載入頂點資料
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        // 載入索引資料
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 設定頂點屬性指標 (Position)
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 設定頂點屬性指標 (Normal)
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // 設定頂點屬性指標 (TexCoords)
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};