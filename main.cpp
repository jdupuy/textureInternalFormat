////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
// \mail     etu.jdupuy@gmail.com
// \brief    AMD Catalyst 12.6 bug. Textures generated should be the same when 
//           rendered.
//
////////////////////////////////////////////////////////////////////////////////

// GL libraries
#include "glew.hpp"
#include "GL/freeglut.h"

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
// Global variables and types
//
////////////////////////////////////////////////////////////////////////////////

// RGBA8 type
struct Colour4 {
	Colour4() {}
	Colour4(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a):
		r(_r),g(_g),b(_b),a(_a)
	{}
	GLubyte r, g, b, a;
};

// texture resolution
const GLint TEXTURE_WIDTH  = 512;
const GLint TEXTURE_HEIGHT = 256;

enum TexNames {
	TEXTURE_DEFAULT = 0,
	TEXTURE_SOFTWARE,
	TEXTURE_COUNT
};

// OpenGL objects
GLuint vertexArray = 0;
GLuint program     = 0;
GLuint textures[TEXTURE_COUNT];

// program code
const GLchar* vertexSrc[]={
"#version 420 core\n",

"layout(location=0) out vec2 oTexCoord;",

"void main(){\n",
	"oTexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);",
	"gl_Position.xy = oTexCoord*2.0-1.0;",
	"gl_Position.zw = vec2(0,1);",
"}\n"
};

const GLchar* fragmentSrc[]={
"#version 420 core\n",

"uniform sampler2D sColour;",

"layout(location=0) in vec2 iTexCoord;",
"layout(location=0) out vec4 oColour;",

"void main(){\n",
	"oColour = texture(sColour,iTexCoord);\n",
"}\n"
};


////////////////////////////////////////////////////////////////////////////////
// Functions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
////////////////////////////////////////////////////////////////////////////////
// Convert GL error code to string
GLvoid gl_debug_message_callback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar* message,
                                 GLvoid* userParam) {
	std::cerr << "[DEBUG_OUTPUT] "
	          << message
	          << std::endl;
}
#else
const GLchar* gl_error_to_string(GLenum error) {
	switch(error) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "unknown code";
	}
}

#endif

GLubyte pack_3ub_to_ubyte_3_3_2(GLubyte r,
	                            GLubyte g,
	                            GLubyte b) {
	return (r & 0xE0u) | (g >> 3 & 0x1Cu) | (b >> 6 & 0x03u); // rrrg ggbb
}

GLubyte ub8_to_ub2(GLubyte r) {
	return r & 0xC0u;
}

GLubyte pack_4ub_to_ubyte_2_2_2_2(GLubyte r,
	                              GLubyte g,
	                              GLubyte b,
	                              GLubyte a) {
	return (r & 0xC0u) |        // rr-- ---- 
	       (g >> 2 & 0x30u) |   // rrgg ----
	       (b >> 4 & 0x0Cu) |   // rrgg bb--
	       (a >> 6 & 0x03u);    // rrgg bbaa
}

GLushort pack_4ub_to_ushort_4_4_4_4(GLubyte r,
                                    GLubyte g,
                                    GLubyte b,
                                    GLubyte a) {
	GLushort pack;
	pack = (0xF000u & (r << 8)); // rrrr ---- ---- ----
	pack|= (0x0F00u & (g << 4)); // rrrr gggg ---- ----
	pack|= (0x00F0u & b);        // rrrr gggg bbbb ----
	pack|= (0x000Fu & (a >> 4)); // rrrr gggg bbbb aaaa
	return pack;
}

GLushort pack_4ub_to_ushort_5_5_5_1(GLubyte r,
                                    GLubyte g,
                                    GLubyte b,
                                    GLubyte a) {
	GLushort pack;
	pack = (0xF800u & (r << 8)); // rrrr r--- ---- ----
	pack|= (0x07C0u & (g << 3)); // rrrr rggg gg-- ----
	pack|= (0x003Eu & (b >> 2)); // rrrr rggg ggbb bbb-
	pack|= (0x0001u & (a >> 7)); // rrrr rggg ggbb bbba
	return pack;
}

GLubyte pack_3ubv_to_ubyte_3_3_2(const GLubyte *v) {
	return pack_3ub_to_ubyte_3_3_2(v[0],v[1],v[2]);
}

