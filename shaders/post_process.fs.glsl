#version 330

uniform sampler2D texture0;
uniform float exposure;

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord) * fragColor;
    texColor.rgb *= exposure;
    finalColor = texColor;
}
