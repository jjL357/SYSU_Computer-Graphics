#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <IL/il.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective
#include <vector>

#include "camera.h"
#include "camera_free.h"
#include "shader_helper.h"
#include "_cube/cube.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "_plane/plane.h"
#include "framebuffer.h"
#include "drawable.h"
#include "_trees/tree.h"
#include "depth_framebuffer.hpp"

#define DEPTH_TEXTURE_SIZE 512

#include "_skybox/skybox.h"
#include "Snow.h"
#include"particles_manager.h"
const float cam_fov_angle = 3.1415f / 2.0f;
const float cam_fov_angle_shadowmap = 3.1415f / 1.8f;

void init();
void display(float time_delta);
void cleanup();
GLuint load_shader(char* path, GLenum shader_type);


Camera* cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer* framebuffer;
GLuint pid_quad_screen;



Cube cube_base;

Tree* tree;
Quad_screen quad_screen;
Transform cube_base_transf;

Transform new_cube_transf;
Transform tree_transf;
glm::mat4x4 projection_mat;
glm::mat4x4 projection_mat_depth;
glm::mat4x4 projection_mat_shadowmap_depth;

Depth_framebuffer depth_framebuffer_camera;
Depth_framebuffer depth_framebuffer_light;

unsigned int trunk_pid;
unsigned int leaves_pid;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
unsigned int light_mode_selected;
GLfloat camera_position[3];
GLfloat camera_direction[3];
GLuint light_mode = 0;

const int win_width = 1280;
const int win_height = 720;

unsigned int shadow_mapping_effect = 5;

// const int win_width = 400;
// const int win_height = 400;

float time_measured;

unsigned int effect_select;

//天空盒类
Skybox skybox;

//雪
Particles_manager snow_particles_manager;


void wind_func(float pos[3], float ret[3], float time) {
    ret[0] = (sin((pos[1] + pos[2] + time) / 2)) * 2;
    ret[1] = (cos((pos[0] + pos[2] + time) / 2)) * 2;
    ret[2] = (sin((pos[1] + pos[2] + time) / 2)) * 2;
}
int main() {
    if (!glfwInit()) {
        std::cout << "Error to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "tree", NULL, NULL);

    if (!window) {
        std::cout << "failed to open window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_NO_ERROR) {
        std::cout << "glew error\n";
        return -1;
    }
    // 四季变换所需变量
    int current_state = -1;
    //float red_inc = 0.02f;
    float red_inc = 0.05f;

    float redValue = 0.0f;
    init();
    //Snow::Snow snow;
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
        glfwPollEvents();

        static float prev_time = 0;

        float current_time = glfwGetTime();
        float time_delta = current_time - prev_time;
        // 四季变换
        glUseProgram(leaves_pid);
        int loc = glGetUniformLocation(leaves_pid, "red");
        redValue += red_inc * time_delta;
        glUniform1f(loc, redValue);

        if (redValue < 0.2f)
        {
            if (current_state != 3)
            {
                current_state = 3;
                cube_base.isWinter = 1;
                tree->refresh_leaves(3, time_delta);
            }
        }

        else if (redValue < 0.8f)
        {
            if (current_state != 1)
            {
                cube_base.isWinter = 0;
                //std::cout << "winter" << std::endl;
                current_state = 1;
                tree->isFall = 0;
                tree->refresh_leaves(0, time_delta);
            }
        }
        else if (redValue < 0.95f)
        {
            if (current_state != 2)
            {
                current_state = 2;
                tree->isFall = 1;
            }
        }
        else
        {
            if (current_state != 3)
            {
                cube_base.isWinter = 1;
                //std::cout << "winter" << std::endl;
                current_state = 3;
                tree->isFall = 1;
                tree->refresh_leaves(3, time_delta);
            }
        }
        if (redValue >= 1.0f)
        {
            redValue = 0.0f;
        }

        // camera move
        if (glfwGetKey(window, 'S') == GLFW_PRESS) {
            cam->input_handling('S', time_delta);
        }
        if (glfwGetKey(window, 'A') == GLFW_PRESS) {
            cam->input_handling('A', time_delta);
        }
        if (glfwGetKey(window, 'W') == GLFW_PRESS) {
            cam->input_handling('W', time_delta);
        }
        if (glfwGetKey(window, 'D') == GLFW_PRESS) {
            cam->input_handling('D', time_delta);
        }

        if (glfwGetKey(window, 'L') == GLFW_PRESS) {
            cam->input_handling('L', time_delta);
        }
        if (glfwGetKey(window, 'J') == GLFW_PRESS) {
            cam->input_handling('J', time_delta);
        }
        if (glfwGetKey(window, 'K') == GLFW_PRESS) {
            cam->input_handling('K', time_delta);
        }
        if (glfwGetKey(window, 'I') == GLFW_PRESS) {
            cam->input_handling('I', time_delta);
        }

        // reset tree
        if (glfwGetKey(window, 'R') == GLFW_PRESS) {
            delete tree;
            lst_drawable.pop_back();
            tree = new Tree;
            tree->init(trunk_pid, leaves_pid);
            tree->load();
            lst_drawable.push_back(tree);
        }

        // mapping effect
        if (glfwGetKey(window, '0') == GLFW_PRESS) {
            shadow_mapping_effect = 0;
        }
        if (glfwGetKey(window, '1') == GLFW_PRESS) {
            shadow_mapping_effect = 1;
        }
        if (glfwGetKey(window, '2') == GLFW_PRESS) {
            shadow_mapping_effect = 2;
        }
        if (glfwGetKey(window, '3') == GLFW_PRESS) {
            shadow_mapping_effect = 3;
        }
        if (glfwGetKey(window, '4') == GLFW_PRESS) {
            shadow_mapping_effect = 4;
        }
        if (glfwGetKey(window, '5') == GLFW_PRESS) {
            shadow_mapping_effect = 5;
        }
        if (glfwGetKey(window, '6') == GLFW_PRESS) {
            shadow_mapping_effect = 6;
        }

        // light mode select
        if (glfwGetKey(window, 'Z') == GLFW_PRESS) {
            light_mode_selected = 0;
        }
        if (glfwGetKey(window, 'X') == GLFW_PRESS) {
            light_mode_selected = 1;
        }
        if (glfwGetKey(window, 'C') == GLFW_PRESS) {
            light_mode_selected = 2;
        }
        snow_particles_manager.handle_particles(current_state);
        display(time_delta);
        glfwSwapBuffers(window);

        prev_time = current_time;
    }

    cleanup();

    return 0;
}

