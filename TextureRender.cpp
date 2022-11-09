/***************************************************
file TextureRender.cpp
author Layne Duras
email: duras.l@digipen.edu
DigiPen login: duras.l
Course: CS200
Assignment #8
due date: 10/28/2022
***************************************************/

#include <stdexcept> // for exception, runtime_error, out_of_range

#include "TextureRender.h"

cs200::TextureRender::TextureRender(void) : mesh_face_count(0)
{
	GLint value;

	// compile fragment shader
	const char* fragment_shader_text =
		"#version 130\n\
		uniform sampler2D usamp;\
		in vec2 vtexcoord;\
		out vec4 frag_color;\
		void main(void) {\
			frag_color = texture(usamp, vtexcoord);\
     }";

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fragment_shader_text, 0);  // load shader prog on GPU
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &value);
	if (!value) {
		char buffer[1024];
		glGetShaderInfoLog(fshader, 1024, 0, buffer);
		throw std::runtime_error(buffer);
	}

	// compile vertex shader
	const char* vertex_shader_text =
		"#version 130\n\
		in vec4 position;\
		in vec2 texcoord;\
		uniform mat4 transform;\
		out vec2 vtexcoord;\
		void main() {\
			gl_Position = transform * position;\
			vtexcoord = texcoord;\
		}";

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vertex_shader_text, 0);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &value);
	if (!value) {
		char buffer[1024];
		glGetShaderInfoLog(vshader, 1024, 0, buffer);
		throw std::runtime_error(buffer);
	}

	// link shader program
	program = glCreateProgram();
	glAttachShader(program, fshader);
	glAttachShader(program, vshader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &value);
	if (!value) {
		char buffer[1024];
		glGetProgramInfoLog(program, 1024, 0, buffer);
		throw std::runtime_error(buffer);
	}
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	// shader parameter locations
	utransform = glGetUniformLocation(program, "transform");
}

cs200::TextureRender::~TextureRender(void)
{
	// delete vertex array objects used
	glDeleteVertexArrays(1, &vao);

	// delete GPU buffers used
	glDeleteBuffers(1, &texture_buffer);
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &texcoord_buffer);
	glDeleteBuffers(1, &face_buffer);

	// delete shader program
	glUseProgram(0);
	glDeleteProgram(program);
}

void cs200::TextureRender::clearFrame(const glm::vec4& c)
{
	// clear background
	glClearColor(c.x, c.y, c.z, c.w);	// background color (RGBA floats)
	glClear(GL_COLOR_BUFFER_BIT);       // clear frame buffer with background color
}

void cs200::TextureRender::loadTexture(unsigned char* rgbdata, int width, int height)
{
	// texture buffer
	glGenTextures(1, &texture_buffer);
	glBindTexture(GL_TEXTURE_2D, texture_buffer);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
		0, GL_RGB, GL_UNSIGNED_BYTE, rgbdata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void cs200::TextureRender::unloadTexture(void)
{
	glBindVertexArray(0);
}

void cs200::TextureRender::setTransform(const glm::mat4& M)
{
	glUseProgram(program);
	glUniformMatrix4fv(utransform, 1, false, &M[0][0]);
}

void cs200::TextureRender::loadMesh(const TexturedMesh& m)
{
	mesh_face_count = m.faceCount();

	// vertex buffer for standard square
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, m.vertexCount() * sizeof(glm::vec4), m.vertexArray(), GL_STATIC_DRAW);
	
	// face buffer
	glGenBuffers(1, &face_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_face_count * sizeof(glm::vec4), m.faceArray(), GL_STATIC_DRAW);

	// texture coordinate buffer 
	glGenBuffers(1, &texcoord_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
	glBufferData(GL_ARRAY_BUFFER, m.vertexCount() * sizeof(glm::vec2), m.texcoordArray(), GL_STATIC_DRAW);

	// VAO for rendering faces
	GLint aposition = glGetAttribLocation(program, "position");
	glGenVertexArrays(1, &vao);

	// start recording:
	glBindVertexArray(vao);

	// associate position attribute with vertex buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(aposition, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
	glEnableVertexAttribArray(aposition);

	// select face buffer:
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);

	// VAO for first texturing
	GLint atexcoord = glGetAttribLocation(program, "texcoord");
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(aposition, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(aposition);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
	glVertexAttribPointer(atexcoord, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(atexcoord);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
	glBindVertexArray(0);
}

void cs200::TextureRender::unloadMesh(void)
{
	// stop recording
	glBindVertexArray(0);
}

void cs200::TextureRender::displayFaces(void)
{
	//select shader program to use
	glUseProgram(program);

	// select texture buffer to use
	glBindTexture(GL_TEXTURE_2D, texture_buffer);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, mesh_face_count * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


