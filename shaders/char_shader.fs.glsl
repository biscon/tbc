#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

/* Source colors (from sprite) */
uniform vec3 uSkinSrc1;
uniform vec3 uSkinSrc2;
uniform vec3 uHairSrc1;
uniform vec3 uHairSrc2;
uniform vec3 uOutfitSrc1;
uniform vec3 uOutfitSrc2;
uniform vec3 uOutfitSrc3;
uniform vec3 uOutfitSrc4;

/* Destination colors (runtime-chosen) */
uniform vec3 uSkinDst1;
uniform vec3 uSkinDst2;
uniform vec3 uHairDst1;
uniform vec3 uHairDst2;
uniform vec3 uOutfitDst1;
uniform vec3 uOutfitDst2;
uniform vec3 uOutfitDst3;
uniform vec3 uOutfitDst4;

uniform float uTolerance; // e.g. 0.01

out vec4 finalColor;

bool matchColor(vec3 a, vec3 b)
{
    return distance(a, b) <= uTolerance;
}

void main()
{
    vec4 tex = texture(texture0, fragTexCoord);

    vec3 rgb = tex.rgb;

    if (matchColor(rgb, uSkinSrc1))      rgb = uSkinDst1;
    else if (matchColor(rgb, uSkinSrc2)) rgb = uSkinDst2;
    else if (matchColor(rgb, uHairSrc1)) rgb = uHairDst1;
    else if (matchColor(rgb, uHairSrc2)) rgb = uHairDst2;
    else if (matchColor(rgb, uOutfitSrc1)) rgb = uOutfitDst1;
    else if (matchColor(rgb, uOutfitSrc2)) rgb = uOutfitDst2;
    else if (matchColor(rgb, uOutfitSrc3)) rgb = uOutfitDst3;
    else if (matchColor(rgb, uOutfitSrc4)) rgb = uOutfitDst4;

    finalColor = vec4(rgb, tex.a) * fragColor;
}
