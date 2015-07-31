#include "viewerwidget.h"
#include "ui_viewerwidget.h"

viewerWidget::viewerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::viewerWidget)
{
    ui->setupUi(this);
}

viewerWidget::~viewerWidget()
{
    delete ui;
}

void viewerWidget::initializeGL()
{
    //initializeOpenGLFunctions();
    glClearColor(0.0, 0.0, 0.0, 1.0);

}

void viewerWidget::resizeGL(int w, int h)
{

}

void viewerWidget::paintGL()
{

}
