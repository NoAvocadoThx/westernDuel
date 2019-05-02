#include "TexturedCube.h"
#include <GL/glew.h>
#include <iostream>
#include <vector>

unsigned char* loadPPM(const char* filename, int& width, int& height)
{
  const int BUFSIZE = 128;
  FILE* fp;
  unsigned int read;
  unsigned char* rawData;
  char buf[3][BUFSIZE];
  char* retval_fgets;
  size_t retval_sscanf;

  if ((fp = fopen(filename, "rb")) == NULL)
  {
    std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
    width = 0;
    height = 0;
    return NULL;
  }

  // Read magic number:
  retval_fgets = fgets(buf[0], BUFSIZE, fp);

  // Read width and height:
  do
  {
    retval_fgets = fgets(buf[0], BUFSIZE, fp);
  }
  while (buf[0][0] == '#');
  retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
  width = atoi(buf[1]);
  height = atoi(buf[2]);

  // Read maxval:
  do
  {
    retval_fgets = fgets(buf[0], BUFSIZE, fp);
  }
  while (buf[0][0] == '#');

  // Read image data:
  rawData = new unsigned char[width * height * 3];
  read = fread(rawData, width * height * 3, 1, fp);
  fclose(fp);
  if (read != 1)
  {
    std::cerr << "error parsing ppm file, incomplete data" << std::endl;
    delete[] rawData;
    width = 0;
    height = 0;
    return NULL;
  }

  return rawData;
}

unsigned loadCubemap(const std::string directory, std::vector<std::string>& faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    std::string path = directory + faces[i];
    unsigned char* data = loadPPM(path.c_str(), width, height);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
      );
    }
    else
    {
      std::cout << "Cubemap texture failed to load at path: " << faces[i].c_str() << std::endl;
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

std::vector<std::string> faces
{
  "left.ppm",
  "right.ppm",
  "up.ppm",
  "down.ppm",
  "back.ppm",
  "front.ppm"
};

std::vector<std::string> faces_r
{
  "left_r.ppm",
  "right_r.ppm",
  "up_r.ppm",
  "down_r.ppm",
  "back_r.ppm",
  "front_r.ppm"
};

TexturedCube::TexturedCube(const std::string dir) : Cube()
{
	if (dir == "skybox_r") {

		cubeMap_r = loadCubemap("./" + dir + "/", faces_r);
	}
	else if(dir=="skybox_l"){
	    cubeMap_l = loadCubemap("./" + dir + "/", faces);
    }
	else if (dir == "cube") {
		cubeMap_l= loadCubemap("./" + dir + "/", faces);
		texID = cubeMap_l;
	}
}

TexturedCube::~TexturedCube()
{
  glDeleteTextures(1, &cubeMap_l);
  glDeleteTextures(1, &cubeMap_r);
  glDeleteTextures(1, &texID);
}

void TexturedCube::draw(unsigned shader, const glm::mat4& p, const glm::mat4& v)
{
  glUseProgram(shader);
  // ... set view and projection matrix
  uProjection = glGetUniformLocation(shader, "projection");
  uView = glGetUniformLocation(shader, "view");

  glm::mat4 modelview = v * toWorld;

  // Now send these values to the shader program
  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &p[0][0]);
  glUniformMatrix4fv(uView, 1, GL_FALSE, &modelview[0][0]);

  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP,texID);
  glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}


