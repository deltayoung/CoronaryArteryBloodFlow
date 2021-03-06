#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QFileDialog>
#include <QQuaternion>

#include "meshprocessor.h"

namespace Ui {
class ViewerWidget;
}

class ViewerWidget : public QOpenGLWidget
{
    Q_OBJECT    // mark that this class contains slots and signals

public:
    explicit ViewerWidget(QWidget *parent = 0);
    ~ViewerWidget();

private:
    Ui::ViewerWidget *ui;
    QFileDialog fileGetter;

    QPoint      startPos;
    bool        rotateMode, zoomMode, moveMode;
    QQuaternion quaternion;
    float       rotAngle, rotX, rotY, rotZ,
                zoomIn, overZoom,
                moveX, moveY;

    MeshProcessor meshProc;
    float fovFactor;  // tan(0.5*field of view), where field of view is the angle from top to bottom
    float initSceneDistance; // initial distance from camera to center of main object
    GLdouble left, right, top, bottom, nearVal, farVal;
    GLfloat alpha;   // 1 = opaque, 0 = transparent
    GLfloat alpha_step;
    GLint frame;    // start from 0
    bool reverse;

    void showNextFrame();
    void showPrevFrame();
    void reverseFlowDirection();

    void rotate(QPoint);
    void zoom(QPoint);
    void moveTo(QPoint);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent*) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent*) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent*) Q_DECL_OVERRIDE;
};

#endif // VIEWERWIDGET_H
