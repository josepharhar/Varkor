#include <glad/glad.h>

#include <fstream>
#include <sstream>

#include "error.h"

#include "shader.h"

const int Shader::smInvalidLocation = -1;

Shader::Shader()
{
  mProgram = 0;
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
  Init(vertexFile, fragmentFile);
}

void Shader::Init(const char* vertexFile, const char* fragmentFile)
{
  unsigned int vertId, fragId;
  bool compiled = Compile(vertexFile, GL_VERTEX_SHADER, &vertId);
  compiled &= Compile(fragmentFile, GL_FRAGMENT_SHADER, &fragId);
  if (!compiled)
  {
    mProgram = 0;
    return;
  }

  mProgram = glCreateProgram();
  glAttachShader(mProgram, vertId);
  glAttachShader(mProgram, fragId);
  glLinkProgram(mProgram);
  int linked;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    const int errorLogLen = 512;
    char errorLog[errorLogLen];
    glGetProgramInfoLog(mProgram, errorLogLen, NULL, errorLog);
    std::stringstream reason;
    reason << " shader files " << vertexFile << " and " << fragmentFile
           << " failed to link." << std::endl
           << errorLog;
    LogError(reason.str().c_str());
    mProgram = 0;
    return;
  }
  glDeleteShader(vertId);
  glDeleteShader(fragId);
}

void Shader::Use() const
{
  glUseProgram(mProgram);
}

unsigned int Shader::Id() const
{
  return mProgram;
}

int Shader::UniformLocation(const char* name) const
{
  int loc = glGetUniformLocation(mProgram, name);
  bool valid = loc != smInvalidLocation;
  if (!valid)
  {
    std::stringstream reason;
    reason << name << " uniform not found";
    LogError(reason.str().c_str());
  }
  return loc;
}

void Shader::SetVec3(const char* name, const float* data) const
{
  int loc = UniformLocation(name);
  Use();
  glUniform3fv(loc, 1, data);
}

void Shader::SetMat4(const char* name, const float* data, bool transpose) const
{
  int loc = UniformLocation(name);
  Use();
  glUniformMatrix4fv(loc, 1, transpose, data);
}

bool Shader::Compile(
  const char* shaderFile, int shaderType, unsigned int* shaderId)
{
  std::ifstream file;
  file.open(shaderFile);
  if (!file.is_open())
  {
    std::stringstream reason;
    reason << "shader file " << shaderFile << " does not exist.";
    LogError(reason.str().c_str());
    return false;
  }

  std::stringstream fileContentStream;
  fileContentStream << file.rdbuf();
  std::string fileContentStr = fileContentStream.str();
  const char* fileContent = fileContentStr.c_str();

  *shaderId = glCreateShader(shaderType);
  glShaderSource(*shaderId, 1, &fileContent, NULL);
  glCompileShader(*shaderId);
  int success;
  glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &success);
  if (success)
  {
    return true;
  }

  const int logLen = 512;
  char log[logLen];
  glGetShaderInfoLog(*shaderId, logLen, NULL, log);
  std::stringstream error;
  if (shaderType == GL_VERTEX_SHADER)
  {
    error << "vertex";
  } else if (shaderType == GL_FRAGMENT_SHADER)
  {
    error << "fragment";
  }
  error << " shader file " << shaderFile << " failed to compile" << std::endl
        << log;
  LogError(error.str().c_str());
  return false;
}