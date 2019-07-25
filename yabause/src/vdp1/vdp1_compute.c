#include "vdp1_compute.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vdp1.h"

//#define VDP1CDEBUG
#ifdef VDP1CDEBUG
#define VDP1CPRINT printf
#else
#define VDP1CPRINT
#endif

#define NB_COARSE_RAST (NB_COARSE_RAST_X * NB_COARSE_RAST_Y)

static int local_size_x = LOCAL_SIZE_X;
static int local_size_y = LOCAL_SIZE_Y;

static int tex_width;
static int tex_height;
static float tex_ratiow;
static float tex_ratioh;
static int struct_size;

static int work_groups_x;
static int work_groups_y;

static vdp1cmd_struct* cmdVdp1;
static int* nbCmd;

static int* clear;

static GLuint compute_tex[2] = {0};
static GLuint ssbo_cmd_ = 0;
static GLuint ssbo_vdp1ram_ = 0;
static GLuint ssbo_nbcmd_ = 0;
static GLuint prg_vdp1[NB_PRG] = {0};

static const GLchar * a_prg_vdp1[NB_PRG][3] = {
  //VDP1_0_PAL
	{
		vdp1_start_f,
		vdp1_0_Pal_f,
		vdp1_end_f
	},
	//CLEAR
	{
		vdp1_clear_f,
		NULL,
		NULL
  },
	//TEST_PRG
	{
		vdp1_start_f,
		vdp1_test_f,
		vdp1_end_f
	}
};

static int getProgramId() {
  return TEST_PRG;
}

int ErrorHandle(const char* name)
{
#ifdef VDP1CDEBUG
  GLenum   error_code = glGetError();
  if (error_code == GL_NO_ERROR) {
    return  1;
  }
  do {
    const char* msg = "";
    switch (error_code) {
    case GL_INVALID_ENUM:      msg = "INVALID_ENUM";      break;
    case GL_INVALID_VALUE:     msg = "INVALID_VALUE";     break;
    case GL_INVALID_OPERATION: msg = "INVALID_OPERATION"; break;
    case GL_OUT_OF_MEMORY:     msg = "OUT_OF_MEMORY";     break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:  msg = "INVALID_FRAMEBUFFER_OPERATION"; break;
    default:  msg = "Unknown"; break;
    }
    VDP1CPRINT("GLErrorLayer:ERROR:%04x'%s' %s\n", error_code, msg, name);
    error_code = glGetError();
  } while (error_code != GL_NO_ERROR);
  abort();
  return 0;
#else
  return 1;
#endif
}

static GLuint createProgram(int count, const GLchar** prg_strs) {
  GLint status;
	int exactCount = 0;
  GLuint result = glCreateShader(GL_COMPUTE_SHADER);

  for (int id = 0; id < count; id++) {
		if (prg_strs[id] != NULL) exactCount++;
		else break;
	}
  glShaderSource(result, exactCount, prg_strs, NULL);
  glCompileShader(result);
  glGetShaderiv(result, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE) {
    GLint length;
    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = malloc(sizeof(GLchar) *length);
    glGetShaderInfoLog(result, length, NULL, info);
    VDP1CPRINT("[COMPILE] %s\n", info);
    free(info);
    abort();
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, result);
  glLinkProgram(program);
  glDetachShader(program, result);
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = malloc(sizeof(GLchar) *length);
    glGetProgramInfoLog(program, length, NULL, info);
    VDP1CPRINT("[LINK] %s\n", info);
    free(info);
    abort();
  }
  return program;
}

static int generateComputeBuffer(int w, int h) {
	printf("Generate %d %d\n", w, h);
  if (compute_tex[0] != 0) {
    glDeleteTextures(2,&compute_tex[0]);
  }
	if (ssbo_vdp1ram_ != 0) {
    glDeleteBuffers(1, &ssbo_vdp1ram_);
	}

	glGenBuffers(1, &ssbo_vdp1ram_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vdp1ram_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0x80000, NULL, GL_DYNAMIC_DRAW);

  if (ssbo_cmd_ != 0) {
    glDeleteBuffers(1, &ssbo_cmd_);
  }
  glGenBuffers(1, &ssbo_cmd_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cmd_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, struct_size*2000*NB_COARSE_RAST, NULL, GL_DYNAMIC_DRAW);

  if (ssbo_nbcmd_ != 0) {
    glDeleteBuffers(1, &ssbo_nbcmd_);
  }
  glGenBuffers(1, &ssbo_nbcmd_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_nbcmd_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, NB_COARSE_RAST * sizeof(int),NULL,GL_DYNAMIC_DRAW);

  glGenTextures(2, &compute_tex[0]);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, compute_tex[0]);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, compute_tex[1]);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (clear != NULL) free(clear);
	clear = (int*)malloc(w*h * sizeof(int));
	memset(clear, 0, w*h * sizeof(int));


  return 0;
}

