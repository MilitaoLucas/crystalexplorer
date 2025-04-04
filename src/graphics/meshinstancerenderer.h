#pragma once
#include "mesh.h"
#include "meshinstancevertex.h"
#include "colormap.h"
#include "renderer.h"
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <vector>

class MeshInstanceRenderer : public IndexedRenderer, protected QOpenGLExtraFunctions {
public:
  struct IndexTuple {
    GLuint i, j, k;
  };

  explicit MeshInstanceRenderer(Mesh *mesh = nullptr);


  void addInstance(const MeshInstanceVertex &);
  void addInstances(const std::vector<MeshInstanceVertex> &instances);

  virtual void draw() override;
  virtual void beginUpdates() override;
  virtual void endUpdates() override;
  virtual void clear() override;

  [[nodiscard]] inline const auto &instances() const { return m_instances; }
  [[nodiscard]] inline auto &instances() { return m_instances; }
  void setMesh(Mesh *);

  inline const auto &availableProperties() const { return m_availableProperties; }
  bool hasTransparentObjects() const;

private:
  void updateBuffers();
  QOpenGLBuffer m_vertex;
  QOpenGLBuffer m_instance;

  std::vector<MeshInstanceVertex> m_instances;

  QOpenGLBuffer m_vertexPropertyBuffer;
  QOpenGLTexture *m_vertexPropertyTexture{nullptr}; 

  QStringList m_availableProperties;
  int m_numIndices{0};
  int m_numVertices{0};

protected:
  bool m_impostor = false;
};
