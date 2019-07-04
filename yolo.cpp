/**
 * https://www3.ntu.edu.sg/home/ehchua/programming/opengl/HowTo_OpenGL_C.html
 * GL01Hello.cpp: Test OpenGL C/C++ Setup
 */
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <IL/il.h>
const std::string FragmentShaderCode =
#include "yolo.fs"
;
const std::string VertexShaderCode =
#include "yolo.vs"
;

GLuint programID;
GLuint texid;

GLuint LoadShaders() {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
  	std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
  	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }

  // Compile Fragment Shader
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ) {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    printf("%s\n", &FragmentShaderErrorMessage[0]);
  }

  // Link the program
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
  std::vector<char> ProgramErrorMessage(InfoLogLength+1);
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

/* Load an image using DevIL and return the devIL handle (-1 if failure) */
int LoadImage(char *filename) {
  ILboolean success;
  ILuint image;

  ilGenImages(1, &image);          /* Generation of one image name */
  ilBindImage(image);              /* Binding of image name */
  success = ilLoadImage(filename); /* Loading of the image filename by DevIL */

  if (success) /* If no error occured: */
  {
    /* Convert every colour component into unsigned byte. If your image contains
     * alpha channel you can replace IL_RGB with IL_RGBA */
    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    if (!success) {
      return -1;
    }
  } else
    return -1;

  return image;
}

// This will identify our vertex buffer
GLuint vertexbuffer;

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

/**
 * Handler for window-repaint event. Call back when the window first appears and
 * whenever the window needs to be re-painted.
 */
void display() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
//  glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
  // Draw a Red 1x1 Square centered at origin
  glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex2f(-0.5f, -0.5f);    // x, y
    glVertex2f( 0.5f, -0.5f);
    glVertex2f( 0.5f,  0.25f);
    glVertex2f(-0.5f,  0.5f);
  glEnd();
*/
  glEnable(GL_DEPTH_TEST);
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(
     0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
     3,                  // size
     GL_FLOAT,           // type
     GL_FALSE,           // normalized?
     0,                  // stride
     (void*)0            // array buffer offset
  );
  // Draw the triangle !
  glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
  glDisableVertexAttribArray(0);

  glUseProgram(programID);
  GLint uniform = glGetUniformLocation(programID, "u_time");
  glUniform1f( uniform, glutGet(GLUT_ELAPSED_TIME) );

  glBindTexture(GL_TEXTURE_2D, texid); /* Binding of texture name */
  GLint imageLoc = glGetUniformLocation(programID, "u_image");
  glUniform1i( imageLoc, 0 );

  glFlush();  // Render now
}

/**
 * Main function: GLUT runs as a console application starting at main()
 */
int main(int argc, char** argv) {
  glutInit(&argc, argv);                 // Initialize GLUT
  glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
  glutInitWindowSize(320, 320);   // Set the window's initial width & height
  glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
  glewInit();
  programID = LoadShaders();
  // Generate 1 buffer, put the resulting identifier in vertexbuffer
  glGenBuffers(1, &vertexbuffer);
  // The following commands will talk about our 'vertexbuffer' buffer
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  // Give our vertices to OpenGL.
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  ILuint image;
  ilInit();
  image = LoadImage("tex.png");
  /* OpenGL texture binding of the image loaded by DevIL  */
  glGenTextures(1, &texid);            /* Texture name generation */
  glBindTexture(GL_TEXTURE_2D, texid); /* Binding of texture name */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR); /* We will use linear interpolation for
                                 magnification filter */
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      GL_LINEAR); /* We will use linear interpolation for minifying filter */
  glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
               ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
               ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
               ilGetData()); /* Texture specification */

  glutDisplayFunc(display); // Register display callback handler for window re-paint
  glutIdleFunc(display);    // Repaint continuously
  glutMainLoop();           // Enter the infinitely event-processing loop

  /* Delete used resources and quit */
  ilDeleteImages(
      1, &image); /* Because we have already copied image data into texture data
                     we can release memory used by image. */
  glDeleteTextures(1, &texid);

  return 0;
}
