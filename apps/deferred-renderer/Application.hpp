#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/scene_loading.hpp>

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
    GLuint m_defaultTexture = 0;

    glmlv::GLProgram m_geometryProgram;
    glmlv::GLProgram m_shadingProgram;

    glmlv::ViewController m_viewController;

    GLuint m_scene_vbo;
    GLuint m_scene_vao;
    GLuint m_scene_ibo;
    GLuint m_FBO;

    GLuint m_triangle_VBO;  // Used for GBuffer pixels
    GLuint m_triangle_VAO;  // Used for GBuffer pixels

    using PhongMaterial = glmlv::SceneData::PhongMaterial;

    struct Shape
    {
        uint32_t indexCount; // Number of indices
        uint32_t indexOffset; // Offset in GPU index buffer
        int materialID = -1;
        glm::mat4 localToWorldMatrix;
    };

    std::vector<uint32_t> m_indexCountPerShape;    
    std::vector<PhongMaterial> m_SceneMaterials;
    std::vector<Shape> m_shapes;

    PhongMaterial m_defaultMaterial;
    
    float m_SceneSize = 0.f; // Used for camera speed and projection matrix parameters

    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
        GBufferTextureCount
    };
    GLuint m_GBufferTextures[GBufferTextureCount];
    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
    int m_frameBuffer = 0;
    GLuint m_uGBufferSamplerLocations[GBufferTextureCount];

    // Shadow Mapping
    GLuint m_directionalSMTexture;
    GLuint m_directionalSMFBO;
    GLuint m_directionalSMSampler;
    int32_t m_nDirectionalSMResolution = 512;

    glmlv::GLProgram m_directionalSMProgram;
    GLint m_uDirLightViewProjMatrix;
};