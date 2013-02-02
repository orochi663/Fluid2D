#version 400

uniform sampler2D FragInTex;
uniform sampler2D VelocityTex;
uniform sampler2D FrontierTex;
uniform vec2 Size;
uniform float rDx;
uniform float Dt;

out vec4 FragOut;

void main(void)
{
    vec2 nPos = gl_FragCoord.xy - Dt * rDx *
            texelFetch(VelocityTex, ivec2(gl_FragCoord.xy), 0).xy;

    nPos = mix(nPos,
               gl_FragCoord.xy,
               texelFetch(FrontierTex, ivec2(nPos), 0).x);

    FragOut = texture(FragInTex, nPos / Size);
}
