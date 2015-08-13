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

    void showNextFrame();
    void showPrevFrame();
    void reverseFlowDirection();

private:
    Ui::ViewerWidget *ui;
    QFileDialog fileGetter;
    MeshProcessor meshProc;
    double left, right, top, bottom, nearVal, farVal;
    GLfloat alpha;   // 1 = opaque, 0 = transparent
    GLfloat alpha_step;
    GLint frame;    // start from 0
    bool reverse;

protected:
    void keyPressEvent(QKeyEvent*);
};

#endif // VIEWERWIDGET_H
