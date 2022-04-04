#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// (c) Nico Brügel, 2021

#include "glwidget.h"
#include <QtGui>

//#if defined(__APPLE__)
// we're on macOS and according to their documentation Apple hates developers
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
/**
#elseif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
// windows, even if it's case insensitive QT-Create might generate a warning
#include <gl/GL.h>
#include <gl/GLU.h>
#else
// hopefully on linux
// If can't be found, ensure that the following is installed:
// libglu1-mesa-dev and/or mesa-common-dev
#include <GL/gl.h>
#include <GL/glu.h>
*/
//#endif

#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include <cmath>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <utility>

#include "mainwindow.h"
#include "GLConvenience.h"
#include "QtConvenience.h"

using namespace std;

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent),
    _pointSize(1)
{
    setMouseTracking(true);

    // axes cross
    _axesLines.push_back(make_pair(QVector3D(0.0, 0.0, 0.0), QColor(1.0, 0.0, 0.0)));
    _axesLines.push_back(make_pair(QVector3D(1.0, 0.0, 0.0), QColor(1.0, 0.0, 0.0)));
    _axesLines.push_back(make_pair(QVector3D(0.0, 0.0, 0.0), QColor(0.0, 1.0, 0.0)));
    _axesLines.push_back(make_pair(QVector3D(0.0, 1.0, 0.0), QColor(0.0, 1.0, 0.0)));
    _axesLines.push_back(make_pair(QVector3D(0.0, 0.0, 0.0), QColor(0.0, 0.0, 1.0)));
    _axesLines.push_back(make_pair(QVector3D(0.0, 0.0, 1.0), QColor(0.0, 0.0, 1.0)));
}

GLWidget::~GLWidget()
{
    this->cleanup();
}

void GLWidget::cleanup()
{
  makeCurrent();
 // _vertexBuffer.destroy();
  _shaders.reset();
  doneCurrent();
}

void GLWidget::initializeGL()
{
  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

  initializeOpenGLFunctions();
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.4f,0.4f,0.4f,1);     // screen background color

  // the world is still for now
  _worldMatrix.setToIdentity();

  // create shaders and map attributes
  initShaders();

  // create array container and load points into buffer
  createContainers();
}

