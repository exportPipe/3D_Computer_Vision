#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
// (c) Nico Brügel, 2021

#include "glwidget.h"
#include <QtGui>

#if defined(__APPLE__)
// we're on macOS and according to their documentation Apple hates developers
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#endif

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

QMatrix4x4 rotate_x(float angle)
{
    float degrees = angle * M_PI / 180;
    float cos = cosf(degrees);
    float sin = sinf(degrees);
    QVector4D v1 = QVector4D(1, 0, 0, 0);
    QVector4D v2 = QVector4D(sin, cos, 0, 0);
    QVector4D v3 = QVector4D(0, 0, 1, 0);
    QVector4D v4 = QVector4D(0, 0, 0, 1);
    QMatrix4x4 rotation_matrix;
    rotation_matrix.setToIdentity();
    rotation_matrix.setColumn(0, v1);
    rotation_matrix.setColumn(1, v2);
    rotation_matrix.setColumn(2, v3);
    rotation_matrix.setColumn(3, v4);
    return rotation_matrix;
}

QMatrix4x4 rotate_y(float angle)
{
    float degrees = angle * M_PI / 180;
    float cos = cosf(degrees);
    float sin = sinf(degrees);
    QVector4D v1 = QVector4D(cos, 0, sin, 0);
    QVector4D v2 = QVector4D(0, 1, 0, 0);
    QVector4D v3 = QVector4D(-sin, 0, cos, 0);
    QVector4D v4 = QVector4D(0, 0, 0, 1);
    QMatrix4x4 rotation_matrix;
    rotation_matrix.setToIdentity();
    rotation_matrix.setColumn(0, v1);
    rotation_matrix.setColumn(1, v2);
    rotation_matrix.setColumn(2, v3);
    rotation_matrix.setColumn(3, v4);
    return rotation_matrix;
}

