#ifndef TREE_H
#define TREE_H

#include "drawable.h"
#include "trunk.h"
#include "individual_leaves.h"
#include "noise_generator.hpp"
#include "texture_float.h"
#include "stb_image.h"


class Tree : public Drawable{

public:

   virtual ~Tree(){
      glDeleteBuffers(1, &_vbo_leaves_transf);
      glDeleteBuffers(1, &_vbo_trunk_transf);
      glDeleteBuffers(1, &_vbo_leaves_base_pos);
      glDeleteVertexArrays(1, &_vao_ileaves);
      glDeleteVertexArrays(1, &_vao_trunk);
   }

   void init(GLuint _pid_trunk, GLuint _pid_ileaves){
      srand(time(0));

      enabled = true;

      this->_pid_trunk = _pid_trunk;
      this->_pid_ileaves = _pid_ileaves;

      wind_offset[0] = wind_offset[1] = 0.0f;
   }

   void init(){
      _pid_trunk = load_shaders("trunk_vshader.glsl", "trunk_fshader.glsl");
      _pid_ileaves = load_shaders("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");

      init(_pid_trunk, _pid_ileaves);
   }

   virtual void load(){

      glGenVertexArrays(1, &_vao_ileaves);
      Individual_leaves ileaves;
      ileaves.init(_pid_ileaves, _vao_ileaves);

      //Trunk trunk;
      glGenVertexArrays(1, &_vao_trunk);
      trunk.init(_pid_trunk, _vao_trunk);

      //get translation matrices for the end points of the tree branches
      std::vector<glm::mat4> trunk_end_point_matrices = trunk.get_end_point_matrices();

      ileaves.generate(trunk_end_point_matrices, 0, 0);

      std::vector<glm::mat4x4> ileaves_mat = ileaves.get_mat_vector();
      std::vector<glm::vec3> ileaves_pos_mat = ileaves.get_pos_vector();

      // insert the model vector of leaves into the main vector
      mat_vector_ileaves.insert(mat_vector_ileaves.end(), ileaves_mat.begin(), ileaves_mat.end());
      pos_vector_ileaves.insert(pos_vector_ileaves.end(), ileaves_pos_mat.begin(), ileaves_pos_mat.end());

      std::vector<glm::mat4> trunks = trunk.get_transf();
      for (size_t i = 0; i < trunks.size(); i++) {
         mat_vector_trunks.push_back(trunks[i]);
      }

      trunk_nb_indices_to_draw = trunk.indices_to_draw();

      ////////////////trunk
      {
         glBindVertexArray(_vao_trunk);
         glGenBuffers(1, &_vbo_trunk_transf);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_trunk_transf);

         glm::mat4 *matrices_array = new glm::mat4x4[mat_vector_trunks.size()];

         printf("will draw %lu trunks\n", mat_vector_trunks.size());

         for (size_t i = 0; i < mat_vector_trunks.size(); i++) {
            matrices_array[i] = mat_vector_trunks[i];
         }

         glBufferData(GL_ARRAY_BUFFER, mat_vector_trunks.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

         unsigned vec4_size = sizeof(glm::vec4);

         GLuint model_attrib_0 = glGetAttribLocation(_pid_trunk, "model_mat");

         glEnableVertexAttribArray(model_attrib_0);
         glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

         glEnableVertexAttribArray(model_attrib_0+1);
         glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

         glEnableVertexAttribArray(model_attrib_0+2);
         glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

         glEnableVertexAttribArray(model_attrib_0+3);
         glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

         glVertexAttribDivisor(model_attrib_0, 1);
         glVertexAttribDivisor(model_attrib_0+1, 1);
         glVertexAttribDivisor(model_attrib_0+2, 1);
         glVertexAttribDivisor(model_attrib_0+3, 1);

         glBindVertexArray(0);

         nb_trunks_to_draw = mat_vector_trunks.size();
         mat_vector_trunks.clear();
         delete[] matrices_array;


         //树干纹理
         unsigned char* texture_data;
         int width, height, nrChannels;
         texture_data = stbi_load("../resources/image/trunk.jpg", &width, &height, &nrChannels, 0);
         std::cout << width << " " << height << " " << std::endl;
         glGenTextures(1, &_texture_trunk);
         glBindTexture(GL_TEXTURE_2D, _texture_trunk);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         if (texture_data)
         {
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
             glGenerateMipmap(GL_TEXTURE_2D);
         }
         else
         {
             std::cout << "fail to load image" << std::endl;
         }
         stbi_image_free(texture_data);
         glBindTexture(GL_TEXTURE_2D, 0);
         
      }

      // LEAVES
      {
         glBindVertexArray(_vao_ileaves);

         glGenBuffers(1, &_vbo_leaves_transf);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_leaves_transf);

         glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector_ileaves.size()];

