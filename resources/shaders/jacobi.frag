#version 400

uniform sampler2D XTex;
uniform sampler2D BTex;
uniform vec2 Size;
uniform float Alpha;
uniform float rBeta;

out vec4 FragOut;

void main(void)
{
    ivec2 pos = ivec2(gl_FragCoord.xy);
    vec4 xL = texelFetch(XTex, pos - ivec2(1, 0), 0);
    vec4 xR = texelFetch(XTex, pos + ivec2(1, 0), 0);
    vec4 xB = texelFetch(XTex, pos - ivec2(0, 1), 0);
    vec4 xT = texelFetch(XTex, pos + ivec2(0, 1), 0);

    vec4 bC = texelFetch(BTex, pos, 0);

    FragOut = (xL + xR + xB + xT + bC*Alpha)  * rBeta;
}
