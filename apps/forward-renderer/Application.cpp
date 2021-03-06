#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glmlv/Image2DRGBA.hpp>

int Application::run()
{
	// Put here code to run before rendering loop

    // Location des uniformes
    GLint uModelViewProjMatrix_location = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    GLint uModelViewMatrix_location = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    GLint uNormalMatrix_location = glGetUniformLocation(m_program.glId(), "uNormalMatrix");

    GLint uDirectionalLightDir_location = glGetUniformLocation(m_program.glId(), "uDirectionalLightDir");
    GLint uDirectionalLightIntensity_location = glGetUniformLocation(m_program.glId(), "uDirectionalLightIntensity");
    GLint uPointLightPosition_location = glGetUniformLocation(m_program.glId(), "uPointLightPosition");
    GLint uPointLightIntensity_location = glGetUniformLocation(m_program.glId(), "uPointLightIntensity");

    GLint uKa_location = glGetUniformLocation(m_program.glId(), "uKa");
    GLint uKd_location = glGetUniformLocation(m_program.glId(), "uKd");
    GLint uKs_location = glGetUniformLocation(m_program.glId(), "uKs");
    GLint uShininess_location = glGetUniformLocation(m_program.glId(), "uShininess");

    GLint uKaSampler_Location = glGetUniformLocation(m_program.glId(), "uKaSampler");
    GLint uKdSampler_Location = glGetUniformLocation(m_program.glId(), "uKdSampler");
    GLint uKsSampler_Location = glGetUniformLocation(m_program.glId(), "uKsSampler");
    GLint uShininessSampler_Location = glGetUniformLocation(m_program.glId(), "uShininessSampler");

    // Création des matrices
    //glm::mat4 projMatrix(glm::perspective(70.f, (float)m_nWindowWidth/m_nWindowHeight, 0.1f, 100.f));
    const auto projMatrix = glm::perspective(70.f, (float)m_nWindowWidth/m_nWindowHeight, 0.01f * m_SceneSize, m_SceneSize); // near = 1% de la taille de la scene, far = 100%
    glm::mat4 viewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
    glm::mat4 normalMatrix(glm::transpose(glm::inverse(viewMatrix)));

    glm::vec3 cubePosition = { -2, 0, 0 };
    glm::vec3 spherePosition = { 2, 0, 0 };

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

    glm::vec3 cubeKd = { 1.0, 1.0, 1.0 };
    glm::vec3 sphereKd = { 1.0, 1.0, 1.0 };
    
    m_viewController.setViewMatrix(viewMatrix);
    m_viewController.setSpeed(m_SceneSize * 0.1f); // 10% de la scene parcouru par seconde

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // Put here rendering code
		const auto fbSize = m_GLFWHandle.framebufferSize();
		glViewport(0, 0, fbSize.x, fbSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMatrix = m_viewController.getViewMatrix();

        // Lumieres
        glUniform3fv(uDirectionalLightDir_location, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(dirLightDirection), 0))));
        glUniform3fv(uDirectionalLightIntensity_location, 1, glm::value_ptr(dirLightColor * dirLightIntensity));
        glUniform3fv(uPointLightPosition_location, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLightPosition, 1))));
        glUniform3fv(uPointLightIntensity_location, 1, glm::value_ptr(pointLightColor * pointLightIntensity));

        // Same sampler for all texture units
        for (GLuint i : {0, 1, 2, 3})
            glBindSampler(i, m_textureSampler);

        // Set texture unit of each sampler
        glUniform1i(uKaSampler_Location, 0);
        glUniform1i(uKdSampler_Location, 1);
        glUniform1i(uKsSampler_Location, 2);
        glUniform1i(uShininessSampler_Location, 3);

        /*
        glActiveTexture(GL_TEXTURE0);

        // Render objects
        {   // CUBE
            modelMatrixCube = glm::translate(glm::mat4(1), cubePosition);
                // Transforms
            glUniformMatrix4fv(uModelViewProjMatrix_location, 1, GL_FALSE, glm::value_ptr(projMatrix * viewMatrix * modelMatrixCube));
            glUniformMatrix4fv(uModelViewMatrix_location, 1, GL_FALSE, glm::value_ptr(viewMatrix * modelMatrixCube));
            glUniformMatrix4fv(uNormalMatrix_location, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(viewMatrix * modelMatrixCube))));

            glUniform3fv(uKd_location, 1, glm::value_ptr(cubeKd));

            glBindTexture(GL_TEXTURE_2D, m_cubeTextureKd);
            glBindVertexArray(m_cube_vao);
            glDrawElements(GL_TRIANGLES, m_cube_vertices, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }

        {   // SPHERE
            modelMatrixSphere = glm::translate(glm::mat4(1), spherePosition);
                // Transforms
            glUniformMatrix4fv(uModelViewProjMatrix_location, 1, GL_FALSE, glm::value_ptr(projMatrix * viewMatrix * modelMatrixSphere));
            glUniformMatrix4fv(uModelViewMatrix_location, 1, GL_FALSE, glm::value_ptr(viewMatrix * modelMatrixSphere));
            glUniformMatrix4fv(uNormalMatrix_location, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(viewMatrix * modelMatrixSphere))));

            glUniform3fv(uKd_location, 1, glm::value_ptr(sphereKd));

            glBindTexture(GL_TEXTURE_2D, m_sphereTextureKd);
            glBindVertexArray(m_sphere_vao);
            glDrawElements(GL_TRIANGLES, m_sphere_vertices, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }        

        glBindTexture(GL_TEXTURE_2D, 0);
        */

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

        {   // SCENE OBJ
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
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindSampler(0, 0); // Unbind the sampler

        // GUI code:
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

            if (ImGui::CollapsingHeader("Materials"))
            {
                ImGui::ColorEdit3("Cube Kd", glm::value_ptr(cubeKd));
                ImGui::ColorEdit3("Sphere Kd", glm::value_ptr(sphereKd));
            }

            if (ImGui::CollapsingHeader("Positions"))
            {
                ImGui::DragFloat3("Cube", glm::value_ptr(cubePosition));
                ImGui::DragFloat3("Sphere", glm::value_ptr(spherePosition));
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
    // Récupération des shaders
    const auto pathToVS = m_ShadersRootPath / m_AppName / "forward.vs.glsl";
    const auto pathToFS = m_ShadersRootPath / m_AppName / "forward.fs.glsl";

    // Compilation et exécution des shaders
    std::vector<glmlv::fs::path> shaderPaths {pathToVS, pathToFS};
    m_program = glmlv::GLProgram(glmlv::compileProgram(shaderPaths));
    m_program.use();
    
    const GLuint VERTEX_ATTR_POSITION = 0;
    const GLuint VERTEX_ATTR_NORMAL = 1;
    const GLuint VERTEX_ATTR_TEX_COORDS = 2;

    // --- CUBE --- //
    glmlv::SimpleGeometry cube = glmlv::makeCube();
        // VBO
    glGenBuffers(1, &m_cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
    glBufferStorage(GL_ARRAY_BUFFER, cube.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), cube.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
        // IBO
    GLuint cube_ibo;
    glGenBuffers(1, &cube_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(uint32_t), cube.indexBuffer.data(), 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // VAO
    glGenVertexArrays(1, &m_cube_vao);
    glBindVertexArray(m_cube_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEX_COORDS);
    glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glVertexAttribPointer(VERTEX_ATTR_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_cube_vertices = cube.indexBuffer.size();

    // --- SPHERE --- //
    glmlv::SimpleGeometry sphere = glmlv::makeSphere(32);
        // VBO
    glGenBuffers(1, &m_sphere_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    glBufferStorage(GL_ARRAY_BUFFER, sphere.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), sphere.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
        // IBO
    GLuint sphere_ibo;
    glGenBuffers(1, &sphere_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ibo);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sphere.indexBuffer.size() * sizeof(uint32_t), sphere.indexBuffer.data(), 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // VAO
    glGenVertexArrays(1, &m_sphere_vao);
    glBindVertexArray(m_sphere_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ibo);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEX_COORDS);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
    glVertexAttribPointer(VERTEX_ATTR_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, 
        sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_sphere_vertices = sphere.indexBuffer.size();

    // --- TEXTURES --- //
    glActiveTexture(GL_TEXTURE0);
    {
        glmlv::Image2DRGBA image = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "bob.png");

        glGenTextures(1, &m_cubeTextureKd);
        glBindTexture(GL_TEXTURE_2D, m_cubeTextureKd);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    {
        glmlv::Image2DRGBA image = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "pokeball.png");

        glGenTextures(1, &m_sphereTextureKd);
        glBindTexture(GL_TEXTURE_2D, m_sphereTextureKd);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
        newMaterial.KaTextureId = material.KaTextureId >= 0 ? textureIds[material.KaTextureId] : m_cubeTextureKd;
        newMaterial.KdTextureId = material.KdTextureId >= 0 ? textureIds[material.KdTextureId] : m_cubeTextureKd;
        newMaterial.KsTextureId = material.KsTextureId >= 0 ? textureIds[material.KsTextureId] : m_cubeTextureKd;
        newMaterial.shininessTextureId = material.shininessTextureId >= 0 ? textureIds[material.shininessTextureId] : m_cubeTextureKd;

        m_SceneMaterials.emplace_back(newMaterial);
    }

    {
        m_defaultMaterial.Ka = glm::vec3(0);
        m_defaultMaterial.Kd = glm::vec3(1);
        m_defaultMaterial.Ks = glm::vec3(1);
        m_defaultMaterial.shininess = 32.f;
        m_defaultMaterial.KaTextureId = m_cubeTextureKd;
        m_defaultMaterial.KdTextureId = m_cubeTextureKd;
        m_defaultMaterial.KsTextureId = m_cubeTextureKd;
        m_defaultMaterial.shininessTextureId = m_cubeTextureKd;     
    }
    

    std::cout << "# textures " << textureIds.size() << std::endl;

    // Activate Depth Test
    glEnable(GL_DEPTH_TEST);
}

Application::~Application()
{
    // Free VBO & VAO
    glDeleteBuffers(1, &m_cube_vbo);
    glDeleteVertexArrays(1, &m_cube_vao);
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);
}

// Thx cheat code
glm::vec3 Application::computeDirectionVector(float phiRadians, float thetaRadians)
{
    const auto cosPhi = glm::cos(phiRadians);
    const auto sinPhi = glm::sin(phiRadians);
    const auto sinTheta = glm::sin(thetaRadians);
    return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
}