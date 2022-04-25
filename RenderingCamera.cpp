//
// (c) Georg Umlauf, 2021
//
#include "RenderingCamera.h"

const float CAMERA_STEP = 0.005f;

RenderingCameraState::RenderingCameraState(const QVector3D& _position, const QVector3D& _rotation,
            double _frontClippingDistance, double _rearClippingDistance)
  : position(_position),
    rotation(_rotation),
    frontClippingDistance(_frontClippingDistance),
    rearClippingDistance (_rearClippingDistance)
{}


RenderingCamera::RenderingCamera(QObject* parent) :
    QObject(parent),
    _frontClippingPlaneDistance(0.0),
    _rearClippingPlaneDistance (1.0),
    _xRotation(0),
    _yRotation(0),
    _zRotation(0)
{
}

void RenderingCamera::reset()
{
    _frontClippingPlaneDistance=0.0;
    _rearClippingPlaneDistance =1.0;
    _xRotation=0;
    _yRotation=0;
    _zRotation=0;
    setPosition(QVector3D(-0.05f, -0.0f, -0.1f));
    rotate(0, 130, 30);
}

void RenderingCamera::forward()
{
  _position[2] += CAMERA_STEP;
  _notify();
}

void RenderingCamera::backward()
{
  _position[2] -= CAMERA_STEP;
  _notify();
}

void RenderingCamera::left()
{
  _position[0] += CAMERA_STEP;
  _notify();
}

void RenderingCamera::right()
{
  _position[0] -= CAMERA_STEP;
  _notify();
}

void RenderingCamera::up()
{
  _position[1] -= CAMERA_STEP;
  _notify();
}

void RenderingCamera::down()
{
  _position[1] += CAMERA_STEP;
  _notify();
}

void RenderingCamera::setFrontCPDistance(double distance)
{
  _frontClippingPlaneDistance = distance;
  _notify();
}

void RenderingCamera::setRearCPDistance(double distance)
{
  _rearClippingPlaneDistance = distance;
  _notify();
}

void RenderingCamera::setPosition(const QVector3D& position)
{
  _position = position;
}

void RenderingCamera::setXRotation(int angle)
{
  angle = angle % (360 * RK);
  if (angle != _xRotation) {
    _xRotation = angle;
    _notify();
  }
}

void RenderingCamera::setYRotation(int angle)
{
  angle = angle % (360 * RK);
  if (angle != _yRotation) {
    _yRotation = angle;
    _notify();
  }
}

void RenderingCamera::setZRotation(int angle)
{
  angle = angle % (360 * RK);
  if (angle != _zRotation) {
    _zRotation = angle;
    _notify();
  }
}

void RenderingCamera::rotate(int dx, int dy, int dz)
{
  setXRotation(_xRotation + dx);
  setYRotation(_yRotation + dy);
  setZRotation(_zRotation + dz);
}

RenderingCameraState RenderingCamera::state() const
{
  return RenderingCameraState(
    _position,
    QVector3D(static_cast<float>(_xRotation/RK),
              static_cast<float>(_yRotation/RK),
              static_cast<float>(_zRotation/RK)),
    this->_frontClippingPlaneDistance,
    this->_rearClippingPlaneDistance
    );
}