GLubyte pack_4ubv_to_ubyte_2_2_2_2(const GLubyte *v) {
	return pack_4ub_to_ubyte_2_2_2_2(v[0],v[1],v[2],v[3]);
}

GLushort pack_4ubv_to_ushort_4_4_4_4(const GLubyte *v) {
	return pack_4ub_to_ushort_4_4_4_4(v[0],v[1],v[2],v[3]);
}

GLushort pack_4ubv_to_ushort_5_5_5_1(const GLubyte *v) {
	return pack_4ub_to_ushort_5_5_5_1(v[0],v[1],v[2],v[3]);
}


// flat HSL map
// http://local.wasp.uwa.edu.au/~pbourke/texture_colour/convert/
Colour4 tex_gen(GLint x, GLint y) {
	GLfloat h = GLfloat(x)*360.f / GLfloat(TEXTURE_WIDTH-1);  // hue
	GLfloat l = GLfloat(y) / GLfloat(TEXTURE_HEIGHT-1); // lightness
	GLfloat satr, satg, satb;
	GLfloat ctmpr, ctmpg, ctmpb;
	GLfloat c2r, c2g, c2b;
	if (h < 120) {
		satr = (120 - h) / 60.0;
		satg = h / 60.0;
		satb = 0;
	} else if (h < 240) {
		satr = 0;
		satg = (240 - h) / 60.0;
		satb = (h - 120) / 60.0;
	} else {
		satr = (h - 240) / 60.0;
		satg = 0;
		satb = (360 - h) / 60.0;
	}
	satr = std::min(satr,1.f);
	satg = std::min(satg,1.f);
	satb = std::min(satb,1.f);

	ctmpr = 2 * satr;
	ctmpg = 2 * satg;
	ctmpb = 2 * satb;

	if (l < 0.5) {
		c2r = l * ctmpr;
		c2g = l * ctmpg;
		c2b = l * ctmpb;
	} else {
		c2r = (1 - l) * ctmpr + 2 * l - 1;
		c2g = (1 - l) * ctmpg + 2 * l - 1;
		c2b = (1 - l) * ctmpb + 2 * l - 1;
	}

	return Colour4(c2r*255.f,c2g*255.f,c2b*255.f,255u);
}


////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init() {
#ifndef _WIN32
	// Configure debug output
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(
			reinterpret_cast<GLDEBUGPROCARB>(&gl_debug_message_callback),
			NULL );
#endif

	// gen names
	glGenVertexArrays(1, &vertexArray);
	glGenTextures(TEXTURE_COUNT, textures);
	program = glCreateProgram();

	// build empty vao
	glBindVertexArray(vertexArray);
	glBindVertexArray(0);

	// build textures
	std::vector<Colour4> colours(TEXTURE_WIDTH*TEXTURE_HEIGHT);
#if defined R4_G4_B4 || defined R5_G5_B5
	std::vector<GLushort> colours2(TEXTURE_WIDTH*TEXTURE_HEIGHT);
#elif defined R3_G3_B2
	std::vector<GLubyte> colours2(TEXTURE_WIDTH*TEXTURE_HEIGHT);
#else 
	std::vector<Colour4> colours2(TEXTURE_WIDTH*TEXTURE_HEIGHT);
#endif
	for(GLint i=0; i<TEXTURE_WIDTH; ++i)
		for(GLint j=0; j<TEXTURE_HEIGHT; ++j)
			colours[j*TEXTURE_WIDTH+i] = tex_gen(i,j);
	// convert
	for(GLuint i=0;i<colours.size();++i)
#if defined R4_G4_B4 
		colours2[i] = pack_4ubv_to_ushort_4_4_4_4(&colours[i].r);
#elif defined R5_G5_B5
		colours2[i] = pack_4ubv_to_ushort_5_5_5_1(&colours[i].r);
#elif defined R3_G3_B2
		colours2[i] = pack_3ubv_to_ubyte_3_3_2(&colours[i].r);
#elif defined R2_G2_B2
	{
		colours2[i].r = ub8_to_ub2(colours[i].r);
		colours2[i].g = ub8_to_ub2(colours[i].g);
		colours2[i].b = ub8_to_ub2(colours[i].b);
		colours2[i].a = ub8_to_ub2(colours[i].a);
	}
#else
		colours2[i] = colours[i];
#endif

	glActiveTexture(GL_TEXTURE0+TEXTURE_DEFAULT);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_DEFAULT]);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
