

#include "L_M_Model.h"

#include "objloader.hpp"

struct Character
{
	GLuint programID;
	LabManual::Model * theModel;
	LabManual::Shader * theShader;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.

	Vector3f     Pos;
	Quatf        Rot;

	// Constructor
	Character(int one) {
		initialize();
	}
	void initialize() {
		// Create and compile our GLSL program from the shaders
		//GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
		theShader = new LabManual::Shader("./shaders/shader.vs", "./shaders/shader.frag");
		programID = theShader->getProgram();

		// Get a handle for our "MVP" uniform
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");

		// Load the texture
		GLuint Texture = loadDDS("uvmap.DDS");

		// Get a handle for our "myTextureSampler" uniform
		GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
		// Read our .obj file
		bool res = loadOBJ("./resources/cube/cube.obj", vertices, uvs, normals);
		;
	}

	void update() {

	} //  { Mat = Matrix4f(Rot); Mat = Matrix4f::Translation(Pos) * Mat; return Mat; }

	void Render3(Matrix4f view, Matrix4f proj) {
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (FLOAT*)&proj);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,  (FLOAT*)&view);
		
		Matrix4f mat = Matrix4f::Translation(Pos) * Matrix4f(Rot);
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (FLOAT*)&mat);
		theModel->Draw(*theShader);

	}
};