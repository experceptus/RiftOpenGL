

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
		//theShader = new LabManual::Shader();
		program = theShader->getProgram();
		// Load models
		theModel = new LabManual::Model("./resources/objects/nanosuit/nanosuit.obj");
		//theModel = new LabManual::Model("./resources/monkey/monkey.obj");
		// cube makes even less sense
		//theModel = new LabManual::Model("./resources/cube/cube.obj");

	}

	void update() {

	} //  { Mat = Matrix4f(Rot); Mat = Matrix4f::Translation(Pos) * Mat; return Mat; }

	void Render3(Matrix4f view, Matrix4f proj) {

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Gotta have the program
		glDisable(GL_CULL_FACE);
		//glScalef(-1, -1, -1);
		glUseProgram(program);

		//glm::mat4 model;
	//	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f)); 
		//Matrix4f combined = proj * view * Matrix4f::Translation(Pos) * Matrix4f(Rot);
		// Nano suit, with proper scaling
		Matrix4f combined = proj * view * Matrix4f::Translation(Vector3f(0, 0, 2)) * Matrix4f(Rot) * Matrix4f::Scaling(Vector3f(0.32f, 0.32f, 0.32f));//Matrix4f::RotationX(4.712);
		// Monkey with proper rotation
		//Matrix4f combined = proj * view * Matrix4f::Translation(Vector3f(0, 1, 2)) * Matrix4f::RotationX(4.712);//Matrix4f(Rot);
	

		GLuint uniLoc = glGetUniformLocation(program, "mvp");
		
		//bool isIt = glIsProgram(program);

		glUniformMatrix4fv(uniLoc, 1, GL_TRUE, (FLOAT*)&combined);

	//	glm::mat4 model;
	//	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	//	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
	//	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (FLOAT*)&mat);
		theModel->Draw(*theShader);

	}
};