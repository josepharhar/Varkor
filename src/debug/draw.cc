#include <glad/glad.h>

#include "../ds/vector.hh"
#include "../math/matrix.hh"
#include "../math/vector.hh"
#include "../shader.h"

#include "draw.h"

namespace Debug {
namespace Draw {

struct Renderable
{
  unsigned int mVao;
  unsigned int mVbo;
  unsigned int mCount;
  Vec3 mColor;
};

DS::Vector<Renderable> nRenderables;
Shader nShader;

void Init()
{
  nShader.Init("shader/debug.vs", "shader/debug.fs");
}

void Line(const Vec3& a, const Vec3& b, const Vec3& color)
{
  Vec3 vertexBuffer[2] = {a, b};
  unsigned int vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer, GL_STREAM_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  nRenderables.Push({vao, vbo, 2, color});
}

void Render(const Mat4& view, const Mat4& projection)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  nShader.SetMat4("view", view.CData(), true);
  nShader.SetMat4("proj", projection.CData(), false);
  for (int i = 0; i < nRenderables.Size(); ++i)
  {
    const Renderable& renderable = nRenderables[i];
    nShader.SetVec3("color", renderable.mColor.mValue);
    glBindVertexArray(renderable.mVao);
    glDrawArrays(GL_LINES, 0, renderable.mCount);
  }

  for (int i = 0; i < nRenderables.Size(); ++i)
  {
    const Renderable& renderable = nRenderables[i];
    glDeleteVertexArrays(1, &renderable.mVao);
    glDeleteBuffers(1, &renderable.mVbo);
  }
  nRenderables.Clear();
}

} // namespace Draw
} // namespace Debug
