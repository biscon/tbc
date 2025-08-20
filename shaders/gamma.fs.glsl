#version 330

uniform sampler2D texture0;
uniform float gamma; // usually 2.2

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord) * fragColor;
    // Apply gamma correction: color^(1/gamma)
    finalColor = vec4(pow(texColor.rgb, vec3(1.0 / gamma)), texColor.a);
}
