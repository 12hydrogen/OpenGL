#version 330 core
layout (location = 0) in vec3 inputPos;
layout (location = 1) in vec3 inputNormal;
layout (location = 2) in vec2 inputTexture;

out vec3 aNormal;
out vec3 fragPos;
out vec2 aTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 normalMat;

void main()
{
	gl_Position = projection * view * model * vec4(inputPos, 1.0);
	fragPos = vec3(model * vec4(inputPos, 1.0));
	aNormal = mat3(normalMat) * inputNormal;
	aTexture = inputTexture;
}
