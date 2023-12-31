#version 420 core

layout (location = 0) in vec3 aPosition;

//uniform mat4 projMat;
//uniform mat4 viewMat;
uniform mat4 modelMat;

uniform mat4 PVmat;

void main() {
    //gl_Position = projMat * viewMat * vec4(vec3(modelMat * vec4(aPosition, 1.0)), 1.0);
    gl_Position = PVmat * vec4(vec3(modelMat * vec4(aPosition, 1.0)), 1.0);
}
