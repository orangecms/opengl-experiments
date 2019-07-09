/**
 * https://www3.ntu.edu.sg/home/ehchua/programming/opengl/HowTo_OpenGL_C.html
 * GL01Hello.cpp: Test OpenGL C/C++ Setup
 */
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <IL/il.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

#include "frag_shader.h"
#include "vert_shader.h"

GLuint programID;
GLuint texid;

static GLubyte *pixels = NULL;
static GLuint fbo;
static GLuint rbo_color;
static GLuint rbo_depth;
static unsigned int width = 1920;
static unsigned int height = 1080;

/***************************** ffmpeg video capture ***************************/
static AVCodecContext *c = NULL;
static AVFrame *frame;
static AVPacket pkt;
static FILE *file;
static struct SwsContext *sws_context = NULL;
static uint8_t *rgb = NULL;

static void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb) {
  const int in_linesize[1] = {4 * c->width};
  sws_context = sws_getCachedContext(sws_context, c->width, c->height,
                                     AV_PIX_FMT_RGB32, c->width, c->height,
                                     AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);
  sws_scale(sws_context, (const uint8_t *const *)&rgb, in_linesize, 0,
            c->height, frame->data, frame->linesize);
}

void ffmpeg_encoder_start(const char *filename, int codec_id, int fps,
                          int width, int height) {
  AVCodec *codec;
  int ret;
  avcodec_register_all();
  codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }
  c = avcodec_alloc_context3(codec);
  if (!c) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }
  //c->bit_rate = 400000;
  c->width = width;
  c->height = height;
  c->time_base.num = 1;
  c->time_base.den = fps;
  c->gop_size = 10;
  c->max_b_frames = 1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;
  if (codec_id == AV_CODEC_ID_H264)
    av_opt_set(c->priv_data, "preset", "slow", 0);
  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }
  file = fopen(filename, "wb");
  if (!file) {
    fprintf(stderr, "Could not open %s\n", filename);
    exit(1);
  }
  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }
  frame->format = c->pix_fmt;
  frame->width = c->width;
  frame->height = c->height;
  ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                       c->pix_fmt, 32);
  if (ret < 0) {
    fprintf(stderr, "Could not allocate raw picture buffer\n");
    exit(1);
  }
}

void ffmpeg_encoder_finish(void) {
  uint8_t endcode[] = {0, 0, 1, 0xb7};
  fwrite(endcode, 1, sizeof(endcode), file);
  fclose(file);
  avcodec_close(c);
  av_free(c);
  av_freep(&frame->data[0]);
  av_frame_free(&frame);
}

void ffmpeg_encoder_encode_frame(uint8_t *rgb) {
  int ret;
  ffmpeg_encoder_set_frame_yuv_from_rgb(rgb);
  pkt.data = NULL;
  pkt.size = 0;
  ret = avcodec_send_frame(c, frame);
  if (ret < 0) {
    fprintf(stderr, "Error encoding frame\n");
    exit(1);
  }
  av_init_packet(&pkt);
  ret = avcodec_receive_packet(c, &pkt);
  if (ret == 0) {
    fwrite(pkt.data, 1, pkt.size, file);
    av_packet_unref(&pkt);
  }
}

void ffmpeg_encoder_glread_rgb(uint8_t **rgb, GLubyte **pixels,
                               unsigned int width, unsigned int height) {
  size_t i, j, k, cur_gl, cur_rgb, nvals;
  const size_t format_nchannels = 4;
  nvals = format_nchannels * width * height;
  *pixels = realloc(*pixels, nvals * sizeof(GLubyte));
  *rgb = realloc(*rgb, nvals * sizeof(uint8_t));
  /* Get RGBA to align to 32 bits instead of just 24 for RGB. May be faster for
   * FFmpeg. */
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      cur_gl = format_nchannels * (width * (height - i - 1) + j);
      cur_rgb = format_nchannels * (width * i + j);
      for (k = 0; k < format_nchannels; k++)
        (*rgb)[cur_rgb + k] = (*pixels)[cur_gl + k];
    }
  }
}

GLuint LoadShaders() {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  char const * VertexSourcePointer = vert_shader_txt;
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

  // Compile Fragment Shader
  char const * FragmentSourcePointer = frag_shader_txt;
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

  // Link the program
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

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

int initTime = 0;
int frameCount = 0;
int framesToRender = 50;

/**
 * Handler for window-repaint event. Call back when the window first appears and
 * whenever the window needs to be re-painted.
 */
void display() {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glViewport(0, 0, width, height);

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
  frame->pts = frameCount;
  ffmpeg_encoder_glread_rgb(&rgb, &pixels, width, height);
  ffmpeg_encoder_encode_frame(rgb);
  frameCount++;
  if (frameCount > framesToRender){
    exit(EXIT_SUCCESS);
  }
}

void initBuffers() {
  /*  Framebuffer */
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  /* Color renderbuffer. */
  glGenRenderbuffers(1, &rbo_color);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_color);
  /* Storage must be one of: */
  /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8.
   */
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, width, height);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, rbo_color);

  /* Depth renderbuffer. */
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo_depth);

  glReadBuffer(GL_COLOR_ATTACHMENT0);

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

static void deinit(void) {
  int renderTime = glutGet(GLUT_ELAPSED_TIME) - initTime;
  printf(
    "rendered in %fs, %fFPS\n",
    (double) renderTime / 1000.0,
    1000.0 * frameCount / renderTime
  );
  free(pixels);
  ffmpeg_encoder_finish();
  free(rgb);
  glDeleteFramebuffers(1, &fbo);
  glDeleteRenderbuffers(1, &rbo_color);
  glDeleteRenderbuffers(1, &rbo_depth);
  int exportTime = glutGet(GLUT_ELAPSED_TIME) - renderTime;
  printf("export finished in %fs\n", (double) exportTime / 1000.0);
}

/**
 * Main function: GLUT runs as a console application starting at main()
 */
int main(int argc, char** argv) {
  GLint glut_display;
  glutInit(&argc, argv);                 // Initialize GLUT
  glutInitWindowSize(width, height);   // Set the window's initial width & height
  glut_display = GLUT_SINGLE;
  //glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
  glutInitDisplayMode(glut_display | GLUT_RGBA | GLUT_DEPTH);
  glutCreateWindow("OpenGL Speed Test"); // Create a window with the given title
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

  initTime = glutGet(GLUT_ELAPSED_TIME);
  printf("startup took %d ms\n", initTime);
  ffmpeg_encoder_start("tmp.mp4", AV_CODEC_ID_H264, 25, width, height);
  glutDisplayFunc(display); // Register display callback handler for window re-paint
  glutIdleFunc(display);    // Repaint continuously
  atexit(deinit);
  glutMainLoop();           // Enter the infinitely event-processing loop

  /* Delete used resources and quit */
  ilDeleteImages(
      1, &image); /* Because we have already copied image data into texture data
                     we can release memory used by image. */
  glDeleteTextures(1, &texid);

  return 0;
}
