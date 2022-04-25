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

  // own by Paul
  void initCuboid(std::vector<QVector3D> &cuboid, QVector4D translation, float scale, float angle_x, float angly_y, float angle_z);
  void drawObject(std::vector<QVector3D>, QColor color);
  void initPerspectiveCamera(std::vector<QVector3D> &axes_lines, QVector4D translation, QVector3D rotation);
  QVector4D getPrinciplePoint(float focal_length, QVector4D position_camera, QVector3D rotation_camera);

  void initImagePlane(std::vector<QVector3D> &image_plane, std::vector<QVector3D> &axes,
                      QVector4D position, float scale, float focal_length,
                      QVector3D rotation, QVector4D principle_point);

  void initProjection(std::vector<QVector3D> object, std::vector<QVector3D> &projection, float focal_length,
                      QVector4D center, QVector4D principle_point, QVector3D rotation);

  QVector3D centralProjection(float focal_length, QVector3D vertex, QVector3D center, QVector3D rotation);

  void initProjectionLines(std::vector<QVector3D> object, std::vector<QVector3D> &projection_lines, QVector4D center);
};
