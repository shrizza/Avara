#version 330 core

in vec4 fragmentColor;
in vec3 fragmentNormal;
in vec4 gl_FragCoord;

uniform vec3 light0 = vec3(0, 0, 0);
uniform vec3 light0Color = vec3(1, 1, 1);
uniform vec3 light1 = vec3(0, 0, 0);
uniform vec3 light1Color = vec3(1, 1, 1);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light2Color = vec3(1, 1, 1);
uniform vec3 light3 = vec3(0, 0, 0);
uniform vec3 light3Color = vec3(1, 1, 1);
uniform float ambient = 0.0;
uniform vec3 ambientColor = vec3(1, 1, 1);
uniform float lightsActive = 1.0;
uniform float worldYon = 180.0;
uniform float objectYon = 180.0;
uniform vec3 horizonColor;

out vec4 color;

vec3 apply_fog(vec3 col, // color of pixel
               float t)  // distance to point
{
    float fogAmount = 1.0 - exp(-t * 0.0005);
    return mix(col, horizonColor, fogAmount);
}

vec3 diffuse_light(vec3 light, vec3 lightColor) {
    return max(dot(fragmentNormal, light), 0.0) * lightColor;
}

vec3 diffuse() {
    return diffuse_light(light0, light0Color)
            + diffuse_light(light1, light1Color)
            + diffuse_light(light2, light2Color)
            + diffuse_light(light3, light3Color);

}

vec4 light_color() {
    return mix(
        ambient * vec4(ambientColor, 1.0) * fragmentColor,
        vec4((ambient * ambientColor) + diffuse(), 1.0) * fragmentColor,
        lightsActive
    );
}

void main() {
    color = light_color();
    
    float dist = gl_FragCoord.z / gl_FragCoord.w;
    color.rgb = apply_fog(color.rgb, dist);
    
    float yonFadeRange = min(5.0, objectYon - (objectYon * 0.9));
    float yonFadeDist = objectYon - yonFadeRange;
    float alphaMult = pow(clamp((yonFadeRange + yonFadeDist - dist) / yonFadeRange, 0.0, 1.0), 0.5);
    color.a *= alphaMult;
}