int vdp1_add(vdp1cmd_struct* cmd) {
	int Ax = cmd->CMDXA;
	int Ay = cmd->CMDYA;
	int Bx = cmd->CMDXB;
	int By = cmd->CMDYB;
	int Cx = cmd->CMDXC;
	int Cy = cmd->CMDYC;
	int Dx = cmd->CMDXD;
	int Dy = cmd->CMDYD;

  int minx = (Ax < Bx)?Ax:Bx;
  int miny = (Ay < By)?Ay:By;
  int maxx = (Ax > Bx)?Ax:Bx;
  int maxy = (Ay > By)?Ay:By;

  minx = (minx < Cx)?minx:Cx;
  minx = (minx < Dx)?minx:Dx;
  miny = (miny < Cy)?miny:Cy;
  miny = (miny < Dy)?miny:Dy;
  maxx = (maxx > Cx)?maxx:Cx;
  maxx = (maxx > Dx)?maxx:Dx;
  maxy = (maxy > Cy)?maxy:Cy;
  maxy = (maxy > Dy)?maxy:Dy;

  int intersectX = -1;
  int intersectY = -1;
  for (int i = 0; i<NB_COARSE_RAST_X; i++) {
    int blkx = i * (tex_width/NB_COARSE_RAST_X);
    for (int j = 0; j<NB_COARSE_RAST_Y; j++) {
      int blky = j*(tex_height/NB_COARSE_RAST_Y);
      if (!(blkx > maxx
        || (blkx + (tex_width/NB_COARSE_RAST_X)) < minx
        || (blky + (tex_height/NB_COARSE_RAST_Y)) < miny
        || blky > maxy)) {
					memcpy(&cmdVdp1[(i+j*NB_COARSE_RAST_X)*2000 + nbCmd[i+j*NB_COARSE_RAST_X]], cmd, sizeof(vdp1cmd_struct));
          nbCmd[i+j*NB_COARSE_RAST_X]++;
      }
    }
  }
}

void vdp1_clear() {
	int progId = CLEAR;
	//printf("USe Prog %d\n", progId);
	if (prg_vdp1[progId] == 0)
    prg_vdp1[progId] = createProgram(sizeof(a_prg_vdp1[progId]) / sizeof(char*), (const GLchar**)a_prg_vdp1[progId]);
  glUseProgram(prg_vdp1[progId]);

	glBindImageTexture(0, compute_tex[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(work_groups_x, work_groups_y, 1); //might be better to launch only the right number of workgroup
}

void vdp1_compute_init(int width, int height, float ratiow, float ratioh)
{
  int am = sizeof(vdp1cmd_struct) % 16;
  tex_width = width;
  tex_height = height;
	tex_ratiow = 1.0;//ratiow;
	tex_ratioh = 1.0;//ratioh;
  struct_size = sizeof(vdp1cmd_struct);
  if (am != 0) {
    struct_size += 16 - am;
  }

  work_groups_x = (tex_width*tex_ratiow) / local_size_x;
  work_groups_y = (tex_height*tex_ratioh) / local_size_y;
  generateComputeBuffer(width*tex_ratiow, height*tex_ratioh);
	if (nbCmd == NULL)
  	nbCmd = (int*)malloc(NB_COARSE_RAST *sizeof(int));
  if (cmdVdp1 == NULL)
		cmdVdp1 = (vdp1cmd_struct*)malloc(NB_COARSE_RAST*2000*sizeof(vdp1cmd_struct));
  memset(nbCmd, 0, NB_COARSE_RAST*sizeof(int));
	memset(cmdVdp1, 0, NB_COARSE_RAST*2000*sizeof(vdp1cmd_struct*));
	return;
}

int* vdp1_compute(Vdp2 *varVdp2Regs) {
  GLuint error;
	int progId = getProgramId();
	int needRender = 0;
	//printf("USe Prog %d\n", progId);
	if (prg_vdp1[progId] == 0)
    prg_vdp1[progId] = createProgram(sizeof(a_prg_vdp1[progId]) / sizeof(char*), (const GLchar**)a_prg_vdp1[progId]);
  glUseProgram(prg_vdp1[progId]);

	VDP1CPRINT("Draw VDP1\n");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cmd_);
  for (int i = 0; i < NB_COARSE_RAST; i++) {
    if (nbCmd[i] != 0) {
    	glBufferSubData(GL_SHADER_STORAGE_BUFFER, struct_size*i*2000, nbCmd[i]*sizeof(vdp1cmd_struct), (void*)&cmdVdp1[2000*i]);
			needRender = 1;
		}
  }

  if (needRender == 0) return &compute_tex[0];

	_Ygl->vdp1On[_Ygl->drawframe] = 1;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_nbcmd_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int)*NB_COARSE_RAST, (void*)nbCmd);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vdp1ram_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 0x80000, (void*)Vdp1Ram);

	glBindImageTexture(0, compute_tex[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_vdp1ram_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_nbcmd_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_cmd_);
	glUniform2f(4, tex_ratiow, tex_ratioh);

  glDispatchCompute(work_groups_x, work_groups_y, 1); //might be better to launch only the right number of workgroup
	// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  ErrorHandle("glDispatchCompute");
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BI
  memset(nbCmd, 0, NB_COARSE_RAST*sizeof(int));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  return &compute_tex[0];
}
