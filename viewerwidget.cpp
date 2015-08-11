#include "viewerwidget.h"
#include "ui_viewerwidget.h"

#ifdef _MSC_VISUAL_STUDIO
    #pragma warning(disable:4018)
#endif

ViewerWidget::ViewerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::ViewerWidget),
    fileGetter(parent),
    alpha_step(0.001f)
{
    ui->setupUi(this);

    alpha = 0.0f;

    fileGetter.setDirectory(QDir::homePath());
    //fileGetter.setDirectory(QDir::currentPath());
    fileGetter.setFileMode(QFileDialog::ExistingFiles); // to select all the input files at one go
    fileGetter.setNameFilter("*.vtk");
    if (fileGetter.exec())
        meshProc.loadFilesToMeshes(fileGetter.selectedFiles());

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

    if (meshProc.meshList.size()>0)
    {
        float scalar = 1.0f * 0.5f;
        right = scalar*(meshProc.cornerMax.x-meshProc.cornerMin.x);
        top = scalar*(meshProc.cornerMax.y-meshProc.cornerMin.y);
        float depth = scalar*(meshProc.cornerMax.z-meshProc.cornerMin.z);
        //set the frustum size to the largest dimension
        if (right>top)
        {
            if (right>depth)
                top = depth = right;
            else
                top = right = depth;
        }
        else
        {
            if (top>depth)
                right = depth = top;
            else
                top = right = depth;
        }
        left = bottom = -right;
        nearVal = depth/scalar; // field of view = 90 degrees
        farVal = 2.0f*nearVal;
    }
    else
    {
        right = top = 1.0f;
        left = bottom = -1.0f;
        nearVal = 1.0f;
        farVal = 2.0f;
    }

    glFrustum(left, right, bottom, top, nearVal, farVal);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void ViewerWidget::resizeGL(int w, int h)
{
    float   origAspectRatio = (right-left)/(top-bottom),
            newAspectRatio = (float)w/h;

    if (newAspectRatio > origAspectRatio)
    {
        // expand left&right, retain height & depth
        right = 0.5f*newAspectRatio*(top-bottom);
        left = -right;
    }
    else
    {
        // expand top&bottom, retain width & depth
        top = 0.5f*(right-left)/newAspectRatio;
        bottom = -top;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, nearVal, farVal);

    glMatrixMode(GL_MODELVIEW);

}

void ViewerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);       //black

    //test
    if (meshProc.meshList.size()<1)
    {
        glBegin(GL_TRIANGLES);
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);  //red
            glVertex3f(-1.0f, -1.0f, 1.2f);
            glVertex3f(1.0f, -1.0f, 1.2f);
            glVertex3f(0.0f, 1.0f, 1.2f);

            glColor4f(0.0f, 1.0f, 0.0f, 1.0f);  //green
            glVertex3f(-1.0f, -1.0f, 1.5f);
            glVertex3f(1.0f, -1.0f, 1.5f);
            glVertex3f(0.0f, 1.0f, 1.5f);

            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);  //yellow
            glVertex3f(-1.0f, -1.0f, 1.1f);
            glVertex3f(1.0f, -1.0f, 1.1f);
            glVertex3f(0.0f, 1.0f, 1.3f);
        glEnd();
    }

    glPushMatrix();

    // move the object to a distance from the camera dynamically to put the object in the centre of the current frustum
    glTranslatef(0.0f, 0.0f, -0.5f*(farVal+nearVal));

    // reorient the object to see from its side assuming the default view is from the top
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // move the object to position its center to the origin
    glTranslatef(-0.5f*(meshProc.cornerMin.x+meshProc.cornerMax.x),
                 -0.5f*(meshProc.cornerMin.y+meshProc.cornerMax.y),
                 -0.5f*(meshProc.cornerMin.z+meshProc.cornerMax.z));

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
    alpha = (alpha==1.0f?0.0f:alpha+alpha_step);

    glPopMatrix();
}


