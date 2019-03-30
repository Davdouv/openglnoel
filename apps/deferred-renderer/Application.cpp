#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glmlv/Image2DRGBA.hpp>

int Application::run()
{
	// Put here code to run before rendering loop

    // Location des uniformes
    GLint uModelViewProjMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uModelViewProjMatrix");
    GLint uModelViewMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uModelViewMatrix");
    GLint uNormalMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uNormalMatrix");

    GLint uKa_location = glGetUniformLocation(m_geometryProgram.glId(), "uKa");
    GLint uKd_location = glGetUniformLocation(m_geometryProgram.glId(), "uKd");
    GLint uKs_location = glGetUniformLocation(m_geometryProgram.glId(), "uKs");
    GLint uShininess_location = glGetUniformLocation(m_geometryProgram.glId(), "uShininess");

    GLint uKaSampler_Location = glGetUniformLocation(m_geometryProgram.glId(), "uKaSampler");
    GLint uKdSampler_Location = glGetUniformLocation(m_geometryProgram.glId(), "uKdSampler");
    GLint uKsSampler_Location = glGetUniformLocation(m_geometryProgram.glId(), "uKsSampler");
    GLint uShininessSampler_Location = glGetUniformLocation(m_geometryProgram.glId(), "uShininessSampler");

    GLint uDirectionalLightDir_location = glGetUniformLocation(m_shadingProgram.glId(), "uDirectionalLightDir");
    GLint uDirectionalLightIntensity_location = glGetUniformLocation(m_shadingProgram.glId(), "uDirectionalLightIntensity");
    GLint uPointLightPosition_location = glGetUniformLocation(m_shadingProgram.glId(), "uPointLightPosition");
    GLint uPointLightIntensity_location = glGetUniformLocation(m_shadingProgram.glId(), "uPointLightIntensity");

    m_uGBufferSamplerLocations[GPosition] = glGetUniformLocation(m_shadingProgram.glId(), "uGPosition");
    m_uGBufferSamplerLocations[GNormal] = glGetUniformLocation(m_shadingProgram.glId(), "uGNormal");
    m_uGBufferSamplerLocations[GAmbient] = glGetUniformLocation(m_shadingProgram.glId(), "uGAmbient");
    m_uGBufferSamplerLocations[GDiffuse] = glGetUniformLocation(m_shadingProgram.glId(), "uGDiffuse");
    m_uGBufferSamplerLocations[GGlossyShininess] = glGetUniformLocation(m_shadingProgram.glId(), "uGGlossyShininess");

    m_uDirLightViewProjMatrix = glGetUniformLocation(m_directionalSMProgram.glId(), "uDirLightViewProjMatrix");

    // Création des matrices
    //glm::mat4 projMatrix(glm::perspective(70.f, (float)m_nWindowWidth/m_nWindowHeight, 0.1f, 100.f));
    const auto projMatrix = glm::perspective(70.f, (float)m_nWindowWidth/m_nWindowHeight, 0.01f * m_SceneSize, m_SceneSize); // near = 1% de la taille de la scene, far = 100%
    glm::mat4 viewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
    glm::mat4 normalMatrix(glm::transpose(glm::inverse(viewMatrix)));

    glm::mat4 modelMatrixCube(glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0)));
    glm::mat4 modelMatrixSphere(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)));

    // Parametres de lumières
    float dirLightPhiAngleDegrees = 90.f;
    float dirLightThetaAngleDegrees = 45.f;
    glm::vec3 dirLightDirection = computeDirectionVector(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
    glm::vec3 dirLightColor = glm::vec3(1, 1, 1);
    float dirLightIntensity = 1.f;

    glm::vec3 pointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 pointLightColor = glm::vec3(1, 1, 1);
    float pointLightIntensity = 5.f;

    float clearColor[3] = { 0.8, 0.5, 0.2 };
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
    
    m_viewController.setViewMatrix(viewMatrix);
    m_viewController.setSpeed(m_SceneSize * 0.1f); // 10% de la scene parcouru par seconde

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        viewMatrix = m_viewController.getViewMatrix();

        // --- GEOMETRY PASS --- //
        {
            m_geometryProgram.use(); 

            // Same sampler for all texture units
            for (GLuint i : {0, 1, 2, 3})
                glBindSampler(i, m_textureSampler);               

            // Set texture unit of each sampler
            glUniform1i(uKaSampler_Location, 0);
            glUniform1i(uKdSampler_Location, 1);
            glUniform1i(uKsSampler_Location, 2);
            glUniform1i(uShininessSampler_Location, 3);

            // Lambda to bind textures of the materials
            const auto bindMaterial = [&](const PhongMaterial & material)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.KaTextureId);
                glUniform3fv(uKa_location, 1, glm::value_ptr(material.Ka));

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.KdTextureId);
                glUniform3fv(uKd_location, 1, glm::value_ptr(material.Kd));

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material.KsTextureId);
                glUniform3fv(uKs_location, 1, glm::value_ptr(material.Ks));

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material.shininessTextureId);
                glUniform1fv(uShininess_location, 1, &material.shininess);
            };

            // SCENE OBJ
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

            const auto fbSize = m_GLFWHandle.framebufferSize();
            glViewport(0, 0, fbSize.x, fbSize.y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(m_scene_vao);
            for (const auto shape: m_shapes)
            {
                glUniformMatrix4fv(uModelViewProjMatrix_location, 1, GL_FALSE, glm::value_ptr(projMatrix * viewMatrix * shape.localToWorldMatrix));
                glUniformMatrix4fv(uModelViewMatrix_location, 1, GL_FALSE, glm::value_ptr(viewMatrix * shape.localToWorldMatrix));
                glUniformMatrix4fv(uNormalMatrix_location, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(viewMatrix * shape.localToWorldMatrix))));

                const auto &material = (shape.materialID >= 0) ? m_SceneMaterials[shape.materialID] : m_defaultMaterial;
                bindMaterial(material);

                glDrawElements(GL_TRIANGLES, shape.indexCount, GL_UNSIGNED_INT, (const GLvoid*) (shape.indexOffset * sizeof(GLuint)));
            }
            // Debind sampler
            for (GLuint i : {0, 1, 2, 3})
                glBindSampler(0, m_textureSampler);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }

        // --- SHADING PASS --- //
        {
            // Datas has been send to geometry FS, now we draw a triangle with the shading FS
            const auto fbSize = m_GLFWHandle.framebufferSize();
		    glViewport(0, 0, fbSize.x, fbSize.y);
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw Triangle
            m_shadingProgram.use();      
            
            // Lights            
            glUniform3fv(uDirectionalLightDir_location, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(dirLightDirection), 0))));
            glUniform3fv(uDirectionalLightIntensity_location, 1, glm::value_ptr(dirLightColor * dirLightIntensity));
            glUniform3fv(uPointLightPosition_location, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLightPosition, 1))));
            glUniform3fv(uPointLightIntensity_location, 1, glm::value_ptr(pointLightColor * pointLightIntensity));
            
            for (int i = GPosition; i < GBufferTextureCount-1; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
                glUniform1i(m_uGBufferSamplerLocations[i], i);
            }
            
            glBindVertexArray(m_triangle_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        /*
        // Blit Textures
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + m_frameBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // //glBlitFramebuffer(GLint srcX0​, GLint srcY0​, GLint srcX1​, GLint srcY1​, GLint dstX0​, GLint dstY0​, GLint dstX1​, GLint dstY1​, GLbitfield mask​, GLenum filter​);
        glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);     
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        */

        // --- ImGUI --- //
		glmlv::imguiNewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            if (ImGui::ColorEdit3("clearColor", clearColor))
            {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }

            if (ImGui::CollapsingHeader("Directional Light"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(dirLightColor));
                ImGui::DragFloat("DirLightIntensity", &dirLightIntensity, 0.1f, 0.f, 100.f);
                if (ImGui::DragFloat("Phi Angle", &dirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                    ImGui::DragFloat("Theta Angle", &dirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                    dirLightDirection = computeDirectionVector(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
                }
            }

            if (ImGui::CollapsingHeader("Point Light"))
            {
                ImGui::ColorEdit3("PointLightColor", glm::value_ptr(pointLightColor));
                ImGui::DragFloat("PointLightIntensity", &pointLightIntensity, 0.1f, 0.f, 16000.f);
                ImGui::InputFloat3("Position", glm::value_ptr(pointLightPosition));
            }

            
            if (ImGui::CollapsingHeader("Frame Buffer"))
            {
                ImGui::RadioButton("Position", &m_frameBuffer, GPosition);
                ImGui::RadioButton("Normal", &m_frameBuffer, GNormal);
                ImGui::RadioButton("Ambient", &m_frameBuffer, GAmbient);
                ImGui::RadioButton("Diffuse", &m_frameBuffer, GDiffuse);
                ImGui::RadioButton("GlossyShininess", &m_frameBuffer, GGlossyShininess);
                ImGui::RadioButton("Depth", &m_frameBuffer, GDepth);
            }

            ImGui::End();
        }

		glmlv::imguiRenderFrame();

        glfwPollEvents(); // Poll for and process events

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            // Put here code to handle user interactions
            m_viewController.update(ellapsedTime);
        }

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" },
    m_viewController { m_GLFWHandle.window(), 3.f }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    // Put here initialization code

    // --- SHADERS --- //

    // Lambda to get shadersPath
    /* TO FIX
    const auto shadersPath = [&](const std::string vsName, const std::string fsName)
    {
        const auto pathToVS = m_ShadersRootPath / m_AppName / vsName;
        const auto pathToFS = m_ShadersRootPath / m_AppName / fsName;
        // Compilation et exécution des shaders
        const std::vector<glmlv::fs::path> shaderPaths {pathToVS, pathToFS};
        return shaderPaths;
    };

    // GEMOTRY PASS
    const auto shadersPath = shadersPath("geometryPass.vs.glsl", "geometryPass.fs.glsl");
    m_geometryProgram = glmlv::GLProgram(glmlv::compileProgram(shadersPath));
    */

    // SHADING PASS
    // Récupération des shaders
    const auto pathToShadingVS = m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl";
    const auto pathToShadingFS = m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl";
    // Compilation et exécution des shaders
    std::vector<glmlv::fs::path> shadingShaderPaths {pathToShadingVS, pathToShadingFS};
    m_shadingProgram = glmlv::GLProgram(glmlv::compileProgram(shadingShaderPaths));
    //m_shadingProgram.use();
    
    
    const GLuint VERTEX_ATTR_POSITION = 0;
    const GLuint VERTEX_ATTR_NORMAL = 1;
    const GLuint VERTEX_ATTR_TEX_COORDS = 2;

    // --- 3D SCENE --- //
    glmlv::SceneData sceneData;
    const auto objPath = m_AssetsRootPath / m_AppName / "models" / "sponza" / "sponza.obj";
    glmlv::loadObjScene(objPath, sceneData);

    m_SceneSize = glm::length(sceneData.bboxMax - sceneData.bboxMin);

    std::cout << "# of shapes    : " << sceneData.shapeCount << std::endl;
    std::cout << "# of materials : " << sceneData.materials.size() << std::endl;
    std::cout << "# of vertex    : " << sceneData.vertexBuffer.size() << std::endl;
    std::cout << "# of triangles    : " << sceneData.indexBuffer.size() / 3 << std::endl;
    //std::cout << "bbox : " << sceneData.bboxMin << ", " << sceneData.bboxMax << std::endl;

    // Fill VBO
    glGenBuffers(1, &m_scene_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_scene_vbo);
    glBufferStorage(GL_ARRAY_BUFFER, sceneData.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), sceneData.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Fill IBO
    glGenBuffers(1, &m_scene_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, m_scene_ibo);
    glBufferStorage(GL_ARRAY_BUFFER, sceneData.indexBuffer.size() * sizeof(uint32_t), sceneData.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_indexCountPerShape = sceneData.indexCountPerShape;

    // Init shapes
    uint32_t indexOffset = 0;
    for (auto shapeID = 0; shapeID < sceneData.indexCountPerShape.size(); ++shapeID)
    {
        m_shapes.emplace_back();
        auto & shape = m_shapes.back();
        shape.indexCount = sceneData.indexCountPerShape[shapeID];
        shape.indexOffset = indexOffset;
        shape.materialID = sceneData.materialIDPerShape[shapeID];
        shape.localToWorldMatrix = sceneData.localToWorldMatrixPerShape[shapeID];
        indexOffset += shape.indexCount;
    }

    // Fill VAO
    glGenVertexArrays(1, &m_scene_vao);
    glBindVertexArray(m_scene_vao);

     // We tell OpenGL what vertex attributes our VAO is describing:
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEX_COORDS);

    glBindBuffer(GL_ARRAY_BUFFER, m_scene_vbo); // We bind the VBO because the next 3 calls will read what VBO is bound in order to know where the data is stored

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, position));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, normal));
    glVertexAttribPointer(VERTEX_ATTR_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, texCoords));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_scene_ibo); // Binding the IBO to GL_ELEMENT_ARRAY_BUFFER while a VAO is bound "writes" it in the VAO for usage when the VAO will be drawn

    glBindVertexArray(0);

    // Textures
    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Default Texture
    {
        glmlv::Image2DRGBA image = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "bob.png");

        glGenTextures(1, &m_defaultTexture);
        glBindTexture(GL_TEXTURE_2D, m_defaultTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Upload all textures to the GPU
    std::vector<GLint> textureIds;
    for (const auto & texture : sceneData.textures)
    {
        GLuint texId = 0;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, texture.width(), texture.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        textureIds.emplace_back(texId);
    }
    // Attach texture IDs to materials (and store the materials)
    for (const auto & material : sceneData.materials)
    {
        PhongMaterial newMaterial;
        newMaterial.Ka = material.Ka;
        newMaterial.Kd = material.Kd;
        newMaterial.Ks = material.Ks;
        newMaterial.shininess = material.shininess;
        newMaterial.KaTextureId = material.KaTextureId >= 0 ? textureIds[material.KaTextureId] : m_defaultTexture;
        newMaterial.KdTextureId = material.KdTextureId >= 0 ? textureIds[material.KdTextureId] : m_defaultTexture;
        newMaterial.KsTextureId = material.KsTextureId >= 0 ? textureIds[material.KsTextureId] : m_defaultTexture;
        newMaterial.shininessTextureId = material.shininessTextureId >= 0 ? textureIds[material.shininessTextureId] : m_defaultTexture;

        m_SceneMaterials.emplace_back(newMaterial);
    }

    {
        m_defaultMaterial.Ka = glm::vec3(0);
        m_defaultMaterial.Kd = glm::vec3(1);
        m_defaultMaterial.Ks = glm::vec3(1);
        m_defaultMaterial.shininess = 32.f;
        m_defaultMaterial.KaTextureId = m_defaultTexture;
        m_defaultMaterial.KdTextureId = m_defaultTexture;
        m_defaultMaterial.KsTextureId = m_defaultTexture;
        m_defaultMaterial.shininessTextureId = m_defaultTexture;     
    }
    
    // Generate GBuffer textures
    {
        // Textures
        glGenTextures(GBufferTextureCount, m_GBufferTextures);
        for (int i = 0; i < GBufferTextureCount; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // FBO
        glGenFramebuffers(GBufferTextureCount, &m_FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        for (int i = 0; i < GBufferTextureCount-1; ++i)
        {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
        }
            // Depth
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GBufferTextureCount-1], 0);
        
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
        glDrawBuffers(5, drawBuffers);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            exit(-1);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    // --- TRIANGLE --- //
        // VBO
    glGenBuffers(1, &m_triangle_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangle_VBO);
    GLfloat vertices[] = { -1, -1, 3, -1, -1, 3};
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);
        // VAO
    glGenVertexArrays(1, &m_triangle_VAO);
    glBindVertexArray(m_triangle_VAO);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Activate Depth Test
    glEnable(GL_DEPTH_TEST);

    // --- SHADOW MAPPING --- //
        // Texture
    glGenTextures(1, &m_directionalSMTexture);
    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
    glBindTexture(GL_TEXTURE_2D, 0);
        // FBO
    glGenFramebuffers(1, &m_directionalSMFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);
    GLenum drawBuffer = GL_DEPTH_ATTACHMENT;
    glDrawBuffers(1, &drawBuffer);
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        exit(-1);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // Sampler
    glGenSamplers(1, &m_directionalSMSampler);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    
    const auto pathToDirectionalSMVS = m_ShadersRootPath / m_AppName / "directionalSM.vs.glsl";
    const auto pathToDirectionalSMFS = m_ShadersRootPath / m_AppName / "directionalSM.fs.glsl";
    // Compilation et exécution des shaders
    std::vector<glmlv::fs::path> directionalSMShaderPaths {pathToDirectionalSMVS, pathToDirectionalSMFS};
    m_directionalSMProgram = glmlv::GLProgram(glmlv::compileProgram(directionalSMShaderPaths));
}

Application::~Application()
{
    glDeleteBuffers(1, &m_scene_vbo);
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteBuffers(1, &m_triangle_VBO);
    glDeleteVertexArrays(1, &m_triangle_VAO);
    glDeleteVertexArrays(1, &m_scene_vao);
}

// Thx cheat code
glm::vec3 Application::computeDirectionVector(float phiRadians, float thetaRadians)
{
    const auto cosPhi = glm::cos(phiRadians);
    const auto sinPhi = glm::sin(phiRadians);
    const auto sinTheta = glm::sin(thetaRadians);
    return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
}