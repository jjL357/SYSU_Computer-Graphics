// skybox.h
#ifndef SKYBOX_H
#define SKYBOX_H

#include "../shader_helper.h"
#include "../drawable.h"
#include <vector>

class Skybox : public Drawable {
public:
    Skybox() {
        // 如果需要，初始化成员变量
        skyboxVAO = 0;
        skyboxVBO = 0;
        cubemapTexture = 0;
    }

    ~Skybox() {
        // 清理资源
        glDeleteTextures(1, &cubemapTexture);
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);
    }

    void init(std::vector<std::string> faces) {
        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else {
                std::cout << "天空盒纹理加载失败: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // 加载并设置着色器程序
        set_shader(load_shaders("../resources/glsl/skybox.vs.glsl", "../resources/glsl/skybox.fs.glsl"));

        
        //天空盒子坐标
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        // 生成一个顶点数组对象的名称，并将其存储在 skyboxVAO 变量中
        glGenVertexArrays(1, &skyboxVAO);
        // 生成一个顶点缓冲对象的名称，并将其存储在 skyboxVBO 变量中
        glGenBuffers(1, &skyboxVBO);
        // 绑定顶点数组对象，后续的顶点属性设置和顶点缓冲对象的操作都将作用在这个 VAO 上
        glBindVertexArray(skyboxVAO);
        // 将顶点缓冲对象绑定到 GL_ARRAY_BUFFER 目标上，GL_ARRAY_BUFFER 是一个缓冲区对象类型，用于存储顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        // 将顶点数据复制到顶点缓冲对象中
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        // 启用顶点属性数组。这里的参数 0 表示使用顶点属性索引为 0 的属性
        glEnableVertexAttribArray(0);
        // 设置顶点属性指针。这里的参数指定了顶点属性的配置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    void draw() override {

        glUseProgram(_pid);

        glUniformMatrix4fv(glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
        glUniformMatrix4fv(glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

        glDepthFunc(GL_LEQUAL); // 保证天空盒在其他物体之后绘制
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // 恢复深度测试函数
    }

private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
};

#endif
