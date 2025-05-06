#include "DeferredRenderer.h"
#include "ShaderProgram.h"
#include <string>
#include <stdio.h>
#include <iostream>

int DeferredRenderer::Initialize()
{
  pGeometry = CreateProgram("../shaders/Geometry.vert", "../shaders/Geometry.frag");
  pLighting = CreateProgram("../shaders/Lighting.vert", "../shaders/Lighting.frag");
  pSolid    = CreateProgram("../shaders/Solid.vert"   , "../shaders/Solid.frag");

  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  glGenTextures(3, textures);
  for (int i = 0; i < 3; ++i)
  {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    if (i != 3)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 768, 0, GL_RGB, GL_FLOAT, NULL);
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 768, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
  }

  GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);

  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);


  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "Frame buffer not complete.\n";
    return -1;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(pLighting);
  glUniform1i(glGetUniformLocation(pLighting, "gPositionMap"), 0);
  glUniform1i(glGetUniformLocation(pLighting, "gNormalMap"), 1);
  glUniform1i(glGetUniformLocation(pLighting, "gColorMap"), 2);



  //full screen quad setup
  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);

  glGenBuffers(2, quadVBO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 12, quadVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 8, quadUVs, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 0;
}

void DeferredRenderer::DrawStart(glm::mat4 cameraMatrix)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(pGeometry);

  camera = cameraMatrix;

  glUniformMatrix4fv(glGetUniformLocation(pGeometry, "cameraMat"), 1, GL_FALSE, &camera[0][0]);
}

void DeferredRenderer::DrawGeometry(Object model, glm::mat4 modelMatrix)
{
  transform = modelMatrix;
  glUniformMatrix4fv(glGetUniformLocation(pGeometry, "objectMat"), 1, GL_FALSE, &transform[0][0]);

  glBindVertexArray(model.vertexArrayID);
  glDrawElements(GL_TRIANGLES, model.faces.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void DeferredRenderer::DrawLighting(LightInfo lights, glm::vec4 cameraPos, int mode)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(pLighting);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[gPosition]);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures[gNormal]);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, textures[gColor]);

  //pass lights as a uniform

  for (int i = 0; i < lights.lightPositions.size(); ++i)
  {
    glUniform4fv(glGetUniformLocation(pLighting, ("lightPositions[" + std::to_string(i) + "]").c_str()), 1, &(lights.lightPositions[i][0]));
    glUniform4fv(glGetUniformLocation(pLighting, ("lightColors[" + std::to_string(i) + "]").c_str()), 1, &(lights.lightColors[i][0]));
  }
  glUniform3f(glGetUniformLocation(pLighting, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

  glUniform1i(glGetUniformLocation(pLighting, "mode"), mode);

  //render full screen quad

  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

}

void DeferredRenderer::CopyDepth()
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, 1024, 768, 0, 0, 1024, 768, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::DrawForward(Object model, glm::mat4 modelMatrix, glm::vec4 modelColor)
{
  glUseProgram(pSolid);
  glUniformMatrix4fv(glGetUniformLocation(pSolid, "cameraMat"), 1, GL_FALSE, &camera[0][0]);
  transform = modelMatrix;
  glUniformMatrix4fv(glGetUniformLocation(pSolid, "objectMat"), 1, GL_FALSE, &transform[0][0]);
  color = modelColor;
  glUniform4fv(glGetUniformLocation(pSolid, "modelColor"), 1, &color[0]);

  glBindVertexArray(model.vertexArrayID);
  glDrawElements(GL_TRIANGLES, model.faces.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void DeferredRenderer::Shutdown()
{
  glDeleteProgram(pGeometry);
  glDeleteProgram(pLighting);
  glDeleteProgram(pSolid);
}