void GLWidget::paintGL()
{
    // ensure GL flags
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); //required for gl_PointSize

    // create shaders and map attributes
    initShaders();

    // create array container and load points into buffer
   // createContainers();
    // create array container and load points into buffer
    const QVector<float>& pointsData =pointcloud.getData();
    if(!_vao.isCreated()) _vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&_vao);
    if(!_vertexBuffer.isCreated()) _vertexBuffer.create();
    _vertexBuffer.bind();
    _vertexBuffer.allocate(pointsData.constData(), pointsData.size() * sizeof(GLfloat));
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat) + sizeof(GLfloat), nullptr);
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat) + sizeof(GLfloat), reinterpret_cast<void *>(3*sizeof(GLfloat)));
    _vertexBuffer.release();

    // set camera
    setupRenderingCamera();

    // draw points cloud
    drawPointCloud();
    drawFrameAxis();

    // Assignement 1, Part 1
    // Draw here your objects as in drawFrameAxis();
    // LEFT
    glBegin(GL_POLYGON);
    QMatrix4x4 myMatrix = _cameraMatrix * _worldMatrix;
    QVector3D v1 = QVector3D(0.005, 0.005, 0.015);
    QVector3D v2 = QVector3D(0.005, 0.015, 0.015);
    QVector3D v3 = QVector3D(0.005, 0.015, 0.005);
    QVector3D v4 = QVector3D(0.005, 0.005, 0.005);
    v1 = _projectionMatrix * myMatrix ^ v1;
    v2 = _projectionMatrix * myMatrix ^ v2;
    v3 = _projectionMatrix * myMatrix ^ v3;
    v4 = _projectionMatrix * myMatrix ^ v4;
    glColor3f(0.0, 1.0, 1.0);
    glVertex3f(v1);
    glVertex3f(v2);
    glVertex3f(v3);
    glVertex3f(v4);
    glEnd();

    // TOP
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.0f, 0.0f);
    v1 = QVector3D(0.015, 0.015, 0.015);
    v2 = QVector3D(0.015, 0.015, 0.005);
    v3 = QVector3D(0.005, 0.015, 0.005);
    v4 = QVector3D(0.005, 0.015, 0.015);
    v1 = _projectionMatrix * myMatrix ^ v1;
    v2 = _projectionMatrix * myMatrix ^ v2;
    v3 = _projectionMatrix * myMatrix ^ v3;
    v4 = _projectionMatrix * myMatrix ^ v4;
    glVertex3f(v1);
    glVertex3f(v2);
    glVertex3f(v3);
    glVertex3f(v4);
    glEnd();

    // BACK
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.0f, 1.0f);
    v1 = QVector3D(0.015, 0.005, 0.015);
    v2 = QVector3D(0.015, 0.015, 0.015);
    v3 = QVector3D(0.005, 0.015, 0.015);
    v4 = QVector3D(0.005, 0.005, 0.015);
    v1 = _projectionMatrix * myMatrix ^ v1;
    v2 = _projectionMatrix * myMatrix ^ v2;
    v3 = _projectionMatrix * myMatrix ^ v3;
    v4 = _projectionMatrix * myMatrix ^ v4;
    glVertex3f(v1);
    glVertex3f(v2);
    glVertex3f(v3);
    glVertex3f(v4);
    glEnd();

    // RIGHT
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    v1 = QVector3D(0.015, 0.005, 0.005);
    v2 = QVector3D(0.015, 0.015, 0.005);
    v3 = QVector3D(0.015, 0.015, 0.015);
    v4 = QVector3D(0.015, 0.005, 0.015);
    v1 = _projectionMatrix * myMatrix ^ v1;
    v2 = _projectionMatrix * myMatrix ^ v2;
    v3 = _projectionMatrix * myMatrix ^ v3;
    v4 = _projectionMatrix * myMatrix ^ v4;
    glVertex3f(v1);
    glVertex3f(v2);
    glVertex3f(v3);
    glVertex3f(v4);
    glEnd();

    // BOTTOM
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 0.0f);
    v1 = QVector3D(0.015, 0.005, 0.005);
    v2 = QVector3D(0.015, 0.005, 0.015);
    v3 = QVector3D(0.005, 0.005, 0.015);
    v4 = QVector3D(0.005, 0.005, 0.005);
    v1 = _projectionMatrix * myMatrix ^ v1;
    v2 = _projectionMatrix * myMatrix ^ v2;
    v3 = _projectionMatrix * myMatrix ^ v3;
    v4 = _projectionMatrix * myMatrix ^ v4;
    glVertex3f(v1);
    glVertex3f(v2);
    glVertex3f(v3);
    glVertex3f(v4);
    glEnd();

    // Assignement 1, Part 2
    // Draw here your perspective camera model

    // Assignement 1, Part 3
    // Draw here the perspective projection
}

void GLWidget::drawFrameAxis()
{
  glBegin(GL_LINES);
  QMatrix4x4 mvMatrix = _cameraMatrix * _worldMatrix;
  mvMatrix.scale(0.05f); // make it small
  for (auto vertex : _axesLines) {
    const auto translated = _projectionMatrix * mvMatrix ^ vertex.first;
    glColor3f(vertex.second);
    glVertex3f(translated);
  }
  glEnd();
}


void GLWidget::resizeGL(int w, int h)
{
  _projectionMatrix.setToIdentity();
  _projectionMatrix.perspective(70.0f, GLfloat(w) / GLfloat(h), 0.01f, 100.0f);
}

void GLWidget::wheelEvent(QWheelEvent* event)
{
  if (event->angleDelta().y() > 0) {
    _renderingCamera->forward();
  } else {
    _renderingCamera->backward();
  }
}

void GLWidget::keyPressEvent(QKeyEvent * event)
{
    switch ( event->key() )
    {
      case Qt::Key_Escape:
        QApplication::instance()->quit();
        break;

      case Qt::Key_Left:
      case Qt::Key_A:
        _renderingCamera->left();
        break;

      case Qt::Key_Right:
      case Qt::Key_D:
        _renderingCamera->right();
        break;

      case Qt::Key_Up:
      case Qt::Key_W:
        _renderingCamera->forward();
        break;

      case Qt::Key_Down:
      case Qt::Key_S:
        _renderingCamera->backward();
        break;

      case Qt::Key_Space:
      case Qt::Key_Q:
        _renderingCamera->up();
        break;

      case Qt::Key_C:
      case Qt::Key_Z:
        _renderingCamera->down();
        break;

      default:
        QWidget::keyPressEvent(event);
    }
	update();
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos() - _prevMousePosition;

    _prevMousePosition = event->pos();

    if (event->buttons() & Qt::LeftButton)
    {
        _renderingCamera->rotate(X_Pressed?0:pos.y(), Y_Pressed?0:pos.x(), 0);
    }
    else if ( event->buttons() & Qt::RightButton)
    {
        if (pos.x() < 0) _renderingCamera->right();
        if (pos.x() > 0) _renderingCamera->left();
        if (pos.y() < 0) _renderingCamera->down();
        if (pos.y() > 0) _renderingCamera->up();
    }
}