         printf("will draw %lu individual leaves\n", mat_vector_ileaves.size());

         for (size_t i = 0; i < mat_vector_ileaves.size(); i++) {
            matrices_array[i] = mat_vector_ileaves[i];
         }

         glBufferData(GL_ARRAY_BUFFER, mat_vector_ileaves.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

         unsigned vec4_size = sizeof(glm::vec4);

         GLuint model_attrib_0 = glGetAttribLocation(_pid_ileaves, "model_mat");
         glEnableVertexAttribArray(model_attrib_0);
         glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

         glEnableVertexAttribArray(model_attrib_0+1);
         glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

         glEnableVertexAttribArray(model_attrib_0+2);
         glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

         glEnableVertexAttribArray(model_attrib_0+3);
         glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

         glVertexAttribDivisor(model_attrib_0, 1);
         glVertexAttribDivisor(model_attrib_0+1, 1);
         glVertexAttribDivisor(model_attrib_0+2, 1);
         glVertexAttribDivisor(model_attrib_0+3, 1);

         //draw the raw position of the leaves
         glGenBuffers(1, &_vbo_leaves_base_pos);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_leaves_base_pos);

         glm::vec3 *pos_array = new glm::vec3[pos_vector_ileaves.size()];

         for (size_t i = 0; i < pos_vector_ileaves.size(); i++) {
            pos_array[i] = pos_vector_ileaves[i];
         }

         glBufferData(GL_ARRAY_BUFFER, pos_vector_ileaves.size()*sizeof(glm::vec3), pos_array, GL_STATIC_DRAW);

         unsigned vec3_size = sizeof(glm::vec3);

