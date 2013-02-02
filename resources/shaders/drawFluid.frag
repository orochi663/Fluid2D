#version 400

uniform sampler2D DyeTex;
uniform sampler2D VelocityTex;
uniform sampler2D PressureTex;
uniform sampler2D HeatTex;

in  vec2 texCoord;
out vec4 FragColor;

void main(void)
{
    vec4 color = vec4(0.0);

    color += step(texCoord.x, 0.5) * step(texCoord.y, 0.5) *
             texture(DyeTex,      texCoord*2.0);

    color += step(texCoord.x, 0.5) * step(0.5, texCoord.y) *
             texture(VelocityTex, texCoord*2.0 - vec2(0, 1));

    color += step(0.5, texCoord.x) * step(texCoord.y, 0.5) *
             texture(PressureTex, texCoord*2.0 - vec2(1, 0));

    color += step(0.5, texCoord.x) * step(0.5, texCoord.y) *
             texture(HeatTex, texCoord*2.0 - vec2(1, 1));

    color = (color + vec4(1.0)) / 2.0;

    FragColor = color;
}
