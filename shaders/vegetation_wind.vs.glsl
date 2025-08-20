#version 330

in vec4 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;
uniform float time;
uniform float windAmplitude;
uniform float windSpeed;
uniform float windStrength;

uniform float layerTopY;    // top of image layer in world space
uniform float layerHeight;  // height of the image layer

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    //float heightFactor = clamp(1.0 - (vertexPosition.y - layerTopY) / layerHeight, 0.0, 1.0);
    float heightFactor = pow(clamp(1.0 - (vertexPosition.y - layerTopY) / layerHeight, 0.0, 1.0), 1.5);

    float phase = vertexPosition.x * 0.05;

    float offset = sin(time * windSpeed + phase) * windAmplitude * heightFactor * windStrength;

    vec4 displaced = vertexPosition;
    displaced.x += offset;

    gl_Position = mvp * displaced;
}
