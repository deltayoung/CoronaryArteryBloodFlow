#include "viewerwidget.h"
#include "ui_viewerwidget.h"

#ifdef _MSC_VISUAL_STUDIO
    #pragma warning(disable:4018)
#endif

ViewerWidget::ViewerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::ViewerWidget),
    fileGetter(parent)
{
    ui->setupUi(this);

    fileGetter.setDirectory(QDir::homePath());
    //fileGetter.setDirectory(QDir::currentPath());
    fileGetter.setFileMode(QFileDialog::ExistingFiles); // to select all the input files at one go
    fileGetter.setNameFilter("*.vtk");
    if (fileGetter.exec())
    {
        meshProc.loadFilesToMeshes(fileGetter.selectedFiles());
    }

}

ViewerWidget::~ViewerWidget()
{
    delete ui;
}

void ViewerWidget::initializeGL()
{
    //initializeOpenGLFunctions();
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

}

void ViewerWidget::resizeGL(int w, int h)
{

}

void ViewerWidget::paintGL()
{
    GLfloat alpha = 0.5f;   // 1 = opaque, 0 = transparent
    for (int i=0; i<meshProc.meshList.size(); i++)
    {
        feMesh* curMesh = meshProc.meshList[i];
        for (int j=0; j<curMesh->FaceList.size(); j++)
        {
            feFace* curFace = curMesh->FaceList[j];
            glBegin(GL_TRIANGLES);
                glColor4f(1.0f, 0.0f, 0.0f, alpha);
                glVertex3f(curFace->pNode[0]->xyz[0], curFace->pNode[0]->xyz[1], curFace->pNode[0]->xyz[2]);
                glVertex3f(curFace->pNode[1]->xyz[0], curFace->pNode[1]->xyz[1], curFace->pNode[1]->xyz[2]);
                glVertex3f(curFace->pNode[2]->xyz[0], curFace->pNode[2]->xyz[1], curFace->pNode[2]->xyz[2]);
            glEnd();
        }
    }
}


