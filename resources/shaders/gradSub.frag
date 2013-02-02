#version 400

uniform sampler2D PressureTex;
uniform sampler2D VelocityTex;
uniform vec2 Size;
uniform float HalfrDx;

out vec4 FragOut;

void main(void)
{
    ivec2 pos = ivec2(gl_FragCoord.xy);

    float pL = texelFetch(PressureTex, pos - ivec2(1, 0), 0).x;
    float pR = texelFetch(PressureTex, pos + ivec2(1, 0), 0).x;
    float pB = texelFetch(PressureTex, pos - ivec2(0, 1), 0).x;
    float pT = texelFetch(PressureTex, pos + ivec2(0, 1), 0).x;

    vec4 v = texelFetch(VelocityTex, pos, 0);
    v.xy -= HalfrDx * vec2(pR - pL, pT - pB);

    FragOut = v;
}
