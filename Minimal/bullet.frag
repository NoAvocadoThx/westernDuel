#version 330 core

in vec3 Normal;
out vec4 color;

uniform int colorValue;

void main(void) {
  color = vec4(0.3,0.2,0.1, 0.2);
}
