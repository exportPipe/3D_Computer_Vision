//
// (c) Georg Umlauf, 2021
//
// This is the camera for the rendering of the scene ONLY.
//

#pragma once

#include <QObject>
#include <QVector3D>

struct RenderingCameraState {
  RenderingCameraState(const QVector3D& pos, const QVector3D& rot, double frontCD, double rearCD);

  const QVector3D position;
  const QVector3D rotation;
  const double frontClippingDistance;
  const double rearClippingDistance;
};


class RenderingCamera : public QObject
{
  Q_OBJECT

public:
  enum RotationSTEP {RK = 1};

  RenderingCamera(QObject* parent=nullptr);

  void reset();
  void forward();
  void backward();
  void left();
  void right();
  void up();
  void down();
  void setPosition(const QVector3D& position);

  void rotate(int dx, int dy, int dz);

  void setFrontCPDistance(double distance);
  void setRearCPDistance(double distance);

  RenderingCameraState state() const;

signals:
  void changed(const RenderingCameraState& newState);

public slots:
  void setXRotation(int angle);
  void setYRotation(int angle);
  void setZRotation(int angle);


private:
  double _frontClippingPlaneDistance;
  double _rearClippingPlaneDistance;
  QVector3D _position;
  int _xRotation;
  int _yRotation;
  int _zRotation;

  void _notify() { emit changed(state()); }

};

