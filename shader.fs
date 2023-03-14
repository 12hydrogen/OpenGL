#version 330 core
in vec2 tCoord;

out vec4 FragColor;

uniform sampler2D box;
uniform sampler2D smile;

uniform float mixRate;

void main()
{
	FragColor = mix(texture2D(box, tCoord), texture2D(smile, tCoord), mixRate);
}