void init() {

    time_measured = 0.0f;
    effect_select = 0;

    glClearColor(0.3, 0.6, 1.0, 1.0); //sky
    // glClearColor(1.0, 1.0, 1.0, 1.0); //sky


    ilInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, win_width, win_height);
    projection_mat = glm::perspective(cam_fov_angle, (float)win_width / (float)win_height, 0.01f, 1000.0f);
    projection_mat_depth = glm::perspective(cam_fov_angle, (float)win_width / (float)win_height, 0.1f, 1000.0f);
    projection_mat_shadowmap_depth = glm::perspective(cam_fov_angle_shadowmap, (float)win_width / (float)win_height, 2.0f, 1000.0f);

    light_position[0] = 0.0; //x
    light_position[1] = 0.0; //up
    light_position[2] = 0.0; //z

    light_mode_selected = 1;
    //天空盒纹理图片位置
    std::vector<std::string> skyboxFaces = {
     "../resources/image/right.jpg",
     "../resources/image/left.jpg",
     "../resources/image/top.jpg",
     "../resources/image//bottom.jpg",
     "../resources/image/front.jpg",
     "../resources/image/back.jpg"
    };
    skybox.init(skyboxFaces);
    lst_drawable.push_back(&skybox);

    GLuint cube_pid = load_shaders("../resources/glsl/cube_vshader.glsl", "../resources/glsl/cube_fshader.glsl");

    cube_base.init(cube_pid);
    cube_base.set_color(0.8f, 0.8f, 0.8f);
    lst_drawable.push_back(&cube_base);

    //intialize the particle managers
    GLuint snow_particles_pid = load_shaders("../resources/glsl/snow_particles_vshader.glsl", "../resources/glsl/snow_particles_fshader.glsl");

    snow_particles_manager.init(20000, snow_particles_pid);
    snow_particles_manager.set_emiter_boundary(-20, 20, 29, 31, -20, 20);
    snow_particles_manager.set_life_duration_sec(2, 5);
    snow_particles_manager.set_initial_velocity(0, -30.0f / 5.0f, 0, 0, 1.0f, 0); //30/5 unit per second, with +- 1.0
    snow_particles_manager.set_wind_func(wind_func);
    snow_particles_manager.handle_particles(0);
    lst_drawable.push_back(&snow_particles_manager);

    int glError = glGetError();
    if (glError != GL_NO_ERROR) {
        printf("error ogl: %d\n", glError);
    }

    // tree
    tree_transf.translate(0.0f, 0.5f, 0.0f);
    tree_transf.scale(5.0f, 5.0f, 5.0f);

    trunk_pid = load_shaders("../resources/glsl/trunk_vshader.glsl", "../resources/glsl/trunk_fshader.glsl");
    leaves_pid = load_shaders("../resources/glsl/leaves_individual_vshader.glsl", "../resources/glsl/leaves_individual_fshader.glsl");



    tree = new Tree;
    tree->init(trunk_pid, leaves_pid);
    tree->load();
    lst_drawable.push_back(tree);


    depth_framebuffer_camera.init(win_width, win_height, cam_fov_angle);
    depth_framebuffer_light.init(DEPTH_TEXTURE_SIZE);

    depth_framebuffer_camera.set_perspective_mat(projection_mat_depth);
    depth_framebuffer_light.set_perspective_mat(projection_mat_shadowmap_depth);

    // base of tree (biggeer one)
    cube_base_transf.scale(20.0f, 0.5f, 20.0f);

    //framebuffers
    framebuffer = new Framebuffer();
    GLuint tex_fb = framebuffer->init(win_width, win_height, true);
    GLuint depth_tex_fb = depth_framebuffer_camera.get_texture_id();

    pid_quad_screen = load_shaders("../resources/glsl/quad_screen_vshader.glsl", "../resources/glsl/quad_screen_fshader.glsl");
    quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);
    quad_screen.set_depth_texture(depth_tex_fb);
    glUseProgram(pid_quad_screen);
    glUniform1f(glGetUniformLocation(pid_quad_screen, "fov_angle"), cam_fov_angle);


    cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    cam_fixed.set_window_size(win_width, win_height);

    cam_free.lookAt(10.0f, 10.0f, -7.0f, 0.0f, 7.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    cam_free.set_window_size(win_width, win_height);

    cam = &cam_free;
    cam_free.update_pos();


    //clip coord to tell shader not to draw anything over the water
    for (size_t i = 0; i < lst_drawable.size(); i++) {
        lst_drawable[i]->set_clip_coord(0, 1, 0, -20);
    }

}

