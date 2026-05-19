#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3  viewPos;
uniform vec3  objectColor;
uniform vec3  lightDirection;
uniform vec3  lightAmbient;
uniform vec3  lightDiffuse;
uniform vec3  lightSpecular;
uniform float shininess;

uniform sampler2D diffuseTexture;
uniform int       useTexture;    // 0 = kolor, 1 = tekstura

void main(){
    vec3 baseColor = (useTexture == 1)
        ? texture(diffuseTexture, TexCoord).rgb
        : objectColor;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    vec3 ambient  = lightAmbient * baseColor;

    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = lightDiffuse * diff * baseColor;

    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular   = lightSpecular * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}