#if defined R4_G4_B4
	             GL_RGBA4,
#elif defined R5_G5_B5
	             GL_RGB5_A1,
#elif defined R3_G3_B2
	             GL_R3_G3_B2,
#elif defined R2_G2_B2
	             GL_RGBA2,
#else
	             GL_RGBA,
#endif
	             TEXTURE_WIDTH,TEXTURE_HEIGHT,0,
	             GL_RGBA,
	             GL_UNSIGNED_BYTE,
	             &colours[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE0+TEXTURE_SOFTWARE);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SOFTWARE]);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
#if defined R4_G4_B4
	             GL_RGBA4,
#elif defined R5_G5_B5
	             GL_RGB5_A1,
#elif defined R3_G3_B2
	             GL_R3_G3_B2,
#elif defined R2_G2_B2
	             GL_RGBA2,
#else
	             GL_RGBA,
#endif
	             TEXTURE_WIDTH,TEXTURE_HEIGHT,0,
#if defined R3_G3_B2
	             GL_RGB,
#else
	             GL_RGBA,
#endif
#if defined R4_G4_B4
	             GL_UNSIGNED_SHORT_4_4_4_4,
#elif defined R5_G5_B5
	             GL_UNSIGNED_SHORT_5_5_5_1,
#elif defined R3_G3_B2
	             GL_UNSIGNED_BYTE_3_3_2,
#else
	             GL_UNSIGNED_BYTE,
#endif
	             &colours2[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// build program
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 7, vertexSrc, NULL);
	glCompileShader(vertex);
	glAttachShader(program, vertex);
	glDeleteShader(vertex);

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 7, fragmentSrc, NULL);
	glCompileShader(fragment);
	glAttachShader(program, fragment);
	glDeleteShader(fragment);

	glLinkProgram(program);

	glBindVertexArray(vertexArray);

#ifdef _WIN32
	GLenum error = glGetError();
	if(error!=GL_NO_ERROR)
		std::cerr << "caught "
		          << gl_error_to_string(error)
		          << '\n';
#endif
}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean() {
	// delete objects
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteTextures(TEXTURE_COUNT, textures);
	glDeleteProgram(program);
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update() {
	GLint location = glGetUniformLocation(program,"sColour");

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);

	for(GLint i=0; i < TEXTURE_COUNT; ++i) {
		glViewport(0, TEXTURE_HEIGHT*(TEXTURE_COUNT-1-i), TEXTURE_WIDTH, TEXTURE_HEIGHT);
		glUniform1i(location,i);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

#ifdef _WIN32
	GLenum error = glGetError();
	if(error!=GL_NO_ERROR)
		std::cerr << "caught "
		          << gl_error_to_string(error)
		          << '\n';
#endif

	glutSwapBuffers();
}


////////////////////////////////////////////////////////////////////////////////
// on resize cb
void on_resize(GLint w, GLint h) {

}


////////////////////////////////////////////////////////////////////////////////
// on key down cb
void on_key_down(GLubyte key, GLint x, GLint y) {
	if (key==27) // escape
		glutLeaveMainLoop();
}


////////////////////////////////////////////////////////////////////////////////
// on mouse button cb
void on_mouse_button(GLint button, GLint state, GLint x, GLint y) {

}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y) {

}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y) {

}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	const GLuint CONTEXT_MAJOR = 4;
	const GLuint CONTEXT_MINOR = 2;

	// init glut
	glutInit(&argc, argv);
	glutInitContextVersion(CONTEXT_MAJOR ,CONTEXT_MINOR);

	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// build window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(TEXTURE_WIDTH, TEXTURE_HEIGHT*TEXTURE_COUNT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("texture internalFormat");

	// init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::stringstream ss;
		ss << err;
		std::cerr << "glewInit() gave error " << ss.str() << std::endl;
		return 1;
	}

	// glewInit generates an INVALID_ENUM error for some reason...
	glGetError();


	// set callbacks
	glutCloseFunc(&on_clean);
	glutReshapeFunc(&on_resize);
	glutDisplayFunc(&on_update);
	glutKeyboardFunc(&on_key_down);
	glutMouseFunc(&on_mouse_button);
	glutPassiveMotionFunc(&on_mouse_motion);
	glutMotionFunc(&on_mouse_motion);
	glutMouseWheelFunc(&on_mouse_wheel);

	// run
	on_init();
	glutMainLoop();

	return 0;
}

