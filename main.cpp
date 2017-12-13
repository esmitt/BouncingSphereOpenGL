#include "Angel.h"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#define M_PI_2 (M_PI / 2.f)

float fStep = +0.002f; //amount on the rotation
float fRotation = 0; //value to rotate

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.9, 0.0 };

GLuint  theta;  //angle of rotation
int shininess;	//for the phong

//sphere data
vector<vec4> vertexSphere;
vector<vec3> normalSphere;
vector<vec2> texSphere;
vector<unsigned short> indexSphere;

//uniform matrices
mat4 model;
mat4 view;
mat4 projection;

//to connect with the GLSL program
GLuint program;
GLuint vertex_attribute_loc;
GLuint normal_attribute_loc;
GLuint tex_attribute_loc;
GLuint vertex_buffer;
GLuint normal_buffer;
GLuint texture_buffer;

//IDs to connect the uniform values
GLuint matrixMVP_ID;
GLuint matrixMV_ID;
GLuint matrixM_ID;
GLuint lightPos1_ID;
GLuint lightPos2_ID;
GLuint normalM_ID;
GLuint element_buffer;
GLuint shadingMode_ID;
GLuint plasticMode_ID;
GLuint lightActivate_ID;
GLuint texUnit_loc;
GLuint isTexture_ID;
GLuint modifiedPhong_ID;
GLuint fixedLight_ID;

//texture
unsigned char * image = NULL;
int iTextureID = 0;
GLuint textures[2]; //texture IDs

//lights
vec3 light1_pos(3, 3, 6); //light position 1
vec3 light2_pos(-3, 2, 6); //light position 2
int lightActivate = 2;	//0 both light are off, 1 light1 is on, 2, light2 is on, 3 both are on

int iShadingMode = 0;	//0 is gouraud, 1 is regular phong, 2 is modified phong
bool bIsPlastic = true;	//plastic or metallic
int bIsTextured = 0;	//0 is shading, 1 is textured
bool bModifiedPhong = false; //regular or modified phong
bool bFixedLight = false;	//is moving the light?

// function to read the PPM file, according the slides
// width and height store those values from the image read
bool readPPM(int & width, int & height,string filename)
{
	FILE* fd;
	int n, k, m;
	char c;
	int i;
	char b[100];
	float s;
	int red, green, blue;
	fd = fopen(filename.c_str(), "r");
	if (fd == NULL)
		return false;
	fscanf(fd, "%[^\n]", b);
	if (b[0] != 'P' || b[1] != '3')
	{
		fclose(fd);
		return false;
	}
	fscanf(fd, "%c", &c);
	while (c == '#')
	{
		fscanf(fd, "%[^\n]", b);
		fscanf(fd, "%c", &c);
	}
	ungetc(c, fd);
	fscanf(fd, "%d %d %d", &n, &m, &k);
	width = n;
	height = m;
	int nm = n*m;
	if (image)
		free(image);
	image = (unsigned char *)malloc(nm * 3 * sizeof(GLuint));
	for (i = nm; i > 0; i--)
	{
		fscanf(fd, "%d %d %d", &red, &green, &blue);
		image[3 * nm - 3 * i] = red;
		image[3 * nm - 3 * i + 1] = green;
		image[3 * nm - 3 * i + 2] = blue;
	}
	return true;
}

