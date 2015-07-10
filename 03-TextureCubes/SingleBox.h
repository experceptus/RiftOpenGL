/************************************************************************************

There is no warranty.
Your mileage may vary.
Caveat Emptor.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/

struct SingleBox
{
	// Define opengl ids
	GLuint program;
	GLuint vertId;
	GLuint indexId;

	// new texture ids
	GLuint boxTextureId;
	GLuint texCoordId;

	// new OpenCV Mat
	cv::Mat image;


	// Position and Rotation
	Vector3f     Pos;
	Quatf        Rot;

	// We are only doing one box per object

	Vector3f cube_vertices[24];     
	GLushort cube_indices[36];

	// Texture Coordinates array
	GLfloat cube_texCoordinates[48];


	// Constructor - Based on the Model object in Win32_GLAppUtil.h
	// Modified in this lab to take the path of the texture image
	SingleBox(float x1, float y1, float z1, float x2, float y2, float z2, const char * imagepath)

	{

		// Adding in and out texture coord variables. Removing Color.
		static const GLchar* VertexShaderSrc =
			"#version 440\n"
			"layout (location = 0) uniform mat4 matWVP;\n"
			"layout (location = 1) in      vec4 Position;\n"
			"layout (location = 2) in      vec2 TexCoord;\n"
			"out     vec2 oTexCoord;\n"

			"void main()\n"
			"{\n"
			"   gl_Position = (matWVP * Position);\n"
			"   oTexCoord   = TexCoord;\n"
			"}\n";


		// Removed oColor
		static const char* FragmentShaderSrc =
			"#version 440\n"
			"uniform sampler2D Texture0;\n"
	
			"in      vec2      oTexCoord;\n"
			"out     vec4      FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor =  texture2D(Texture0, oTexCoord);\n"
			"}\n";

		// Create the shaders
		GLuint    vshader = CreateShader(GL_VERTEX_SHADER, VertexShaderSrc);
		GLuint    fshader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSrc);


		// Assemble the program
		program = glCreateProgram();
		glAttachShader(program, vshader);
		glAttachShader(program, fshader);

		glLinkProgram(program);
		GLint r;
		glGetProgramiv(program, GL_LINK_STATUS, &r);
		if (!r)
		{
			GLchar msg[1024];
			glGetProgramInfoLog(program, sizeof(msg), 0, msg);
			OVR_DEBUG_LOG(("Linking shaders failed: %s\n", msg));
		}

		// Once the program is compiled and linked, you don't need the shaders anymore
		glDetachShader(program, vshader);
		glDetachShader(program, fshader);

		// load the image data
		image = loadImage(imagepath);

		// You may need to uncomment these lines for some images. 
		//use fast 4-byte alignment (default anyway) if possible
		//glPixelStorei(GL_UNPACK_ALIGNMENT, (image.step & 3) ? 1 : 4);

		//set length of one complete row in data (doesn't need to equal image.cols)
		//glPixelStorei(GL_UNPACK_ROW_LENGTH, image.step / image.elemSize());

		// create a boxTextureId
		glGenTextures(1, &boxTextureId);
		glBindTexture(GL_TEXTURE_2D, boxTextureId);

		// The texture minifying function is used whenever the pixel being textured
		// maps to an area greater than one texture element.
		// GL_LINEAR returns the weighted average of the four texture elements
		// that are closest to the center of the pixel being textured.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		//The texture magnification function is used when the pixel being textured
		//maps to an area less than or equal to one texture element.
		//GL_NEAREST returns the value of the texture element that is nearest
		//(in Manhattan distance) to the center of the pixel being textured.

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Set texture clamping method
		// Check out a very detailed explaination at:
		// https://open.gl/textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


		glTexImage2D(GL_TEXTURE_2D,     // Type of texture
				0,                 // Pyramid level (for mip-mapping) - 0 is the top level
				GL_RGB,            // Internal colour format to convert to
				image.cols,          // Image width 
				image.rows,          // Image height 
				0,                 // Border width in pixels (can either be 1 or 0)
				//GL_RGB, // Try RGB for inverse images, the Number Box looks better in reverse...
				GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
				GL_UNSIGNED_BYTE,  // Image data type
				image.ptr());        // The actual image data itself

		glGenerateMipmap(GL_TEXTURE_2D);
		



		// Build the box

		AddColorBox(x1, y1, z1, x2, y2, z2);

		// Create the vertex buffer
		glGenBuffers(1, &vertId);

		// Bind it
		glBindBuffer(GL_ARRAY_BUFFER, vertId);

		// fill it with data
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

		// same as above, for the index array
		glGenBuffers(1, &indexId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

		// same for the Texture Coordinates
		glGenBuffers(1, &texCoordId);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texCoordinates), cube_texCoordinates, GL_STATIC_DRAW);



	}  // end NewBox ctor

	cv::Mat  loadImage(const char * imagepath) {

		printf("Reading image %s\n", imagepath);	
		image = cv::imread(imagepath);
		cv::Mat flipped;
		cv::flip(image, flipped, -1);
		image = flipped;
		if (image.empty()){
			std::cout << "image empty" << std::endl;
		}
		return image;
		
	} // end loadImage

	// Now holding the vertex data in an array of Vector3f
	void AddColorBox(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		// For
		Vector3f vertices[] = {

			// Top 
			Vector3f(x1, y2, z1),
			Vector3f(x2, y2, z1),
			Vector3f(x2, y2, z2),
			Vector3f(x1, y2, z2),

			Vector3f(x1, y1, z1),
			Vector3f(x2, y1, z1),
			Vector3f(x2, y1, z2),
			Vector3f(x1, y1, z2),
			
			Vector3f(x1, y1, z2),
			Vector3f(x1, y1, z1),
			Vector3f(x1, y2, z1),
			Vector3f(x1, y2, z2),
		
			Vector3f(x2, y1, z2),
			Vector3f(x2, y1, z1),
			Vector3f(x2, y2, z1),
			Vector3f(x2, y2, z2),
		
			Vector3f(x1, y1, z1),
			Vector3f(x2, y1, z1),
			Vector3f(x2, y2, z1),
			Vector3f(x1, y2, z1),

			Vector3f(x1, y1, z2),
			Vector3f(x2, y1, z2),
			Vector3f(x2, y2, z2),
			Vector3f(x1, y2, z2),
		
		};

		// uncomment to see the vertex values for each box
		//for (int i = 0; i < 24; i++) {
		//	printf("Vector %i (%3.1f,%3.1f,%3.1f)\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
		//}

		GLfloat texCoords[48] = {

			// U,V coordinates are in 2D and range from 0 to 1. In OpenGL U = x, V = y. 
			// Each block of 4 points represents one face of the cube. 
			// front		
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,

			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,

			0.0, 0.0,
			0.5, 0.0,
			0.5, 0.5,
			0.0, 0.5,

			0.0, 0.0,
			2.0, 0.0,
			2.0, 2.0,
			0.0, 2.0,

			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,

			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,
		};


		// 36 points to describe 12 triangles 

	
		GLushort indices[] = { 0, 1, 3,
			3, 1, 2, 
			5, 4, 6, 
			6, 4, 7,
			8, 9, 11,
			11, 9, 10, 
			13, 12, 14, 
			14, 12, 15,
			16, 17, 19,
			19, 17, 18,
			21, 20, 22, 
			22, 20, 23 };
		
		for (int i = 0; i < 36; i++)
			cube_indices[i] = indices[i];

		for (int v = 0; v < 24; v++)
			cube_vertices[v] = vertices[v];

		for (int u = 0; u < 48; u++)
			cube_texCoordinates[u] = texCoords[u];
		
	

		
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


	// called Render2 just to differentiate it from Render in the Model struct, since the both take the same args
	void Render2(Matrix4f view, Matrix4f proj)
	{
	

		// Gotta have the program
		glUseProgram(program);

		// Set the active texture. 
		glActiveTexture(GL_TEXTURE0);

		// For fun. Change the order of the matrix multiplication. 
		// Put Rot ahead of Pos. Don't forget to duck...
		Matrix4f combined = proj * view * Matrix4f::Translation(Pos) * Matrix4f(Rot);

		// Get the location handle to the uniform matWVP
		// This should always be zero because it was set using the layout keyword 
		// in the vertex shader
		GLuint uniLoc = glGetUniformLocation(program, "matWVP");

		// Load the new data on the GPU
		// arg1 = location
		// arg2 = number of matrices
		// arg3 = Need to be transposed? Try changing it to GL_FALSE. Now you REALLY need to duck...
		// arg4 = pointer to data
		glUniformMatrix4fv(uniLoc, 1, GL_TRUE, (FLOAT*)&combined);

		// Bind the GL_TEXTURE_2D target to the Texture Id that points to the texture buffer on the GPU
		glBindTexture(GL_TEXTURE_2D, boxTextureId);

		// Bind the vertex array to the target
		glBindBuffer(GL_ARRAY_BUFFER, vertId);
		// Get the handle to the shader variable Postion from the program
		GLuint posLoc = glGetAttribLocation(program, "Position");
		// Describe the data and assign the Postion variable to the data at the GL_ARRAY_BUFFER
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), 0);
		// Enable the variable
		glEnableVertexAttribArray(posLoc);
		
		// Now, Bind A different id to the GL_ARRAY_BUFFER, all subseqent calls will reference the data 
		// pointed to by textCoordId
		glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
		GLuint uvLoc = glGetAttribLocation(program, "TexCoord");

		// defines the array of postion data

		// arg1 handle
		// arg2 size
		// arg3 type
		// arg4 transpose?
		// arg5 stride. I.E. the distance between the start of the data points
		//              Passing in zero tells opengl to calculate the stride 
		//              based on the size and type (args 2 and 3)
		// arg6 pointer to data. Here, the data starts at location zero in the array.
		// The last arg is the offset into the array that is currently 
		// bound to GL_ARRAY_BUFFER
		
		glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(uvLoc);

		// bind the indices array
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);

		// An example of how to get information about data back from the GPU
		int indicesArraySize;  
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indicesArraySize);


		// Finally, finally... draw something. 
		// arg1 What are we drawing? Triangles
		// arg2 How many are we drawing? 36 points that define 12 triangles
		// arg3 What type of data is this?
		// arg4 pointer to index data, start at the first element in the array. 

    	glDrawElements(GL_TRIANGLES, indicesArraySize / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);


		// Clean up after each draw call
		glDisableVertexAttribArray(posLoc);
		glDisableVertexAttribArray(uvLoc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

};