void display(float time_delta) {
    //tree->leaves_fall();
    //tree->load();
    snow_particles_manager.set_time(glfwGetTime());
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    cam->get_position(camera_position);
    cam->get_direction(camera_direction);

    depth_framebuffer_camera.set_light_pos(camera_position);
    depth_framebuffer_camera.set_light_dir(camera_direction);
    depth_framebuffer_light.set_light_pos(light_position);
    //to be done before binding any fb (since it loads a fb itself)
    //draw the scene from the point of view of the light to the depth buffer
    //which will be given to the various shaders of opjects
    depth_framebuffer_light.draw_fb(&lst_drawable);
    depth_framebuffer_camera.draw_fb(&lst_drawable);


    framebuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);



    cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
    snow_particles_manager.set_view_matrix(cam->getMatrix());
    snow_particles_manager.set_projection_matrix(projection_mat);

    tree->set_MVP_matrices(tree_transf.get_matrix(), cam->getMatrix(), projection_mat);
    //skybox
    // 渲染天空盒
    glDepthFunc(GL_LEQUAL); // 使用这个深度函数用于天空盒
    skybox.set_view_matrix(glm::mat4(glm::mat3(cam->getMatrix())));// 从视图矩阵中移除平移部分
    skybox.set_projection_matrix(projection_mat);

    for (size_t i = 0; i < lst_drawable.size(); i++) {

        lst_drawable[i]->set_light_pos(light_position);
        lst_drawable[i]->set_camera_pos(camera_position);

        lst_drawable[i]->set_camera_direction(camera_direction);

        lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
        lst_drawable[i]->set_window_dim(win_width, win_height);

        lst_drawable[i]->set_shadow_buffer_texture(depth_framebuffer_light.get_texture_id());
        lst_drawable[i]->set_shadow_matrix(depth_framebuffer_light.get_shadow_mat());

        lst_drawable[i]->set_shadow_mapping_effect(shadow_mapping_effect);
    }

    tree->move_leaves(time_delta);


    glBindTexture(GL_TEXTURE_2D, depth_framebuffer_light.get_texture_id());
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < lst_drawable.size(); i++) {
        lst_drawable[i]->draw();
    }

    framebuffer->unbind();

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(pid_quad_screen);
    glUniformMatrix4fv(glGetUniformLocation(pid_quad_screen, "camera_transform_matrix"), 1, GL_FALSE, glm::value_ptr(cam->getMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(pid_quad_screen, "shadow_map_transform_mat"), 1, GL_FALSE, glm::value_ptr(depth_framebuffer_light.get_shadow_mat()));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depth_framebuffer_light.get_texture_id());
    GLuint tex_id = glGetUniformLocation(pid_quad_screen, "shadow_map_depth_tex");
    glUniform1i(tex_id, 3);
    glUniformMatrix4fv(glGetUniformLocation(pid_quad_screen, "proj_transform_matrix"), 1, GL_FALSE, glm::value_ptr(projection_mat_depth));
    quad_screen.draw(effect_select);

    if (light_mode_selected == 0) {
        light_position[0] = 10;
        light_position[1] = 15;
        light_position[2] = -10;
    }
    else if (light_mode_selected == 1) {
        light_position[0] = 20.0 * cos(glfwGetTime() / 2);
        light_position[1] = 25;
        light_position[2] = 20.0 * sin(glfwGetTime() / 2);
    }
    else if (light_mode_selected == 2) {
        light_position[0] = 30;
        light_position[1] = 30;
        light_position[2] = 0;
    }
}

void cleanup() {
}
