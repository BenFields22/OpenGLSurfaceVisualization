#version 410 core

out vec4 color;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

vec3 objectColor;

float remap( float minval, float maxval, float curval )
{
    return ( curval - minval ) / ( maxval - minval );
}


const vec3 GREEN = vec3( 0.0, 0.0, 1.0 );
const vec3 WHITE = vec3( 1.0, 1.0, 0.0 );
const vec3 RED   = vec3( 1.0, 0.0, 0.0 );

uniform int wireframe;


void main()
{
    if( FragPos.y < 0.25)
        objectColor = mix( GREEN, WHITE, remap( 0.0, 0.25, FragPos.y ) );
    else
        objectColor = mix( WHITE, RED, remap( 0.25, 1.0, FragPos.y ) );
    // Ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.9f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    if (wireframe == 1) {
        objectColor = vec3(0.0f,0.0f,0.0f);
    }
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    
    color = vec4(result, 1.0f);
}
