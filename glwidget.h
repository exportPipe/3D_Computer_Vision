//
// Widget für Interaktion und Kontrolle
//
// (c) Georg Umlauf, 2021
//

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector3D>
#include <QSharedPointer>

#include <vector>

#include "RenderingCamera.h"
#include "PointCloud.h"


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget* parent = nullptr);
    ~GLWidget() Q_DECL_OVERRIDE;

public slots:
    // open a PLY file
    void openFileDialog();
    void radioButton1Clicked();
    void radioButton2Clicked();
    void setPointSize(size_t size);
    void attachCamera(QSharedPointer<RenderingCamera> camera);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

    // navigation
    void keyPressEvent(QKeyEvent   *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent (QMouseEvent *event) Q_DECL_OVERRIDE;


private slots:
  void onCameraChanged(const RenderingCameraState& state);

private:

  // interaction control
  bool X_Pressed=false, Y_Pressed=false;
  QPoint _prevMousePosition;

  // shader control
  void initShaders();
  void createContainers();
  QOpenGLVertexArrayObject _vao;
  QOpenGLBuffer _vertexBuffer;
  QScopedPointer<QOpenGLShaderProgram> _shaders;

  // rendering control
  void setupRenderingCamera();
  QSharedPointer<RenderingCamera> _renderingCamera;
  QMatrix4x4 _projectionMatrix;
  QMatrix4x4 _cameraMatrix;
  QMatrix4x4 _worldMatrix;

  // scene and scene control
  void cleanup       ();
  void drawScene     ();
  void drawPointCloud();
  void drawFrameAxis ();
  float _pointSize;
  PointCloud pointcloud;
  std::vector<std::pair<QVector3D, QColor> > _axesLines;

};
