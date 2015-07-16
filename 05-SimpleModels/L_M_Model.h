

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>





namespace LabManual
{
	GLint TextureFromFile(const char* path, string directory);

	////////////////////////////////////  Shader ////////////////////////////////////////////////////

	struct Shader
	{


		GLuint Program;
		// Constructor generates the shader on the fly

		Shader() {
			// Adding in and out texture coord variables. Removing Color.



			static const GLchar* VertexShaderSrc =
				"#version 440\n"
				"layout (location = 0) in vec3 position;\n"
				"layout (location = 1) in vec3 normal;\n"
				"layout (location = 2) in vec2 texCoords;\n"
				"layout (location = 3) uniform mat4 mvp;\n"

	
				"out vec2 TexCoords;\n"

				"void main()\n"
				"{\n"
				"   gl_Position = mvp* vec4(position, 1.0f);\n"
			//	"   oTexCoord   = TexCoord;\n"
				"}\n";


			// Removed oColor
			static const char* FragmentShaderSrc =
				"#version 440\n"
				"uniform sampler2D texture_diffuse1;\n"

				"in vec2 TexCoords;\n"
				"out vec4 color;\n"
				"void main()\n"
				"{\n"
				"   color = texture(texture_diffuse1, TexCoords);\n"
				"}\n";

			// Create the shaders
			GLuint    vshader = CreateShader(GL_VERTEX_SHADER, VertexShaderSrc);
			GLuint    fshader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSrc);
			// Assemble the program
			Program = glCreateProgram();
			glAttachShader(Program, vshader);
			glAttachShader(Program, fshader);

			glLinkProgram(Program);
			GLint r;
			glGetProgramiv(Program, GL_LINK_STATUS, &r);
			if (!r)
			{
				GLchar msg[1024];
				glGetProgramInfoLog(Program, sizeof(msg), 0, msg);
				OVR_DEBUG_LOG(("Linking shaders failed: %s\n", msg));
			}

			// Once the program is compiled and linked, you don't need the shaders anymore
			glDetachShader(Program, vshader);
			glDetachShader(Program, fshader);


		}
		Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
		{
			// 1. Retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;
			// ensures ifstream objects can throw exceptions:
			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				// Open files

				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				// Read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// Convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
			const GLchar* vShaderCode = vertexCode.c_str();
			const GLchar * fShaderCode = fragmentCode.c_str();
			// 2. Compile shaders
			GLuint vertex, fragment;
			GLint success;
			GLchar infoLog[512];
			// Vertex Shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			// Print compile errors if any
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// Fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			// Print compile errors if any
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// Shader Program
			this->Program = glCreateProgram();
			glAttachShader(this->Program, vertex);
			glAttachShader(this->Program, fragment);
			glLinkProgram(this->Program);


			// Print linking errors if any
			glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
			// Delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);

		}
		// Uses the current shader
		void Use()
		{
			glUseProgram(this->Program);
		}

		GLuint getProgram()
		{
			return this->Program;
		}

		// Taken directly from Win32_GLAppUtil.h
		GLuint CreateShader(GLenum type, const GLchar* src)
		{
			GLuint shader = glCreateShader(type);

			glShaderSource(shader, 1, &src, NULL);
			glCompileShader(shader);

			GLint r;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &r);
			if (!r)
			{
				GLchar msg[1024];
				glGetShaderInfoLog(shader, sizeof(msg), 0, msg);
				if (msg[0]) {
					OVR_DEBUG_LOG(("Compiling shader failed: %s\n", msg));
				}
				return 0;
			}

