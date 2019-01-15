#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

class Application
{
public:
    Application(int argc, char** argv);
    ~Application();

    int run();
private:
    glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians);

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    GLuint m_textureSampler = 0; // Only one sampler object since we will use the sample sampling parameters for the two textures

    GLuint m_cube_vbo;
    GLuint m_cube_vao;
    std::size_t m_cube_vertices;
    GLuint m_cubeTextureKd = 0;
    GLuint m_sphere_vbo;
    GLuint m_sphere_vao;
    std::size_t m_sphere_vertices;
    GLuint m_sphereTextureKd = 0;

    glmlv::GLProgram m_program;
    glmlv::ViewController m_viewController;
};