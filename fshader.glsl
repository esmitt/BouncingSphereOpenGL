#version 330
//output color
out vec4 gl_FragColor;

//uniform values
uniform int shadingMode;
uniform int isPlastic;
uniform int lightActivate; //0 both are off, 1 light1 is on, 2 light2 is on, 3 both are on
uniform int isTextured;
uniform int isModifiedPhong;
//texture sampler
uniform sampler2D theTexture;
//coming from vertex shader
in vec3 normal;
in vec2 texCoord;
in vec3 eyeDir;
in vec3 lightDir1;
in vec3 lightDir2;
in vec3 color;
in vec3 fragPos;

void main() 
{ 
    vec3 colorFrag = vec3(0, 0, 0);
    if(lightActivate > 0) //is some light on
    {
	    vec3 ambientColor;
		vec3 specularColor;
		vec3 diffuseColor;
		float shininess;
		//material
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

	    if(shadingMode == 0) //gouraud
	    {
			colorFrag = color;
	    }
	    if(shadingMode == 1)	//regular phong
	    {
	    	vec3 n = normalize(normal);
			vec3 e = normalize(eyeDir);
			vec3 color1 = vec3(0, 0, 0);
			vec3 color2 = vec3(0, 0, 0);
			float intSpec = 0;
			
			//light 1 is the directional light
			vec3 L = normalize(lightDir1);
			float intensity = max(dot(n, L), 0.0);
			
			if (intensity > 0.0)
			{
		        // compute the half vector
				vec3 h;
		        if(isModifiedPhong == 0)
		        	h = normalize(L + e);
		        else
		        	h = reflect(-L, n);
		        // compute the specular term into spec
		        intSpec = max(dot(h,n), 0.0);
	    	}
	    	color1 = ambientColor + ((intensity *  diffuseColor)
			+ (specularColor * pow(intSpec, shininess)));	
			//color1 = L;

			//light 2 is the point light
			intensity = max(dot(n, normalize(lightDir2)), 0.0);
			intSpec = 0;
			L = normalize(lightDir2);
			if (intensity > 0.0)
			{
		        // compute the half vector
		        vec3 h;
		        if(isModifiedPhong == 0)
		        	h = normalize(L + e);
		        else
		        	h = reflect(-L, n);
		        // compute the specular term into spec
		        intSpec = max(dot(h,n), 0.0);
	    	}
	    	float distanceToLight = length(lightDir2 - fragPos);
			float kAtt = 0.2f;
			float attenuation = 1.0 / (1.0 + kAtt * pow(distanceToLight, 2));
	    	color2 = ambientColor + (attenuation *((intensity *  diffuseColor) 
			+ (specularColor * pow(intSpec, shininess))));	
			
			if(lightActivate == 1)
				colorFrag = color1;
			if(lightActivate == 2)
				colorFrag = color2;
			if (lightActivate == 3)
				colorFrag = color1 + color2;
	    }	
    }
    //texture value
    if(isTextured == 1)
    	gl_FragColor = texture(theTexture, texCoord);
    else
    	gl_FragColor = vec4(colorFrag, 1);
}