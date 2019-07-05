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
GLuint positionbuffer;
GLuint coordbuffer;
GLuint elementbuffer;

static const GLfloat g_position_buffer_data[] = {
  -1.0f, -1.0f,
   1.0f, -1.0f,
   1.0f,  1.0f,
  -1.0f,  1.0f,
};

// An array of 4 vectors which represents texture coordinates
static const GLfloat g_coord_buffer_data[] = {
  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,
};

static const GLuint g_element_buffer_data[] = {
  0, 1, 2,
  0, 2, 3,
};

int lastTime = 0;
int nbFrames = 0;

/**
 * Handler for window-repaint event. Call back when the window first appears and
 * whenever the window needs to be re-painted.
 */
void display() {
  // Measure speed
  int currentTime = glutGet(GLUT_ELAPSED_TIME);
  nbFrames++;
  if (nbFrames == 200){
    double deltaT = double(currentTime-lastTime);
    char title[200];
    sprintf(title, "%f ms/frame - %f fps", deltaT/200.0, 200000.0/deltaT);
    glutSetWindowTitle(title);
    nbFrames = 0;
    lastTime = currentTime;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  GLint uniform = glGetUniformLocation(programID, "u_time");
  glUniform1f( uniform, glutGet(GLUT_ELAPSED_TIME) );

  GLint position = glGetAttribLocation(programID, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, positionbuffer);
  glEnableVertexAttribArray(position);
  glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  GLint texcoord = glGetAttribLocation(programID, "a_texCoord");
  glBindBuffer(GL_ARRAY_BUFFER, coordbuffer);
  glEnableVertexAttribArray(texcoord);
  glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

  glUseProgram(programID);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texid); /* Binding of texture name */
  GLint imageLoc = glGetUniformLocation(programID, "u_image");
  glUniform1i( imageLoc, 0 );

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(texcoord);
  glDisableVertexAttribArray(position);

  glFlush();  // Render now
}

void initBuffers() {
  glGenBuffers(1, &positionbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, positionbuffer);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(g_position_buffer_data),
    g_position_buffer_data,
    GL_STATIC_DRAW
  );
  glGenBuffers(1, &coordbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, coordbuffer);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(g_coord_buffer_data),
    g_coord_buffer_data,
    GL_STATIC_DRAW
  );

  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(g_element_buffer_data),
    g_element_buffer_data,
    GL_STATIC_DRAW
  );
}

/**
 * Main function: GLUT runs as a console application starting at main()
 */
int main(int argc, char** argv) {
  glutInit(&argc, argv);                 // Initialize GLUT
  glutCreateWindow("OpenGL Speed Test"); // Create a window with the given title
  glutInitWindowSize(320, 320);   // Set the window's initial width & height
  glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
  glewInit();
  programID = LoadShaders();
  initBuffers();

  ILuint image;
  ilInit();
  image = LoadImage("tex.png");
  /* OpenGL texture binding of the image loaded by DevIL  */
  glGenTextures(1, &texid);            /* Texture name generation */
  glBindTexture(GL_TEXTURE_2D, texid); /* Binding of texture name */
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    ilGetInteger(IL_IMAGE_BPP),
    ilGetInteger(IL_IMAGE_WIDTH),
    ilGetInteger(IL_IMAGE_HEIGHT),
    0,
    ilGetInteger(IL_IMAGE_FORMAT),
    GL_UNSIGNED_BYTE,
    ilGetData()
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  lastTime = glutGet(GLUT_ELAPSED_TIME);
  printf("startup took %d ms", lastTime);
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
