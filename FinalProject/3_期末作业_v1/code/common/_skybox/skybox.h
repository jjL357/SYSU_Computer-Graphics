// skybox.h
#ifndef SKYBOX_H
#define SKYBOX_H

#include "../shader_helper.h"
#include "../drawable.h"
#include <vector>

class Skybox : public Drawable {
public:
    Skybox() {
        // �����Ҫ����ʼ����Ա����
        skyboxVAO = 0;
        skyboxVBO = 0;
        cubemapTexture = 0;
    }

    ~Skybox() {
        // ������Դ
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
                std::cout << "��պ��������ʧ��: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // ���ز�������ɫ������
        set_shader(load_shaders("../resources/glsl/skybox.vs.glsl", "../resources/glsl/skybox.fs.glsl"));

        
        //��պ�������
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

        // ����һ�����������������ƣ�������洢�� skyboxVAO ������
        glGenVertexArrays(1, &skyboxVAO);
        // ����һ�����㻺���������ƣ�������洢�� skyboxVBO ������
        glGenBuffers(1, &skyboxVBO);
        // �󶨶���������󣬺����Ķ����������úͶ��㻺�����Ĳ���������������� VAO ��
        glBindVertexArray(skyboxVAO);
        // �����㻺�����󶨵� GL_ARRAY_BUFFER Ŀ���ϣ�GL_ARRAY_BUFFER ��һ���������������ͣ����ڴ洢��������
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        // ���������ݸ��Ƶ����㻺�������
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        // ���ö����������顣����Ĳ��� 0 ��ʾʹ�ö�����������Ϊ 0 ������
        glEnableVertexAttribArray(0);
        // ���ö�������ָ�롣����Ĳ���ָ���˶������Ե�����
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    void draw() override {

        glUseProgram(_pid);

        glUniformMatrix4fv(glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
        glUniformMatrix4fv(glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

        glDepthFunc(GL_LEQUAL); // ��֤��պ�����������֮�����
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // �ָ���Ȳ��Ժ���
    }

private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
};

#endif