         //gives the position of the leave
         GLuint pos_attrib = glGetAttribLocation(_pid_ileaves, "raw_position");
         glEnableVertexAttribArray(pos_attrib);
         glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 1 * vec3_size, (void*)0);

         glVertexAttribDivisor(pos_attrib, 1);

         glBindVertexArray(0);

         nb_ileaves_to_draw = mat_vector_ileaves.size();
         mat_vector_ileaves.clear();
         delete[] pos_array;
         delete[] matrices_array;

         // 树叶纹理
         unsigned char* texture_data;;
         int width, height, nrChannels;
         texture_data = stbi_load("../resources/image/leaves.png", &width, &height, &nrChannels, 0);
         std::cout << width << " " << height << " " << std::endl;
         glGenTextures(1, &_texture_leaves);
         glBindTexture(GL_TEXTURE_2D, _texture_leaves);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         if (texture_data)
         {
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
             glGenerateMipmap(GL_TEXTURE_2D);
         }
         else
         {
             std::cout << "fail to load image" << std::endl;
         }
         stbi_image_free(texture_data);
         glBindTexture(GL_TEXTURE_2D, 0);
         //
      }


      //generate a simple noise to simulate weak wind on the leaves
      noise_gen_wind.set_noise_function(NOISE_SELECT_EASE);
      noise_gen_wind.set_noise_level(3);

      std::vector<std::vector<float> > noise_val = noise_gen_wind.get_2D_noise(512, 512, -1.0f, 1.0f, -1.0f, 1.0f);  // 生成一个512x512的二维噪声，范围在-1.0到1.0之间
      tex_wind_noise.set_data(noise_val);  // / 将噪声数据设置到纹理对象tex_wind_noise中

      //传入数据
      glGenTextures(1, &_tex_wind_noise);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_wind_noise.get_width(), tex_wind_noise.get_height(), 0, GL_RED, GL_FLOAT, tex_wind_noise.get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //can be clamp to edge, clamp to border or gl repeat
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;
   }

   void draw_trunks(){  // 画树干

      glUseProgram(_pid_trunk);
      glBindVertexArray(_vao_trunk);
      // 绑定纹理
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _texture_trunk);  // 绑定树干纹理
      glUniform1i(glGetUniformLocation(_pid_trunk, "trunk_texture"), 0); // 手动设置

      glUniform3fv( glGetUniformLocation(_pid_trunk, "light_position"), 1, this->light_position);
      glUniformMatrix4fv( glGetUniformLocation(_pid_trunk, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid_trunk, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid_trunk, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDisable(GL_CULL_FACE);
      if(enabled){
         glDrawElementsInstanced(GL_TRIANGLES, trunk_nb_indices_to_draw, GL_UNSIGNED_INT, 0, nb_trunks_to_draw);
      }
      glEnable(GL_CULL_FACE);

      glBindVertexArray(0);
   }

   void move_leaves(float time_delta){
      // 树叶扰动
      wind_offset[0] += time_delta/4;
      if(wind_offset[0] > 10.0f){
         wind_offset[0] = 0;
      }

      wind_offset[1] += time_delta/8;
      if(wind_offset[1] > 10.0f){
         wind_offset[1] = 0;
      }
   }

   void draw_ileaves(){  // 画树叶

      glUseProgram(_pid_ileaves);
      glBindVertexArray(_vao_ileaves);
      // 绑定树叶纹理
      glActiveTexture(GL_TEXTURE1);  // 激活纹理单元
      glBindTexture(GL_TEXTURE_2D, _texture_leaves);  // 绑定树叶纹理
      glUniform1i(glGetUniformLocation(_pid_ileaves, "leaves_texture"), 1); // 手动设置纹理单元

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      GLuint tex_id = glGetUniformLocation(_pid_ileaves, "tex_wind");
      glUniform1i(tex_id, 0/*GL_TEXTURE0*/);

      //float fallTime = 10 + int(glfwGetTime()) / 50 * 50;
      //float fallDistance = (glfwGetTime() - fallTime) * 0.60;
      //std::cout << fallDistance << std::endl;
      if (isFall)
      {
          this->model_matrix = glm::translate(this->model_matrix, glm::vec3(0.0f, -fallDistance, 0.0f));
          fallDistance += 0.0025;
      }
      else
      {
          fallDistance = 0.0;
      }

      /*
      std::cout << "Model Matrix:\n";
      for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < 4; ++j) {
              std::cout << this->model_matrix[i][j] << " ";
          }
          std::cout << std::endl;
      }*/

      glUniformMatrix4fv( glGetUniformLocation(_pid_ileaves, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix) );
      glUniformMatrix4fv( glGetUniformLocation(_pid_ileaves, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid_ileaves, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));
      glUniform2fv( glGetUniformLocation(_pid_ileaves, "wind_offset"), 1, this->wind_offset);
      if (isFall) this->model_matrix = glm::translate(this->model_matrix, glm::vec3(0.0f, fallDistance, 0.0f));

      if(enabled){
         glDisable(GL_CULL_FACE);
         //only three indices for the leaves (modelled as triangles)
         glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0, nb_ileaves_to_draw);
         glEnable(GL_CULL_FACE);
      }

      glBindVertexArray(0);
   }

   void draw(){
       
      draw_trunks();
      draw_ileaves();
   }
   void leaves_fall() {
       //float falltime = glfwGetTime();
       int size = pos_vector_ileaves.size();
       int num = size / 5;
       srand(time(NULL));
       int id;
       for (int i = 0; i < num; i++) {
           id = rand() % size;
           pos_vector_ileaves[id][1] -= 0.2f;
       }

       glBindBuffer(GL_ARRAY_BUFFER, _vbo_leaves_base_pos);
       glm::vec3* pos_array = new glm::vec3[pos_vector_ileaves.size()];
       //std::cout << pos_vector_ileaves[0][0]<<" " <<glfwGetTime()<< std::endl;
       for (size_t i = 0; i < pos_vector_ileaves.size(); i++) {
           pos_array[i] = pos_vector_ileaves[i];
       }

       glBufferData(GL_ARRAY_BUFFER, pos_vector_ileaves.size() * sizeof(glm::vec3), pos_array, GL_DYNAMIC_DRAW);
       glBindBuffer(GL_ARRAY_BUFFER, 0);

   }

   // 刷新切换四季
   void refresh_leaves(int state, float deltaTime)
   {
       glDeleteBuffers(1, &_vbo_leaves_base_pos);
       glDeleteBuffers(1, &_vbo_leaves_transf);
       glDeleteVertexArrays(1, &_vao_ileaves);

       glGenVertexArrays(1, &_vao_ileaves);
       Individual_leaves ileaves;
       ileaves.init(_pid_ileaves, _vao_ileaves);

       std::vector<glm::mat4> trunk_end_point_matrices = trunk.get_end_point_matrices();
       ileaves.generate(trunk_end_point_matrices, state, deltaTime);

       std::vector<glm::mat4x4> ileaves_mat = ileaves.get_mat_vector();
       std::vector<glm::vec3> ileaves_pos_mat = ileaves.get_pos_vector();

       // insert the model vector of leaves into the main vector
       //mat_vector_ileaves.clear();  // 清空
       mat_vector_ileaves.insert(mat_vector_ileaves.end(), ileaves_mat.begin(), ileaves_mat.end());
       //pos_vector_ileaves.clear();  // 清空
       pos_vector_ileaves.insert(pos_vector_ileaves.end(), ileaves_pos_mat.begin(), ileaves_pos_mat.end());

       //LEAVES
       {
           glBindVertexArray(_vao_ileaves);

           glGenBuffers(1, &_vbo_leaves_transf);
           glBindBuffer(GL_ARRAY_BUFFER, _vbo_leaves_transf);

           glm::mat4x4* matrices_array = new glm::mat4x4[mat_vector_ileaves.size()];

           printf("will draw %llu individual leaves\n", mat_vector_ileaves.size());

           for (size_t i = 0; i < mat_vector_ileaves.size(); i++) {
               matrices_array[i] = mat_vector_ileaves[i];
           }

           glBufferData(GL_ARRAY_BUFFER, mat_vector_ileaves.size() * sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

           uint32_t vec4_size = sizeof(glm::vec4);

           GLuint model_attrib_0 = glGetAttribLocation(_pid_ileaves, "model_mat");
           glEnableVertexAttribArray(model_attrib_0);
           glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

           glEnableVertexAttribArray(model_attrib_0 + 1);
           glVertexAttribPointer(model_attrib_0 + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

           glEnableVertexAttribArray(model_attrib_0 + 2);
           glVertexAttribPointer(model_attrib_0 + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2 * vec4_size));

           glEnableVertexAttribArray(model_attrib_0 + 3);
           glVertexAttribPointer(model_attrib_0 + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

           glVertexAttribDivisor(model_attrib_0, 1);
           glVertexAttribDivisor(model_attrib_0 + 1, 1);
           glVertexAttribDivisor(model_attrib_0 + 2, 1);
           glVertexAttribDivisor(model_attrib_0 + 3, 1);

           //draw the raw position of the leaves
           glGenBuffers(1, &_vbo_leaves_base_pos);
           glBindBuffer(GL_ARRAY_BUFFER, _vbo_leaves_base_pos);

           glm::vec3* pos_array = new glm::vec3[pos_vector_ileaves.size()];

           for (size_t i = 0; i < pos_vector_ileaves.size(); i++) {
               pos_array[i] = pos_vector_ileaves[i];
           }

           glBufferData(GL_ARRAY_BUFFER, pos_vector_ileaves.size() * sizeof(glm::vec3), pos_array, GL_STATIC_DRAW);

           uint32_t vec3_size = sizeof(glm::vec3);

           //gives the position of the leave
           GLuint pos_attrib = glGetAttribLocation(_pid_ileaves, "raw_position");
           glEnableVertexAttribArray(pos_attrib);
           glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 1 * vec3_size, (void*)0);

           glVertexAttribDivisor(pos_attrib, 1);

           glBindVertexArray(0);

           delete[] pos_array;
           delete[] matrices_array;
           nb_ileaves_to_draw = mat_vector_ileaves.size();  // 距离绘制顶点点数
           mat_vector_ileaves.clear();  // 清空
           pos_vector_ileaves.clear();  // 清空
       }
   }
protected:

   Trunk trunk;  // 树干

   GLuint _texture_trunk;  // 树干纹理
   GLuint _texture_leaves;  // 树叶纹理

   GLuint _tex_wind_noise;
   Noise_generator noise_gen_wind;
   Texture_float tex_wind_noise;

   GLuint _pid_trunk;
   GLuint _vao_trunk;

   GLuint _vbo_leaves_transf;
   GLuint _vbo_trunk_transf;
   GLuint _vbo_leaves_base_pos;

   GLuint _pid_ileaves;
   GLuint _vao_ileaves;

   bool enabled;
   unsigned int nb_ileaves_to_draw;
   unsigned int nb_trunks_to_draw;

   unsigned int trunk_nb_indices_to_draw;

   float time_delta;
   float wind_offset[2];

   std::vector<glm::mat4x4> mat_vector_leaves;
   std::vector<glm::mat4x4> mat_vector_trunks;
   std::vector<glm::mat4x4> mat_vector_ileaves;
   std::vector<glm::vec3> pos_vector_ileaves;

   std::vector<Transform> lst_transf;
   
public:
    int isFall = 0;
    double fallDistance = 0.0;
    double totalDistance = 0.0;
};

#endif
