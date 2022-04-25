//
// (c) Georg Umlauf, 2021
//

#pragma once

#include <QtWidgets/QMainWindow>
#include <QVector3D>
#include <QSharedPointer>

#include "ui_mainwindow.h"

#include "RenderingCamera.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected slots:
  void updatePointSize(size_t);

private:
	Ui::MainWindowClass *ui;
    QSharedPointer<RenderingCamera> _camera;
};
