#version 400

in  vec2 position;
out vec2 texCoord;

void main(void)
{
    texCoord = (position + vec2(1.0, 1.0)) * 0.5;
    gl_Position = vec4(position, 0, 1);
}
