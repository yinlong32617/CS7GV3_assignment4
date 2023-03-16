// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "../../stb_image.h"
#include "../../maths_funcs.h"
#include "../../camera.h"
#include "../../shader.h"

using namespace std;

// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
//#define MESH_NAME "D:/Programming Workplace/ComputerGraphics/assimp-demo/monkeyhead_smooth.dae"

unsigned int loadTexture(const char* path, bool MipMapping);

int scr_width = 1000;
int scr_height = 600;

Camera camera(vec3(0.0, 1.0, 3.0));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

int deltaTime = 0;
int lastFrame = 0;

GLfloat rotate_x = 0.0f;
GLfloat rotate_y = 0.0f;
GLfloat rotate_z = 0.0f;

unsigned int planeVAO = 0;
unsigned int floorTextureWithMipMapping;
unsigned int floorTextureWithoutMipMapping;

Shader shader; 
bool MipMapping = false;

void rendPlane() {
	if (planeVAO == 0)
	{
		float planeVertices[] = {
			// positions            // normals         // texcoords
			 100.0f, -0.5f,  100.0f,  0.0f, 1.0f, 0.0f,  100.0f,  0.0f,
			-100.0f, -0.5f,  100.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
			-100.0f, -0.5f, -100.0f,  0.0f, 1.0f, 0.0f,   0.0f, 100.0f,

			 100.0f, -0.5f,  100.0f,  0.0f, 1.0f, 0.0f,  100.0f,  0.0f,
			-100.0f, -0.5f, -100.0f,  0.0f, 1.0f, 0.0f,   0.0f, 100.0f,
			 100.0f, -0.5f, -100.0f,  0.0f, 1.0f, 0.0f,  100.0f, 100.0f
		};

		unsigned int planeVBO;
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void init() { 

	shader.CompileShader("../CS7G03/Assignment4/simpleVertexShader.txt",
						 "../CS7G03/Assignment4/simpleFragmentShader.txt");

	floorTextureWithMipMapping = loadTexture("../resources/textures/mip_mapping.jpg", true);
	floorTextureWithoutMipMapping = loadTexture("../resources/textures/mip_mapping.jpg", false);
}


void display() {
	int currentFrame = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
		
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(camera.Zoom, (float)scr_width / (float)scr_height, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
	shader.setMat4("proj", persp_proj);
	shader.setMat4("view", view);

	mat4 model;
	float s = 0.1;
	model = identity_mat4();
	model = scale(model, vec3(s, s, s));
	//model = rotate_x_deg(model, rotate_x);
	model = rotate_y_deg(model, rotate_y);
	//model = rotate_z_deg(model, rotate_z);
	//model = translate(model, vec3(-0.1, 0, -0.2));
	shader.setMat4("model", model);

	glActiveTexture(GL_TEXTURE0);


	if (MipMapping) {
		glBindTexture(GL_TEXTURE_2D, floorTextureWithMipMapping);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, floorTextureWithoutMipMapping);
	}
	
	rendPlane();

	glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 20.0f * delta;
	//rotate_y = fmodf(rotate_y, 360.0f);

	rotate_x += 20.0f * delta;
	//rotate_x = fmodf(rotate_x, 360.0f);

	rotate_z += 20.0f * delta;
	//rotate_z = fmodf(rotate_z, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}

void keypress(unsigned char key, int x, int y) {
	float delta = deltaTime/1000.0;
	if (key == 27) {
		exit(0);
	}
	else if (key == 'w') {
		camera.ProcessKeyboard(FORWARD, delta);
	}
	else if (key == 's') {
		camera.ProcessKeyboard(BACKWARD, delta);
	}
	else if (key == 'a') {
		camera.ProcessKeyboard(LEFT, delta);
	}
	else if (key == 'd') {
		camera.ProcessKeyboard(RIGHT, delta);
	}
	else if (key = 'm') {
		MipMapping = !MipMapping;
	}
}

void PassiveMouse_Callback(int x, int y) {

	float xpos = static_cast<float>(x);
	float ypos = static_cast<float>(y);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(scr_width, scr_height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(PassiveMouse_Callback);
		
	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

unsigned int loadTexture(const char* path, bool MipMapping)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		if (MipMapping)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
