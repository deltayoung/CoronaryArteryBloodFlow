#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

namespace Ui {
class viewerWidget;
}

class viewerWidget : public QOpenGLWidget //, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    viewerWidget(QWidget *parent = 0);
    ~viewerWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    Ui::viewerWidget *ui;
};

#endif // VIEWERWIDGET_H
