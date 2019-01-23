#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

// Sorties du shader
out vec3 vViewSpacePosition; // Position du sommet transformé dans l'espace View
out vec3 vViewSpaceNormal; // Normale du sommet transformé dans l'espace View
out vec2 vTexCoords; // Coordonnées de texture du sommet

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

void main() {
   // Passage en coordonnées homogènes
    vec4 vertexPosition = vec4(aPosition, 1);
    vec4 vertexNormal = vec4(aNormal, 0);

    // Calcul des valeurs de sortie
    vViewSpacePosition = vec3(uModelViewMatrix * vertexPosition);
    vViewSpaceNormal = vec3(uNormalMatrix * vertexNormal);
    vTexCoords = aTexCoords;

    // Calcul de la position projetée
    gl_Position = uModelViewProjMatrix * vertexPosition;
};
