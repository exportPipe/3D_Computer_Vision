//
// (c) Georg Umlauf, 2021
//
#include "GLConvenience.h"

void glVertex3f(const QVector3D& a) { glVertex3f(a.x(), a.y(), a.z()); }
void glVertex3f(const QVector4D& a) { glVertex3f(a.x(), a.y(), a.z()); }
void glColor3f (const QColor   & c) { glColor3f (c.red(),c.green(),c.blue()); }
void glColor4f (const QColor   & c, float a) { glColor4f (c.red(),c.green(),c.blue(),a); }
