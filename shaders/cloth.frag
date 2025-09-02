#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 clothColor;
uniform bool wireframe;

void main() {
    if (wireframe) {
        FragColor = vec4(0.2, 0.2, 0.2, 1.0);
        return;
    }
    
    vec3 color = clothColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = abs(dot(norm, lightDir));
    
    vec3 result = 0.3 * color + 0.7 * diff * color;
    FragColor = vec4(result, 1.0);
}
