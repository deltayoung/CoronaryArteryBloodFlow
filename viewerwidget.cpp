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

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /*float scalar = 1.2f * 0.5f;
    glFrustum(  scalar*(meshProc.cornerMin.x-meshProc.cornerMax.x),       //left
                scalar*(meshProc.cornerMax.x-meshProc.cornerMin.x),       //right
                scalar*(meshProc.cornerMin.y-meshProc.cornerMax.y),       //bottom
                scalar*(meshProc.cornerMax.y-meshProc.cornerMin.y),       //top
                meshProc.cornerMax.z-meshProc.cornerMin.z,                //nearVal
                3.0f*(meshProc.cornerMax.z-meshProc.cornerMin.z)      //farVal
             );
*/
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void ViewerWidget::resizeGL(int w, int h)
{
    float   origAspectRatio = (meshProc.cornerMax.x-meshProc.cornerMin.x)/(meshProc.cornerMax.y-meshProc.cornerMin.y),
            newAspectRatio = (float)w/h,
            newWidthScalar, newHeightScalar;

    if (newAspectRatio > origAspectRatio)
    {
        // expand left&right, retain height
        newWidthScalar = newAspectRatio;
        newHeightScalar = 1.0f;
    }
    else
    {
        // expand bottom&top, retain width
        newWidthScalar = 1.0f;
        newHeightScalar = 1.0f/newAspectRatio;
    }

    //!! This part - try to make the frustum scale from the previous frustum rather than
    //        from the absolute object boundary (to cater for changes that have happened the original object state)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /*glFrustum(newWidthScalar*(meshProc.cornerMin.x-meshProc.cornerMax.x),       //left
              newWidthScalar*(meshProc.cornerMax.x-meshProc.cornerMin.x),       //right
              newHeightScalar*(meshProc.cornerMin.y-meshProc.cornerMax.y),       //bottom
              newHeightScalar*(meshProc.cornerMax.y-meshProc.cornerMin.y),       //top
              meshProc.cornerMax.z-meshProc.cornerMin.z,                //nearVal
              3.0f*(meshProc.cornerMax.z-meshProc.cornerMin.z)      //farVal
             );*/
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void ViewerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 1.0, 1.0);       //blue

    //test
    glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);  //red
        glVertex3f(-1.0f, -1.0f, 0.2f);
        glVertex3f(1.0f, -1.0f, 0.2f);
        glVertex3f(0.0f, 1.0f, 0.2f);

        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);  //green
        glVertex3f(-1.0f, -1.0f, 0.5f);
        glVertex3f(1.0f, -1.0f, 0.5f);
        glVertex3f(0.0f, 1.0f, 0.5f);

        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);  //yellow
        glVertex3f(-1.0f, -1.0f, 0.1f);
        glVertex3f(1.0f, -1.0f, 0.1f);
        glVertex3f(0.0f, 1.0f, 0.3f);
    glEnd();

/*    GLfloat alpha = 0.5f;   // 1 = opaque, 0 = transparent
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
*/
}


