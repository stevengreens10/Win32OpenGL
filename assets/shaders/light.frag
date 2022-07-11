#version 450 core

layout(location = 0) out vec4 color;

// Bitmap for textures present in order of below
// e.g. 0001 for ambient
uniform int u_texturesPresent;
uniform sampler2D u_ambientTex;
uniform sampler2D u_diffuseTex;
uniform sampler2D u_specTex;
uniform sampler2D u_shinyTex;
uniform sampler2D u_bumpTex;

// Multiplied by UV, so UV can go above 1 and repeat texture
uniform float u_texScale;

// Material properties
uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specColor;
uniform float u_shininess;
uniform float u_refractionIndex;
uniform float u_alpha;

struct LightSource {
    int type;
    vec3 pos;
    vec3 dir;
    vec3 color;
    vec3 intensities;
    vec3 attenuation;
};

// Scene properties
readonly layout(std430, binding=2) buffer Scene {
    vec3 cameraPos;
    int numLights;
    LightSource lights[];
};

// Interpolated coords from vertex shader
in vec3 v_pos;
in vec2 v_textureUV;
in vec3 v_normal;

in vec4 v_posLightSpace;
uniform sampler2D u_lightDepthMap;

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

vec3 mapVec(vec3 value, float min1, float max1, float min2, float max2) {
    return vec3(map(value.x, min1, max1, min2, max2),
                map(value.y, min1, max1, min2, max2),
                map(value.z, min1, max1, min2, max2));
}

float shadowMultiplier(vec3 normal, vec3 lightDir) {
    // Perspective divide (does nothing with ortho perspective)
    vec3 projCoords = v_posLightSpace.xyz / v_posLightSpace.w;
    // Map coords from [-1, 1] to [0, 1]
    projCoords = mapVec(projCoords, -1, 1, 0, 1);
    float closestDepth = texture(u_lightDepthMap, projCoords.xy).r;
    float fragDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    if(projCoords.z > 1.0) {
        return 1.0f;
    }

    if(fragDepth - bias > closestDepth) {
        // In shadow, return 0
        return 0;
    }
    return 1.0f;
}

vec3 calculatePointLight(LightSource light, vec3 normal, vec3 ambientColor, vec3 diffuseColor, vec3 specColor, float shininess) {
    vec3 lightDir = normalize(light.pos - v_pos);

    vec3 ambient = light.color * light.intensities[0];
    vec3 diffuse = max(dot(normal, lightDir), 0) * light.color * light.intensities[1];

    vec3 viewDir = normalize(cameraPos-v_pos);
    vec3 halfDir = normalize(lightDir + viewDir);

    float specAngle = max(dot(halfDir, normal), 0);
    vec3 specular = pow(specAngle, shininess) * light.color * light.intensities[2];

    vec3 shading =  (ambient*diffuseColor) +
    (diffuse*diffuseColor) +
    (specular*specColor);

    float distance = pow(pow(v_pos.x - light.pos.x, 2) + pow(v_pos.y - light.pos.y, 2) + pow(v_pos.z - light.pos.z, 2), 0.5f);
    // Prevent div/0
    float attenFactor = max(0.01f, light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * distance * distance);
    float attenuation = 1.0 / (attenFactor);

    return attenuation * shading;
}

vec3 calculateDirectionalLight(LightSource light, vec3 normal, vec3 ambientColor, vec3 diffuseColor, vec3 specColor, float shininess) {
    vec3 lightDir = normalize(-light.dir);

    vec3 ambient = light.color * light.intensities[0];
    vec3 diffuse = max(dot(normal, lightDir), 0) * light.color * light.intensities[1];

    vec3 viewDir = normalize(cameraPos-v_pos);
    vec3 halfDir = normalize(lightDir + viewDir);

    float specAngle = max(dot(halfDir, normal), 0);
    vec3 specular = pow(specAngle, shininess) * light.color * light.intensities[2];

    float shadow = shadowMultiplier(normal, lightDir);

    vec3 shading =  (ambient*diffuseColor) +
    shadow * ((diffuse*diffuseColor) +
    (specular*specColor));

    return shading;
}

vec3 calculateLight(LightSource light, vec3 normal, vec3 ambientColor, vec3 diffuseColor, vec3 specColor, float shininess) {
    if(light.type == 0) {
        return calculatePointLight(light, normal, ambientColor, diffuseColor, specColor, shininess);
    } else if(light.type == 1) {
        return calculateDirectionalLight(light, normal, ambientColor, diffuseColor, specColor, shininess);
    }
    return vec3(0.0f);
}

void main() {
    // Load texture data
    vec3 ambientTex = vec3(1.0f);
    vec3 diffuseTex = vec3(1.0f);
    vec3 specTex = vec3(1.0f);
    float shinyTex = 1.0f;
    vec3 bumpTex = vec3(1.0f);
    if ((u_texturesPresent & 1) >= 1)
        ambientTex = vec3(texture(u_ambientTex, v_textureUV*u_texScale));
    if ((u_texturesPresent & 2) >= 1)
        diffuseTex = vec3(texture(u_diffuseTex, v_textureUV*u_texScale));
    if ((u_texturesPresent & 4) >= 1)
        specTex    = vec3(texture(u_specTex, v_textureUV*u_texScale));
    if ((u_texturesPresent & 8) >= 1)
        shinyTex   = float(texture(u_shinyTex, v_textureUV*u_texScale));
    if ((u_texturesPresent & 16) >= 1)
        bumpTex    = vec3(texture(u_bumpTex, v_textureUV*u_texScale));

    vec3 ambientColor = u_ambientColor * ambientTex;
    vec3 diffuseColor = u_diffuseColor * diffuseTex;
    vec3 specColor = u_specColor * specTex;
    float shininess = u_shininess * shinyTex;

    vec3 normal = normalize(v_normal);

    vec3 result = vec3(0.0f);
    for(int i = 0; i < numLights; i++) {
        result += calculateLight(lights[i], normal, ambientColor, diffuseColor, specColor, shininess);
    }

    color = vec4(result, u_alpha);
}