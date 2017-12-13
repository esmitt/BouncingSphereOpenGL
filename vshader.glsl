#version 330

in vec4 vPosition;
in vec3 vNormal;	
in vec2 vTexCoord;

// Model-View-Projection matrix and uniform values
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 M;
uniform mat3 normalMatrix;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform int shadingMode;
uniform int isModifiedPhong;
uniform int isPlastic;
uniform int lightActivate; //0 both are off, 1 light1 is on, 2 light2 is on, 3 both are on
uniform int fixedLight;
//output
out vec3 normal;
out vec3 eyeDir;
out vec3 lightDir1;
out vec3 lightDir2;
out vec3 reflected;
out vec3 color;
out vec2 texCoord;
out vec3 fragPos;

void main() 
{
    texCoord = vTexCoord;
    if(lightActivate > 0)
    {
    	normal = normalize(normalMatrix * vNormal);
		vec4 P = MV * vPosition;
	
		color = vec3(0, 0, 0);

	    if(shadingMode == 0) // is gouraud mode?
	    {
			vec3 ambientColor;
			vec3 specularColor;
	    	vec3 diffuseColor;
	    	float shininess;

	    	//plastic or metallic
			if(isPlastic == 1)
			{
				ambientColor = vec3(0, 0, 0);
				diffuseColor = vec3(0.5,0.5,0);
				specularColor = vec3(0.6,0.6,0.5);
				shininess = 32;
			}
			else
			{
				ambientColor = vec3(0.25, 0.25, 0.25);
				diffuseColor = vec3(0.4, 0.4, 0.4);
				specularColor = vec3(0.774597, 0.774597, 0.774597);
				shininess = 76.8;
			}

			vec3 eye = normalize(-P.xyz);
			vec3 color1 = vec3(0, 0, 0);
			vec3 color2 = vec3(0, 0, 0);

			//light 1 is the directional light
			if(fixedLight == 1)
				lightDir1 = normalize(inverse(transpose(MV)) * vec4(lightPos1,0)).xyz;
			else
				lightDir1 = normalize(lightPos1);
			
			float intensity1 = max(dot(normal, lightDir1), 0.0);
			float intSpec = 0;
			if (intensity1 > 0.0)
			{
				vec3 h;
		        if(isModifiedPhong == 0)
		        	h = normalize(lightDir1 + eye);
		        else
		        	h = reflect(-lightDir1, normal);
				intSpec = max(dot(h, normal), 0.0);
			}
			color1 = ambientColor + (intensity1 *  diffuseColor) 
			+ (specularColor * pow(intSpec, shininess));
			
			//light 2 is the point light
			if(fixedLight == 1)
				lightDir2 = normalize(inverse(transpose(MV)) * vec4(lightPos2 - P.xyz,0)).xyz;
			else
				lightDir2 = normalize(lightPos2- P.xyz);

			float intensity2 = max(dot(normal, lightDir2), 0.0);
			intSpec = 0;
			if (intensity2 > 0.0)
			{
				vec3 h;
		        if(isModifiedPhong == 0)
		        	h = normalize(lightDir2 + eye);
		        else
		        	h = reflect(-lightDir2, normal);
				intSpec = max(dot(h, normal), 0.0);
			}
			//attenuation
			float distanceToLight = length(lightPos2 - (M*vPosition).xyz);
			float kAtt = 0.2f;
			float attenuation = 1.0 / (1.0 + kAtt * pow(distanceToLight, 2));
			color2 = ambientColor + (attenuation * ((intensity2 *  diffuseColor)
									+ (specularColor * pow(intSpec, shininess))));
								  

			if(lightActivate == 1)	//light 1 on
				color = color1;
			if(lightActivate == 2)	//light 2 on
				color = color2;	
			if(lightActivate == 3)	//both are on
				color = color1 + color2;
	    }
	 	else if(shadingMode == 1) //phong normal
	    {
			eyeDir = -P.xyz;
			if(fixedLight == 1)
				lightDir1 = normalize(inverse(transpose(MV)) * vec4(lightPos1,0)).xyz;
			else
				lightDir1 = lightPos1;

			if(fixedLight == 1)
				lightDir2 = normalize(inverse(transpose(MV)) * vec4(lightPos2 - P.xyz,0)).xyz;
			else
				lightDir2 = normalize(lightPos2- P.xyz);
			fragPos = (M*vPosition).xyz;
	    }
    }
    gl_Position = MVP * vPosition;
} 