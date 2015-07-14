

#include "L_M_Model.h"



struct Character
{
	GLuint program;
	LabManual::Model * theModel;
	LabManual::Shader * theShader;

	Vector3f     Pos;
	Quatf        Rot;

	// Constructor
	Character(int one) {
		initialize();
	}
	void initialize() {
		// Setup and compile our shaders
		theShader = new LabManual::Shader("./shaders/shader.vs", "./shaders/shader.frag");
		 
		program = theShader->getProgram();
		// Load models
		//theModel = new LabManual::Model("./resources/objects/nanosuit/nanosuit.obj");
		theModel = new LabManual::Model("./resources/monkey/monkey.obj");

	}

	void update() {

	} //  { Mat = Matrix4f(Rot); Mat = Matrix4f::Translation(Pos) * Mat; return Mat; }

	void Render3(Matrix4f view, Matrix4f proj) {
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (FLOAT*)&proj);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,  (FLOAT*)&view);
		
		Matrix4f mat = Matrix4f::Translation(Pos) * Matrix4f(Rot);
		glm::mat4 model;
		//model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (FLOAT*)&mat);
		theModel->Draw(*theShader);

	}
};