void GLWidget::attachCamera(QSharedPointer<RenderingCamera> camera)
{
  if (_renderingCamera)
  {
    disconnect(_renderingCamera.data(), &RenderingCamera::changed, this, &GLWidget::onCameraChanged);
  }
  _renderingCamera = camera;
  connect(camera.data(), &RenderingCamera::changed, this, &GLWidget::onCameraChanged);
}


void GLWidget::onCameraChanged(const RenderingCameraState&)
{
  update();
}

void GLWidget::setPointSize(size_t size)
{
  assert(size > 0);
  _pointSize = static_cast<float>(size);
  update();
}

void GLWidget::openFileDialog()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open PLY file"), "../data", tr("PLY Files (*.ply)"));
     if (!filePath.isEmpty())
     {
         std::cout << filePath.toStdString() << std::endl;
         pointcloud.loadPLY(filePath);
         update();
     }
}

void GLWidget::radioButton1Clicked()
{
    // TODO: toggle to
    QMessageBox::warning(this, "Feature" ,"upsi hier fehlt noch was");
    update();
}

void GLWidget::radioButton2Clicked()
{
    // TODO: toggle to
    QMessageBox::warning(this, "Feature" ,"upsi hier fehlt noch was");
    update();
}

void GLWidget::initShaders()
{
    _shaders.reset(new QOpenGLShaderProgram());
    auto vsLoaded = _shaders->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
    auto fsLoaded = _shaders->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
    assert(vsLoaded && fsLoaded);
    // vector attributes
    _shaders->bindAttributeLocation("vertex", 0);
    _shaders->bindAttributeLocation("pointRowIndex", 1);
    // constants
    _shaders->bind();
    _shaders->setUniformValue("lightPos", QVector3D(0, 0, 50));
    _shaders->setUniformValue("pointsCount", static_cast<GLfloat>(pointcloud.getCount()));
    _shaders->link();
    _shaders->release();

   }

void GLWidget::createContainers()
{
    // create array container and load points into buffer
    const QVector<float>& pointsData =pointcloud.getData();
    if(!_vao.isCreated()) _vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&_vao);
    if(!_vertexBuffer.isCreated()) _vertexBuffer.create();
    _vertexBuffer.bind();
    _vertexBuffer.allocate(pointsData.constData(), pointsData.size() * sizeof(GLfloat));
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat) + sizeof(GLfloat), nullptr);
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat) + sizeof(GLfloat), reinterpret_cast<void *>(3*sizeof(GLfloat)));
    _vertexBuffer.release();
}

void GLWidget::setupRenderingCamera()
{
    const RenderingCameraState cameraState = _renderingCamera->state();
    // position and angles
    _cameraMatrix.setToIdentity();
    _cameraMatrix.translate(cameraState.position.x(), cameraState.position.y(), cameraState.position.z());
    _cameraMatrix.rotate   (cameraState.rotation.x(), 1, 0, 0);
    _cameraMatrix.rotate   (cameraState.rotation.y(), 0, 1, 0);
    _cameraMatrix.rotate   (cameraState.rotation.z(), 0, 0, 1);

    // set clipping planes
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    const double rearClippingPlane[] = {0., 0., -1., cameraState.rearClippingDistance};
    glClipPlane(GL_CLIP_PLANE1 , rearClippingPlane);
    const double frontClippingPlane[] = {0., 0., 1., cameraState.frontClippingDistance};
    glClipPlane(GL_CLIP_PLANE2 , frontClippingPlane);

}

void GLWidget::drawPointCloud()
{
    const auto viewMatrix = _projectionMatrix * _cameraMatrix * _worldMatrix;
    _shaders->bind();
    _shaders->setUniformValue("pointsCount", static_cast<GLfloat>(pointcloud.getCount()));
    _shaders->setUniformValue("viewMatrix", viewMatrix);
    _shaders->setUniformValue("pointSize", _pointSize);
    //_shaders->setUniformValue("colorAxisMode", static_cast<GLfloat>(_colorMode));
    _shaders->setUniformValue("colorAxisMode", static_cast<GLfloat>(0));
    _shaders->setUniformValue("pointsBoundMin", pointcloud.getMin());
    _shaders->setUniformValue("pointsBoundMax", pointcloud.getMax());
    glDrawArrays(GL_POINTS, 0, pointcloud.getData().size());
    _shaders->release();
}
