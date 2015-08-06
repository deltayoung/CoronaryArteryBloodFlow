#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QFileDialog>

#include "meshprocessor.h"

namespace Ui {
class ViewerWidget;
}

class ViewerWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit ViewerWidget(QWidget *parent = 0);
    ~ViewerWidget();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    Ui::ViewerWidget *ui;
    QFileDialog fileGetter;
    MeshProcessor meshProc;
    double left, right, top, bottom, nearVal, farVal;
};

#endif // VIEWERWIDGET_H