			return shader;
		}
	};
	//////////////////////////////////// Mesh /////////////////////////////////////////////////////
	struct Vertex {
		// Position
		glm::vec3 Position;
		// Normal
		glm::vec3 Normal;
		// TexCoords
		glm::vec2 TexCoords;
	};

	struct Texture {
		GLuint id;
		string type;
		aiString path;
	};
	struct Obj {

		Obj() {}

		vector<glm::vec4> suzanne_vertices;
		vector<glm::vec3> suzanne_normals;
		vector<GLushort> suzanne_elements;


		//void setup() {
		//	load_obj("./resources/monkey/monkey.obj", suzanne_vertices, suzanne_normals, suzanne_elements);
		//}

		void load_obj(const char* filename, vector<glm::vec4> &vertices, vector<glm::vec3> &normals, vector<GLushort> &elements) {
			ifstream in(filename, ios::in);
			if (!in) { cerr << "Cannot open " << filename << endl; exit(1); }

			string line;
			while (getline(in, line)) {
				if (line.substr(0, 2) == "v ") {
					istringstream s(line.substr(2));
					glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
					vertices.push_back(v);
				}
				else if (line.substr(0, 2) == "f ") {
					istringstream s(line.substr(2));
					GLushort a, b, c;
					s >> a; s >> b; s >> c;
					a--; b--; c--;
					elements.push_back(a); elements.push_back(b); elements.push_back(c);
				}
				else if (line[0] == '#') { /* ignoring this line */ }
				else { /* ignoring this line */ }
			}

			normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
			for (int i = 0; i < elements.size(); i += 3) {
				GLushort ia = elements[i];
				GLushort ib = elements[i + 1];
				GLushort ic = elements[i + 2];
				glm::vec3 normal = glm::normalize(glm::cross(
					glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
					glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
				normals[ia] = normals[ib] = normals[ic] = normal;
			}
		}

		void Render5(Matrix4f view, Matrix4f proj) {

		}

	};
	struct Mesh {

		/*  Mesh Data  */
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		/*  Functions  */
		// Constructor
		Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
		{
			this->vertices = vertices;
			this->indices = indices;
			this->textures = textures;

			// Now that we have all the required data, set the vertex buffers and its attribute pointers.
			this->setupMesh();
		}

		// Render the mesh
		void Draw(Shader shader)
		{
			// Bind appropriate textures

			GLuint diffuseNr = 1;
			GLuint specularNr = 1;
			for (GLint i = 0; i < this->textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				// Retrieve texture number (the N in diffuse_textureN)
				stringstream ss;
				string number;
				string name = this->textures[i].type;
				if (name == "texture_diffuse")
					ss << diffuseNr++; // Transfer GLuint to stream
				else if (name == "texture_specular")
					ss << specularNr++; // Transfer GLuint to stream
				number = ss.str();
				// Now set the sampler to the correct texture unit
				
			//	std::cout << "Uniform name: " << (name + number).c_str() << std::endl;
				GLuint location = glGetUniformLocation(shader.Program, (name + number).c_str());
				glUniform1i(location, i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
			}
			GLuint shiny = glGetUniformLocation(shader.Program, "material.shininess");
			// Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
			glUniform1f(shiny, 16.0f);

			// Draw mesh
			glBindVertexArray(this->VAO);
			glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			// Always good practice to set everything back to defaults once configured.
			for (GLuint i = 0; i < this->textures.size(); i++)
			{
			glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}


		/*  Render data  */
		GLuint VAO, VBO, EBO;

		/*  Functions    */
		// Initializes all the buffer objects/arrays
		void setupMesh()
		{
			// Create buffers/arrays
			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);
			glGenBuffers(1, &this->EBO);

			glBindVertexArray(this->VAO);
			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

		

			// Set the vertex attribute pointers
			// Vertex Positions
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			// Vertex Normals
			
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(1);
			// Vertex Texture Coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

			//glBindVertexArray(0);
			glBindVertexArray(0);
			//glBindBuffer(GL_ARRAY_BUFFER, 0);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		    // glEnable(GL_DEPTH_TEST);
			//  glDepthFunc(GL_LESS);
		}
	};
	/////////////////////////////////////////// Model /////////////////////////////////////////////
	struct Model
	{


		/*  Functions   */
		// Constructor, expects a filepath to a 3D model.
		Model() {}
		Model(int two)
		{
			//GLchar* path = "./resources/objects/nanosuit/nanosuit.obj";
			GLchar* path = "./resources/monkey/monkey.obj";
			this->loadModel(path);
		}

		Model(GLchar* path)
		{
			this->loadModel(path);
		}
		// Draws the model, and thus all its meshes
		void Draw(Shader shader)
		{
			for (GLuint i = 0; i < this->meshes.size(); i++)
				this->meshes[i].Draw(shader);
		}


		/*  Model Data  */
		vector<Mesh> meshes;
		string directory;
		vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

		/*  Functions   */
		// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
		void loadModel(string path)
		{
			// Read file via ASSIMP
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
			// Check for errors
			if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
				return;
			}
			// Retrieve the directory path of the filepath
			this->directory = path.substr(0, path.find_last_of('/'));

			// Process ASSIMP's root node recursively
			this->processNode(scene->mRootNode, scene);
		}

		// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode* node, const aiScene* scene)
		{
			// Process each mesh located at the current node
			for (GLuint i = 0; i < node->mNumMeshes; i++)
			{
				// The node object only contains indices to index the actual objects in the scene. 
				// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				this->meshes.push_back(this->processMesh(mesh, scene));
			}
			// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (GLuint i = 0; i < node->mNumChildren; i++)
			{
				this->processNode(node->mChildren[i], scene);
			}

		}

		Mesh processMesh(aiMesh* mesh, const aiScene* scene)
		{
			// Data to fill
			vector<Vertex> vertices;
			vector<GLuint> indices;
			vector<Texture> textures;

			// Walk through each of the mesh's vertices
			for (GLuint i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
				// Positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;
				// Normals
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
				// Texture Coordinates
				if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else
					vertex.TexCoords = glm::vec2(0.0f, 0.0f);
				vertices.push_back(vertex);
			}
			// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (GLuint i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				// Retrieve all indices of the face and store them in the indices vector
				for (GLuint j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			// Process materials
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
				// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
				// Same applies to other texture as the following list summarizes:
				// Diffuse: texture_diffuseN
				// Specular: texture_specularN
				// Normal: texture_normalN

				// 1. Diffuse maps
				vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				// 2. Specular maps
				vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			}

			// Return a mesh object created from the extracted mesh data
			return Mesh(vertices, indices, textures);
		}

		// Checks all material textures of a given type and loads the textures if they're not loaded yet.
		// The required info is returned as a Texture struct.
		vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
		{
			vector<Texture> textures;
			for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
				GLboolean skip = false;
				for (GLuint j = 0; j < textures_loaded.size(); j++)
				{
					if (textures_loaded[j].path == str)
					{
						textures.push_back(textures_loaded[j]);
						skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
						break;
					}
				}
				if (!skip)
				{   // If texture hasn't been loaded already, load it
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), this->directory);
					texture.type = typeName;
					texture.path = str;
					textures.push_back(texture);
					this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
				}
			}
			return textures;
		}



	};



	GLint TextureFromFile(const char* path, string directory)
	{
		//Generate texture ID and load texture data 
		string filename = string(path);
		filename = directory + '/' + filename;
		GLuint textureID;
		glGenTextures(1, &textureID);
		int width, height;
		unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		// Assign texture to ID
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(image);
		return textureID;
	};





}
