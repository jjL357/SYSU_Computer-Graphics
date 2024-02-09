#ifndef INDIVIDUAL_LEAVES_H
#define INDIVIDUAL_LEAVES_H

#include "drawable.h"
#include "transform.h"

class Individual_leaves : public Drawable{
public:

   virtual ~Individual_leaves(){
      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_vbo_sur_norm);
      glDeleteBuffers(1, &_vbo_idx);
      glDeleteBuffers(1, &_vbo_tex);
   }

   void init(){
      GLuint pid = load_shaders("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");
      srand(time(0));

      init(pid);
   }

   void init(GLuint pid){
      glGenVertexArrays(1, &_vao);
      init(pid, _vao);
   }

   void init(GLuint pid, GLuint _vao){

      this->_pid = pid;
      if(_pid == 0) exit(-1);

      glBindVertexArray(_vao);

      GLfloat vpoint[] = {
         -1.0f, -1.0f, 0.0f, // 0 bottom left
         1.0f, -1.0f, 0.0f, // 1 bottom right
         0.0f, 1.0f, 0.0f, // 2 top
      };

      GLuint vpoint_index[] = {
         0, 1, 2,
      };

      GLfloat per_surface_normals[] = {0, 1, 0,
         0, 1, 0,
         0, 1, 0,
      };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

      //纹理坐标
      const GLfloat vtexcoord[] = { 0.0f, 1.0f,
                                    1.0f, 1.0f,
                                    0.5f, 0.0f};

      glGenBuffers(1, &_vbo_tex);  // 生成纹理坐标对象
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);  // 绑定纹理坐标对象
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);  // 传入纹理坐标
      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");  // 获取着色器属性的位置
      glEnableVertexAttribArray(vtexcoord_id);  // 启动该位置的顶点属性数值
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);  //指定顶点属性数据的格式和位置

      glBindVertexArray(0);

   }

   //generate the leaves in a single sphere around top of tree
   void generate(){
      mat_vector.clear();
      pos_vector.clear();
      float incr = 0.12f;
      for (float x = -1.0; x < 1.0f; x+=incr) {
         for (float y = -1.0; y < 1.0f; y+=incr) {
            for (float z = -1.0; z < 1.0f; z+=incr) {
               float dist_to_centre = sqrt(x*x+y*y+z*z);
               if (dist_to_centre < 1.0f){

                  int val_rand = rand()%1000;
                  float frand = float(val_rand)/1000.0f;
                  Transform t;

                  Transform random_rot;
                  random_rot.rotate(1.0, 0.0, 0.0, frand);
                  random_rot.rotate(0.0, 1.0, 0.0, frand);
                  random_rot.rotate(0.0, 0.0, 1.0, frand);

                  t.mult(random_rot);

                  t.translate(x, y, z);

                  t.mult(random_rot); //rotates again the leaves so they don't have the same orientation
                  //size of individual leaves
                  float leaves_scale = (0.4+(1.0f-dist_to_centre))/4.0f;
                  t.scale(leaves_scale, leaves_scale, leaves_scale);

                  //push this matrix in a vector, will be fed to the shader buffer
                  mat_vector.push_back(t.get_matrix());
                  glm::vec4 val(x, y, z, 1);
                  glm::vec4 val_transf = random_rot.get_matrix()*val;
                  glm::vec3 val_transf3(val_transf.x, val_transf.y, val_transf.z);
                  pos_vector.push_back(val_transf3);
               }
            }
         }
      }
   }

   //generate multiple little sphere around end points
   // 生成树叶
   void generate(std::vector<glm::mat4> end_point_matrices, int state, float deltaTime){
      mat_vector.clear();
      pos_vector.clear();
      currenttime = 0.1 * deltaTime;
      const float leaves_scale = 1.0/32.0f;  // 叶子缩放

      float seasons[] = { 0.4, 0.8, 1.0, 2.0 };  // 控制四季的叶子数量
      for (size_t i = 0; i < end_point_matrices.size(); i++) {  // 遍历枝干
         float incr = seasons[state];  // 当前步长
         for (float x = -1.0; x < 1.0f; x+=incr) {
            for (float y = -1.0; y < 1.0f; y+=incr) {
               for (float z = -1.0; z < 1.0f; z+=incr) {
                  float dist_to_centre = sqrt(x*x+y*y+z*z);  // 计算到中心的距离
                  if (dist_to_centre < 1.0f){  // 防止离枝干太远
                     // 对树叶随机旋转
                     float frand = float(rand()%1000)/1000.0f;
                     Transform t;
                     Transform random_rot;
                     random_rot.rotate(1.0, 0.0, 0.0, frand);
                     frand = float(rand()%1000)/1000.0f;
                     random_rot.rotate(0.0, 1.0, 0.0, frand);
                     frand = float(rand()%1000)/1000.0f;
                     random_rot.rotate(0.0, 0.0, 1.0, frand);
                     t.mult(random_rot);

                     // 对树叶进行平移
                     t.translate(x*leaves_scale*3, y*leaves_scale*3, z*leaves_scale*3);
                     // 在时间的推移下使叶子下落
                     float fallSpeed = 0.1;  // 调整叶子下落的速度
                     float yOffset = fallSpeed * currenttime;
                     t.translate(0.0, -yOffset, 0.0);
                     t.mult(random_rot); //rotates again the leaves so they don't have the same orientation

                     t.scale(leaves_scale, leaves_scale, leaves_scale);

                     //push this matrix in a vector, will be fed to the shader buffer
                     mat_vector.push_back(end_point_matrices[i]*t.get_matrix());
                     glm::vec4 val(x, y, z, 1);
                     glm::vec4 val_transf = random_rot.get_matrix()*val;
                     glm::vec3 val_transf3(val_transf.x, val_transf.y, val_transf.z);
                     pos_vector.push_back(val_transf3);
                  }
               }
            }
         }

      }
   }

   void build_matrices(){
      GLuint _vbo_transf;
      glGenBuffers(1, &_vbo_transf);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

      glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector.size()];

      printf("will draw %lu leaves\n", mat_vector.size());

      for (size_t i = 0; i < mat_vector.size(); i++) {
         matrices_array[i] = mat_vector[i];
      }

      glBufferData(GL_ARRAY_BUFFER, mat_vector.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

      uint32_t vec4_size = sizeof(glm::vec4);

      //separate model matrix into four rows
      GLuint model_attrib_0 = glGetAttribLocation(_pid, "model_mat");
      glEnableVertexAttribArray(model_attrib_0);
      glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

      glEnableVertexAttribArray(model_attrib_0+1);
      glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

      glEnableVertexAttribArray(model_attrib_0+2);
      glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2 * vec4_size));

      glEnableVertexAttribArray(model_attrib_0+3);
      glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

      glVertexAttribDivisor(model_attrib_0, 1);
      glVertexAttribDivisor(model_attrib_0+1, 1);
      glVertexAttribDivisor(model_attrib_0+2, 1);
      glVertexAttribDivisor(model_attrib_0+3, 1);


      glBindVertexArray(0);

      nb_leaves_to_draw = mat_vector.size();
      mat_vector.clear();
      delete[] matrices_array;
   }

   unsigned int get_nb_leaves_to_draw(){
      return nb_leaves_to_draw;
   }

   std::vector<glm::mat4x4> get_mat_vector(){
      return mat_vector;
   }

   std::vector<glm::vec3> get_pos_vector(){
      return pos_vector;
   }

   void draw(){

      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));
      //float fallDistance = currenttime;
      float fallDistance = glfwGetTime();
     
      //std::cout << fallDistance << std::endl;
      this->model_matrix = glm::translate(this->model_matrix, glm::vec3(0.0f, -fallDistance, 0.0f));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix) );

      if(enabled){
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }

      glBindVertexArray(0);
   }
protected:

   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_sur_norm;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _pid;

   unsigned int nb_leaves_to_draw;

   std::vector<glm::mat4x4> mat_vector;
   std::vector<glm::vec3> pos_vector;
   float currenttime=0.0f;
};

#endif