// load the image file and create the texture
GLuint loadTexture(string filename) 
{
	int width, height;
	
	if (!readPPM(width, height, filename)) return 100;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

// function to assign indexes 
void push_indices(vector<GLushort>& indices, int sectors, int r, int s)
{
	int curRow = r * sectors;
	int nextRow = (r + 1) * sectors;
	int nextS = (s + 1) % sectors;

	indices.push_back(curRow + s);
	indices.push_back(nextRow + s);
	indices.push_back(nextRow + nextS);

	indices.push_back(curRow + s);
	indices.push_back(nextRow + nextS);
	indices.push_back(curRow + nextS);
}

//it creates an sphere in parametric way
void createSphere(float radius, unsigned int rings, unsigned int sectors)
{
	float const R = 1. / (float)(rings - 1);
	float const S = 1. / (float)(sectors - 1);

	for (int r = 0; r < rings; ++r)
	{
		for (int s = 0; s < sectors; ++s)
		{
			float const y = sin(-M_PI_2 + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			texSphere.push_back(vec2(1 - s*S, 1 - r*R));
			vertexSphere.push_back(vec4(x, y, z, 1) * radius);
			normalSphere.push_back(vec3(x, y, z));
			if (r < rings - 1)
				push_indices(indexSphere, sectors, r, s);
		}
	}
}

//initialize all values
void init()
{
		createSphere(1.5, 24, 32);
		textures[0] = loadTexture("earth.ppm");
		textures[1] = loadTexture("basketball.ppm");

    // Load shaders and use the resulting shader program
    program = InitShader( "vshader.glsl", "fshader.glsl" );
   
		//sphere
		glGenBuffers(1, &vertex_buffer);
		glGenBuffers(1, &normal_buffer);
		glGenBuffers(1, &texture_buffer);
		glGenBuffers(1, &element_buffer);

		//create the buffers
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER,  vertexSphere.size() * sizeof(vec4), &vertexSphere[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER,  normalSphere.size() * sizeof(vec3), &normalSphere[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, texSphere.size() * sizeof(vec2), &texSphere[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indexSphere.size(), &indexSphere[0], GL_STATIC_DRAW);

		//get all attributes and uniforms locations
		vertex_attribute_loc = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(vertex_attribute_loc);

		normal_attribute_loc = glGetAttribLocation(program, "vNormal");
		glEnableVertexAttribArray(normal_attribute_loc);

		tex_attribute_loc = glGetAttribLocation(program, "vTexCoord");
		glEnableVertexAttribArray(tex_attribute_loc);
		
		texUnit_loc = glGetUniformLocation(program, "theTexture");

		matrixMVP_ID = glGetUniformLocation(program, "MVP");
		matrixMV_ID = glGetUniformLocation(program, "MV");
		matrixM_ID = glGetUniformLocation(program, "M");
		lightPos1_ID = glGetUniformLocation(program, "lightPos1");
		lightPos2_ID = glGetUniformLocation(program, "lightPos2");
		normalM_ID = glGetUniformLocation(program, "normalMatrix");
		shadingMode_ID = glGetUniformLocation(program, "shadingMode");
		plasticMode_ID = glGetUniformLocation(program, "isPlastic");
		lightActivate_ID = glGetUniformLocation(program, "lightActivate");
		isTexture_ID = glGetUniformLocation(program, "isTextured");
		modifiedPhong_ID = glGetUniformLocation(program, "isModifiedPhong");
		fixedLight_ID = glGetUniformLocation(program, "fixedLight");
		glEnable( GL_DEPTH_TEST );
		glEnable(GL_CULL_FACE);
}

//drawing function
void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor(0.1f, 0.1f, 0.25f, 1.0f);

		//basic transformation
		view = Translate(0, 0, -4);
		model = Translate(vec3(0, Theta[Yaxis], 0))*RotateY(fRotation);
		fRotation += 0.05;
		if (fRotation > 360)
			fRotation -= 360;
		
		//modelview
		mat4 MV = view * model;
		mat3 normalMatrix = Normal(MV);
		
		mat4 MVP = projection * view * model;
		
		glUseProgram(program);
			glEnableVertexAttribArray(vertex_attribute_loc);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glVertexAttribPointer(vertex_attribute_loc, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), BUFFER_OFFSET(0));

			glEnableVertexAttribArray(tex_attribute_loc);
			glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
			glVertexAttribPointer(tex_attribute_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), BUFFER_OFFSET(0));
		
			glEnableVertexAttribArray(normal_attribute_loc);
			glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
			glVertexAttribPointer(normal_attribute_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

			//pass uniforms
			glUniformMatrix4fv(matrixMVP_ID, 1, GL_TRUE, &MVP[0][0]);
			glUniformMatrix4fv(matrixMV_ID, 1, GL_TRUE, &MV[0][0]);
			glUniformMatrix4fv(matrixM_ID, 1, GL_TRUE, &model[0][0]);
			glUniformMatrix3fv(normalM_ID, 1, GL_TRUE, &normalMatrix[0][0]);
			glUniform3fv(lightPos1_ID, 1, light1_pos);
			glUniform3fv(lightPos2_ID, 1, light2_pos);
			glUniform1i(lightActivate_ID, lightActivate);
			glUniform1i(shadingMode_ID, iShadingMode);
			glUniform1i(plasticMode_ID, bIsPlastic);
			glUniform1i(isTexture_ID, bIsTextured);
			glUniform1i(modifiedPhong_ID, bModifiedPhong);
			glUniform1i(fixedLight_ID, bFixedLight);
		
			glUniform1i(texUnit_loc, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[iTextureID]);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
			glDrawElements(GL_TRIANGLES, indexSphere.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

		glUseProgram(0);
    glutSwapBuffers();
}

//keyboard function to escape
void keyboard(unsigned char key, int x, int y){
    switch(key) {
    case 033: // Escape Key
    case 'q':
        exit(EXIT_SUCCESS);
        break;
    }
}

//resizing the window
void reshape(int w, int h){

    if(h == 0){
        h = 1;
    }
    float ratio = 1.0* w / (float)h;
    glViewport(0, 0, w, h);
		projection = Perspective(50.f, ratio, 0.5, 1000);
}

//to up and down
void idle(void){

	Theta[Yaxis] += fStep;

	if (Theta[Yaxis] > 1 || Theta[Yaxis] < -1)
		fStep = -1 * fStep;
	
    glutPostRedisplay();
}

//GLUT menu functionalities using the right click
void menu(int id)
{
    if(id == 1){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        printf("Wireframe\n");
    }else if(id == 2){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				bIsTextured = 0;
        printf("Shading\n");
    }else if(id == 3){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				bIsTextured = 1;
				printf("Texture\n");
    }else if(id == 4){
			iShadingMode = 0;
			printf("Gouraud Shading\n");
    }else if (id == 5) {
			iShadingMode = 1;
			printf("Phong Shading\n");
		}
		else if (id == 6) {	//modified phone
			bModifiedPhong = 1;
			printf("Phong\n");
		}	
		else if (id == 7) {	// regular phong
			bModifiedPhong = 0;
			printf("Modified Phong\n");
		}
		else if (id == 8) {
			if (lightActivate == 0 || lightActivate == 1 || lightActivate == 2)
			{
				printf("Turn On Both Lights\n");
				lightActivate = 3;
			}
			else if (lightActivate == 3 || lightActivate == 1 || lightActivate == 2)
			{
				printf("Turn Off Both Lights\n");
				lightActivate = 0;
			}
		}
		else if (id == 9) {
			if (lightActivate == 0 || lightActivate == 2)
			{
				printf("Turn On Light 1\n");
				if (lightActivate == 0)
					lightActivate = 1;
				else
					lightActivate = 3;
			}
			else if (lightActivate == 1 || lightActivate == 3)
			{
				printf("Turn Off Light 1\n");
				if (lightActivate == 1)
					lightActivate = 0;
				else
					lightActivate = 2;
			}
		}
		else if (id == 10) {
			if (lightActivate == 0 || lightActivate == 1)
			{
				printf("Turn On Light 2\n");
				if (lightActivate == 0)
					lightActivate = 2;
				else
					lightActivate = 3;
			}
			else if (lightActivate == 2 || lightActivate == 3)
			{
				printf("Turn Off Light 2\n");
				if (lightActivate == 2)
					lightActivate = 0;
				else
					lightActivate = 1;
			}
		}
		else if (id == 11) {
			bFixedLight = !bFixedLight;
		}
		else if (id == 12) {
			bIsPlastic = 1;
			printf("Plastic Material\n");
		}
		else if (id == 13) {
			bIsPlastic = 0;
			printf("Metallic Material\n");
		}
		else if (id == 14) {
			iTextureID = 0;
			printf("Earth texture\n");
		}
		else if (id == 15) {
			iTextureID = 1;
			printf("Basketball texture\n");
		}
}

//it creates the menu in an ordered way
void createMenu(){
    
		int displayMode = glutCreateMenu(menu);
		glutAddMenuEntry("Wireframe", 1);
		glutAddMenuEntry("Shading", 2);
		glutAddMenuEntry("Texture", 3);

		int shadingMode = glutCreateMenu(menu);
		glutAddMenuEntry("Gouraud", 4);
		glutAddMenuEntry("Phong", 5);

		int phoneMode = glutCreateMenu(menu);
		glutAddMenuEntry("Phong", 6);
		glutAddMenuEntry("Modified Phong", 7);

		int lightMode = glutCreateMenu(menu);
		glutAddMenuEntry("Turn On/Off Lights", 8);
		glutAddMenuEntry("Turn On/Off Light #1", 9);
		glutAddMenuEntry("Turn On/Off Light #2", 10);
		glutAddMenuEntry("Fixed/Moved Lights", 11);

		int materialMode = glutCreateMenu(menu);
		glutAddMenuEntry("Plastic Material", 12);
		glutAddMenuEntry("Metallic Material", 13);

		int texturingMode = glutCreateMenu(menu);
		glutAddMenuEntry("Earth.ppm", 14);
		glutAddMenuEntry("Basketball.ppm", 15);

		int mainMenu = glutCreateMenu(menu);
		glutAddSubMenu("Display mode", displayMode);
		glutAddSubMenu("Shading mode", shadingMode);
		glutAddSubMenu("Reflection mode", phoneMode);
		glutAddSubMenu("Lights", lightMode);
		glutAddSubMenu("Materials", materialMode);
		glutAddSubMenu("Texture", texturingMode);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// main function
int main(int argc, char **argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(512, 512);
    glutCreateWindow("Bouncing Ball");

    glewExperimental = GL_TRUE;
    glewInit();
    init();

    // Menu Function
    createMenu();
		glClearColor(0.1f, 0.1f, 0.25f, 1.0f);

    // Callback Functions    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