QMatrix4x4 rotate_z(float angle)
{
    float degrees = angle * M_PI / 180;
    float cos = cosf(degrees);
    float sin = sinf(degrees);
    QVector4D v1 = QVector4D(cos, -sin, 0, 0);
    QVector4D v2 = QVector4D(sin, cos, 0, 0);
    QVector4D v3 = QVector4D(0, 0, 1, 0);
    QVector4D v4 = QVector4D(0, 0, 0, 1);
    QMatrix4x4 rotation_matrix;
    rotation_matrix.setToIdentity();
    rotation_matrix.setColumn(0, v1);
    rotation_matrix.setColumn(1, v2);
    rotation_matrix.setColumn(2, v3);
    rotation_matrix.setColumn(3, v4);
    return rotation_matrix;
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
    // Draw here your objects as in drawFrameAxis()
    vector<QVector3D> object1;
    vector<QVector3D> object2;
    QVector4D object1_position = QVector4D(0.0, 0.0, 8.0, 1.0);
    QVector4D object2_position = QVector4D(1.0, 1.0, 6.0, 1.0);
    initCuboid(object1, object1_position, 0.8, 0.0, 50.0, 0.0);
    initCuboid(object2, object2_position, 1.2, 50.0, 0.0, 0.0);

    QColor red = QColor(1.0f, 0.0f, 0.0f);
    QColor green = QColor(0.0f, 1.0f, 0.0f);
    QColor blue = QColor(0.0f, 0.0f, 1.0f);

    drawObject(object1, red);
    drawObject(object2, green);




    // Assignement 1, Part 2
    // Draw here your perspective camera model
    vector<QVector3D> projectionPlane;
    QVector4D camera_position = QVector4D(1, 1, 1, 1);

    float focal_length = 2.0;
    float image_plane_size = 3.0;

    QVector3D rotation_camera = QVector3D(0, 0, 0);
    QVector4D priciple_point = getPrinciplePoint(focal_length, camera_position, rotation_camera);


    initPerspectiveCamera(projectionPlane, camera_position, rotation_camera);

    vector<QVector3D> imagePlane;
    vector<QVector3D> imagePlaneAxes;
    initImagePlane(imagePlane, imagePlaneAxes, camera_position, image_plane_size, focal_length, rotation_camera, priciple_point);

    drawObject(imagePlane, red);
    drawObject(imagePlaneAxes, red);




    // Assignement 1, Part 3
    // Draw here the perspective projection
    vector<QVector3D> object1_projection;
    vector<QVector3D> object2_projection;
    initProjection(object1, object1_projection, focal_length, camera_position, priciple_point, rotation_camera);
    initProjection(object2, object2_projection, focal_length, camera_position, priciple_point, rotation_camera);
    drawObject(object1_projection, red);
    drawObject(object2_projection, green);

    vector<QVector3D> projection_lines;
    initProjectionLines(object1, projection_lines, camera_position);
    initProjectionLines(object2, projection_lines, camera_position);
    drawObject(projection_lines, blue);

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


void GLWidget::drawObject(vector<QVector3D> object, QColor color)
{
    glBegin(GL_LINES);
    glColor3f(color);
    QMatrix4x4 mvMatrix = _projectionMatrix * _cameraMatrix * _worldMatrix;
    mvMatrix.scale(0.05f); // make it small
    for (auto vertex : object) {
      const auto translated = mvMatrix ^ vertex;
      glVertex3f(translated);
    }
    glEnd();
}


void GLWidget::initCuboid(vector<QVector3D> &cuboid, QVector4D translation, float scale, float angle_x, float angly_y, float angle_z)
{
    QMatrix4x4 translation_matrix;
    translation_matrix.setToIdentity();
    translation_matrix.setColumn(3, translation);

    QMatrix4x4 rotation_matrix = rotate_x(angle_x) * rotate_y(angly_y) * rotate_z(angle_z);


    QVector3D v1 = QVector3D(0.0, 0.0, 0.0);
    QVector3D v2 = QVector3D(1.0, 0.0, 0.0);
    QVector3D v3 = QVector3D(1.0, 1.0, 0.0);
    QVector3D v4 = QVector3D(0.0, 1.0, 0.0);

    v1 = rotation_matrix * v1;
    v2 = rotation_matrix * v2;
    v3 = rotation_matrix * v3;
    v4 = rotation_matrix * v4;

    v1 = translation_matrix * v1 * scale;
    v2 = translation_matrix * v2 * scale;
    v3 = translation_matrix * v3 * scale;
    v4 = translation_matrix * v4 * scale;


    QVector3D v11 = QVector3D(0.0, 0.0, 1.0);
    QVector3D v22 = QVector3D(1.0, 0.0, 1.0);
    QVector3D v33 = QVector3D(1.0, 1.0, 1.0);
    QVector3D v44 = QVector3D(0.0, 1.0, 1.0);

    v11 = rotation_matrix * v11;
    v22 = rotation_matrix * v22;
    v33 = rotation_matrix * v33;
    v44 = rotation_matrix * v44;

    v11 = translation_matrix * v11 * scale;
    v22 = translation_matrix * v22 * scale;
    v33 = translation_matrix * v33 * scale;
    v44 = translation_matrix * v44 * scale;

    cuboid.push_back(v1);
    cuboid.push_back(v2);
    cuboid.push_back(v2);
    cuboid.push_back(v3);
    cuboid.push_back(v3);
    cuboid.push_back(v4);
    cuboid.push_back(v4);
    cuboid.push_back(v1);
    cuboid.push_back(v11);
    cuboid.push_back(v22);
    cuboid.push_back(v22);
    cuboid.push_back(v33);
    cuboid.push_back(v33);
    cuboid.push_back(v44);
    cuboid.push_back(v44);
    cuboid.push_back(v11);
    cuboid.push_back(v1);
    cuboid.push_back(v11);
    cuboid.push_back(v2);
    cuboid.push_back(v22);
    cuboid.push_back(v3);
    cuboid.push_back(v33);
    cuboid.push_back(v4);
    cuboid.push_back(v44);
}

void GLWidget::initPerspectiveCamera(vector<QVector3D> &axes_lines, QVector4D translation, QVector3D rotation)
{
    QMatrix4x4 translation_matrix;
    translation_matrix.setToIdentity();
    translation_matrix.setColumn(3, translation);

    QMatrix4x4 rotation_matrix = rotate_x(rotation.x()) * rotate_y(rotation.y()) * rotate_z(rotation.z());
    QVector3D center = QVector3D(0.0, 0.0, 0.0);
    QVector3D x_axes = QVector3D(0.5, 0.0, 0.0);
    QVector3D y_axes = QVector3D(0.0, 0.5, 0.0);
    QVector3D z_axes = QVector3D(0.0, 0.0, 0.5);

    center = translation_matrix * rotation_matrix * center;
    x_axes = translation_matrix * rotation_matrix  * x_axes;
    y_axes = translation_matrix * rotation_matrix * y_axes;
    z_axes = translation_matrix * rotation_matrix * z_axes;

    axes_lines.push_back(center);
    axes_lines.push_back(x_axes);
    axes_lines.push_back(center);
    axes_lines.push_back(y_axes);
    axes_lines.push_back(center);
    axes_lines.push_back(z_axes);
}

QVector4D GLWidget::getPrinciplePoint(float focal_length, QVector4D position_camera, QVector3D rotation_camera)
{
    QMatrix4x4 translationMatrix;
    translationMatrix.setToIdentity();
    translationMatrix.setColumn(3, position_camera);

    QMatrix4x4 rotation_matrix = rotate_x(rotation_camera.x()) * rotate_y(rotation_camera.y()) * rotate_z(rotation_camera.z());
    QVector4D principle_point = QVector4D(0, 0, focal_length, 1);
    principle_point = translationMatrix * rotation_matrix * principle_point;
    return principle_point;
}

void GLWidget::initImagePlane(vector<QVector3D> &image_plane, vector<QVector3D> &axes,
                    QVector4D position, float scale, float focal_length,
                    QVector3D rotation, QVector4D principle_point)
{

    QMatrix4x4 scaling_matrix;
    scaling_matrix = QMatrix4x4(scale, 0.0, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    QMatrix4x4 translation_matrix;
    translation_matrix.setToIdentity();
    translation_matrix.setColumn(3, position);

    QMatrix4x4 rotation_matrix = rotate_x(rotation.x()) * rotate_y(rotation.y()) * rotate_z(rotation.z());

    QVector3D v1 = QVector3D(1.0, 1.0, focal_length);
    QVector3D v2 = QVector3D(1.0, -1.0, focal_length);
    QVector3D v3 = QVector3D(-1.0, -1.0, focal_length);
    QVector3D v4 = QVector3D(-1.0, 1.0, focal_length);
    v1 = translation_matrix * rotation_matrix * scaling_matrix * v1;
    v2 = translation_matrix * rotation_matrix * scaling_matrix * v2;
    v3 = translation_matrix * rotation_matrix * scaling_matrix * v3;
    v4 = translation_matrix * rotation_matrix * scaling_matrix * v4;

    image_plane.push_back(v1);
    image_plane.push_back(v2);
    image_plane.push_back(v2);
    image_plane.push_back(v3);
    image_plane.push_back(v3);
    image_plane.push_back(v4);
    image_plane.push_back(v4);
    image_plane.push_back(v1);

    QVector3D x_axes = QVector3D(0.5, 0.0, focal_length);
    QVector3D y_axes = QVector3D(0.0, 0.5, focal_length);
    x_axes = translation_matrix * x_axes;
    y_axes = translation_matrix * y_axes;

    axes.push_back(principle_point.toVector3D());
    axes.push_back(x_axes);
    axes.push_back(principle_point.toVector3D());
    axes.push_back(y_axes);
}

void GLWidget::initProjection(vector<QVector3D> object, vector<QVector3D> &projection, float focal_length,
                    QVector4D center, QVector4D principle_point, QVector3D rotation)
{
    QMatrix4x4 rotation_matrix = rotate_x(rotation.x()) * rotate_y(rotation.y()) * rotate_z(rotation.z());
    QVector3D v1 = QVector3D(1.0, 0.0, 0.0);
    QVector3D v2 = QVector3D(0.0, 1.0, 0.0);
    v1 = rotation_matrix * v1;
    v2 = rotation_matrix * v2;
    float normalized_x = v1.y() * v2.z() - v1.z() * v2.y();
    float normalized_y = v1.z() * v2.x() - v1.x() * v2.z();
    float normalized_z = v1.x() * v2.y() - v1.y() - v2.z();
    float result_v = normalized_x * principle_point.x() + normalized_y * principle_point.y() + normalized_z * principle_point.z();

    QVector4D image_plane = QVector4D(normalized_x, normalized_y, normalized_z, result_v);
    for (QVector3D vertex: object)
    {
        QVector3D projected_point = centralProjection(focal_length, vertex, center.toVector3D(), rotation);
        projection.push_back(projected_point);
    }
}

QVector3D GLWidget::centralProjection(float focal_length, QVector3D vertex, QVector3D center, QVector3D rotation)
{

    QMatrix4x4 rotation_matrix_t = (rotate_x(rotation.x()) * rotate_y(rotation.y()) * rotate_z(rotation.z())).transposed();
    QMatrix4x4 rotation_matrix = (rotate_x(rotation.x()) * rotate_y(rotation.y()) * rotate_z(rotation.z()));

    // lines from projection/camera center to image plane
    //calculate x
    // rotation around x axes
    float x_term_1 = rotation_matrix_t.column(0).x() * (vertex.x() - center.x());
    float x_term_2 = rotation_matrix_t.column(1).x() * (vertex.y() - center.y());
    float x_term_3 = rotation_matrix_t.column(2).x() * (vertex.z() - center.z());

    // rotation around z axes
    float x_term_4 = rotation_matrix_t.column(0).z() * (vertex.x() - center.x());
    float x_term_5 = rotation_matrix_t.column(1).z() * (vertex.y() - center.y());
    float x_term_6 = rotation_matrix_t.column(2).z() * (vertex.z() - center.z());

    float x = focal_length * ( (x_term_1 + x_term_2 + x_term_3) / (x_term_4 + x_term_5 + x_term_6));

    // calculate y
    float y_term_1 = rotation_matrix_t.column(0).y() * (vertex.x() - center.x());
    float y_term_2 = rotation_matrix_t.column(1).y() * (vertex.y() - center.y());
    float y_term_3 = rotation_matrix_t.column(2).y() * (vertex.z() - center.z());

    float y_term_4 = rotation_matrix_t.column(0).z() * (vertex.x() - center.x());
    float y_term_5 = rotation_matrix_t.column(1).z() * (vertex.y() - center.y());
    float y_term_6 = rotation_matrix_t.column(2).z() * (vertex.z() - center.z());

    float y = focal_length * ((y_term_1 + y_term_2 + y_term_3) / (y_term_4 + y_term_5 + y_term_6));

    // calculate z
    float z = focal_length;

    QVector3D result = QVector3D(x, y, z);
    result = rotation_matrix * result;

    return QVector3D(result.x() + center.x(), result.y() + center.y(), result.z() + center.z());
}

void GLWidget::initProjectionLines(vector<QVector3D> object, vector<QVector3D> &projection_lines, QVector4D center)
{
    for (QVector3D vertex: object)
    {
        projection_lines.push_back(vertex);
        projection_lines.push_back(center.toVector3D());
    }
}
