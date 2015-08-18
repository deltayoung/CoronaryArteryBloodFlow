#include "viewerwidget.h"
#include "ui_viewerwidget.h"

#include <QKeyEvent>

//#ifdef _MSC_VISUAL_STUDIO
//   #pragma warning(disable:4018)
//#endif

ViewerWidget::ViewerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::ViewerWidget),
    fileGetter(parent),
    rotateMode(false), rotAngle(0.0f), rotX(0.0f), rotY(1.0f), rotZ(0.0f),
    zoomMode(false), zoomIn(0.0f), overZoom(0.0f),
    moveMode(false), moveX(0.0f), moveY(0.0f),
    fovFactor(0.5f),    // fovFactor=1 for 90-degree FOV (tan(90/2)=1); smaller fovFactor for smaller FOV
    alpha_step(-0.01f), alpha(1.0f),
    frame(0), reverse(false)
{
    ui->setupUi(this);

    fileGetter.setWindowTitle("Open Main Object File");
    fileGetter.setDirectory(QDir::homePath());
    //fileGetter.setDirectory(QDir::currentPath());
    fileGetter.setFileMode(QFileDialog::ExistingFiles); // to select all the input files at one go
    fileGetter.setNameFilter("*.vtk");
    if (fileGetter.exec())
    {
        meshProc.loadFilesToMeshes(fileGetter.selectedFiles());

        while (true)
        {
            QFileDialog nextFileGetter(parent);
            nextFileGetter.setWindowTitle("Open Next Background Object File");
            nextFileGetter.setDirectory(QDir::homePath());
            //nextFileGetter.setDirectory(QDir::currentPath());
            nextFileGetter.setFileMode(QFileDialog::ExistingFiles); // to select all the input files at one go
            nextFileGetter.setNameFilter("*.vtk");
            if (nextFileGetter.exec())
            {
                meshProc.loadFilesToMeshes(nextFileGetter.selectedFiles());
            }
            else
                break;
        }

        meshProc.findMeshesBoundary();
        initSceneDistance = (1+fovFactor)/fovFactor*0.5*meshProc.maxLength;
        meshProc.traversePolygonsOntoMeshesAllObjects();  // traverse all meshes of all objects
    }

    setFocusPolicy(Qt::StrongFocus);    // enable processing keyboard's key event
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

    //###############
    //If glFrustum/glOrtho is not used (i.e., the Z axis is not flipped), we have to change the display from left-handed system to right-handed system (i.e. Z- axis going into the screen),
    //by either flipping the depth range from the default (0,1) to (1,0), or to change the depth function test from the default GL_LEQUAL to GL_GEQUAL
    //glDepthRange(1.0f, 0.0f);
    //glDepthFunc(GL_GEQUAL); glClearDepth(0.0f);
    //###############

    //Enabling transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 1.0);       //black background

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (meshProc.meshList.size()>0)
    {
        float scalar = 1.0f * 0.5f, depth;

        /*//set the frustum size to the largest dimension
        right = scalar*(meshProc.cornerMax.x-meshProc.cornerMin.x);
        top = scalar*(meshProc.cornerMax.y-meshProc.cornerMin.y);
        depth = scalar*(meshProc.cornerMax.z-meshProc.cornerMin.z);
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
        }*/

        //set the frustum size to main object's maxLength
        right = top = depth = scalar*meshProc.maxLength;

        left = bottom = -right;
        nearVal = top/fovFactor;
        farVal = nearVal+meshProc.maxLength;
    }
    else
    {
        right = top = 1.0f;
        left = bottom = -1.0f;
        nearVal = 2.0f;
        farVal = 4.0f;
    }

    glFrustum(left, right, bottom, top, nearVal, farVal);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void ViewerWidget::resizeGL(int w, int h)
{
    // shrink/expand left&right, retain height & depth
    right = 0.5f*w/h*(top-bottom);
    left = -right;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, nearVal, farVal);

    glMatrixMode(GL_MODELVIEW);

}

void ViewerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //test
    if (meshProc.meshList.size()<1)
    {
        glBegin(GL_TRIANGLES);
            //Warning: the objects should be rendered in order of back-to-front to achieve proper transparency effect!
            glColor4f(0.0f, 0.0f, 1.0f, 1.0f);  //blue
            glVertex3f(-1.0f, -1.0f, -3.5f);
            glVertex3f(1.0f, -1.0f, -3.5f);
            glVertex3f(0.0f, 1.0f, -3.5f);

            glColor4f(0.0f, 1.0f, 0.0f, alpha);  //green
            glVertex3f(-1.0f, -1.0f, -2.5f);
            glVertex3f(1.0f, -1.0f, -2.5f);
            glVertex3f(0.0f, 1.0f, -3.5f);

            glColor4f(1.0f, 0.0f, 0.0f, alpha);  //red
            glVertex3f(-1.0f, -1.0f, -3.0f);
            glVertex3f(1.0f, -1.0f, -3.0f);
            glVertex3f(0.0f, 1.0f, -3.0f);

        glEnd();

        // change transparency at the next paint
        if (alpha+alpha_step < 0.0f || alpha+alpha_step > 1.0f)
            alpha_step = -alpha_step;   // reverse
        alpha += alpha_step;
    }
    else
    {
        if (zoomMode)
        {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(left, right, bottom, top, nearVal, farVal);

            glMatrixMode(GL_MODELVIEW);
        }

        glPushMatrix();

        // move the object to a distance from the camera dynamically to put the object in the centre of the current frustum
        glTranslatef(moveX, moveY, zoomIn-initSceneDistance);
        //glTranslatef(0.0f, 0.0f, -initSceneDistance);

        // reorient the object to see from its side assuming the default view is from the top
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(rotAngle, rotX, rotY, rotZ);  // rotation by the left click

        // move the object to position its center to the origin
        glTranslatef(-0.5f*(meshProc.cornerMin.x+meshProc.cornerMax.x),
                     -0.5f*(meshProc.cornerMin.y+meshProc.cornerMax.y),
                     -0.5f*(meshProc.cornerMin.z+meshProc.cornerMax.z));

        //Warning: the objects should be rendered in order of back-to-front to achieve proper transparency effect!
        //Make sure the meshes are ordered from the wall first, then the blood arteries!
        //GLfloat curColor[4] = {0.5f, 0.2f, 0.2f, 1.0f}; // wall color

        feMesh* curMesh = meshProc.meshList[frame];
        for (int j=0; j<curMesh->FaceList.size(); j++)
        {
            feFace* curFace = curMesh->FaceList[j];
            glBegin(GL_TRIANGLES);
                //glColor4fv(curColor);

                glColor4f(1.0f, 0.0f, 0.0f, reverse?(1-curFace->scalarAttrib):curFace->scalarAttrib);
                glVertex3f(curFace->pNode[0]->xyz[0], curFace->pNode[0]->xyz[1], curFace->pNode[0]->xyz[2]);
                glVertex3f(curFace->pNode[1]->xyz[0], curFace->pNode[1]->xyz[1], curFace->pNode[1]->xyz[2]);
                glVertex3f(curFace->pNode[2]->xyz[0], curFace->pNode[2]->xyz[1], curFace->pNode[2]->xyz[2]);
            glEnd();
        }
        for (int i=0; i<meshProc.secondaryMeshLists.size(); i++)
        {
            curMesh = meshProc.secondaryMeshLists[i][frame];
             for (int j=0; j<curMesh->FaceList.size(); j++)
             {
                 feFace* curFace = curMesh->FaceList[j];
                 glBegin(GL_TRIANGLES);
                     //glColor4fv(curColor);
                     glColor4f(0.8f, 0.2f, 0.2f, reverse?curFace->scalarAttrib:(1-curFace->scalarAttrib));
                     glVertex3f(curFace->pNode[0]->xyz[0], curFace->pNode[0]->xyz[1], curFace->pNode[0]->xyz[2]);
                     glVertex3f(curFace->pNode[1]->xyz[0], curFace->pNode[1]->xyz[1], curFace->pNode[1]->xyz[2]);
                     glVertex3f(curFace->pNode[2]->xyz[0], curFace->pNode[2]->xyz[1], curFace->pNode[2]->xyz[2]);
                 glEnd();
             }
        }

        glPopMatrix();
    }
}

void ViewerWidget::showNextFrame()
{
    frame = (frame+1)==meshProc.meshList.size()? 0 : (frame+1);
    update();
}

void ViewerWidget::showPrevFrame()
{
    frame = (frame==0) ? (meshProc.meshList.size()-1) : (frame-1);
    update();
}

void ViewerWidget::reverseFlowDirection()
{
    reverse = !reverse;
    update();
}

void ViewerWidget::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Right:
            showNextFrame();
            break;
        case Qt::Key_Left:
            showPrevFrame();
            break;
        case Qt::Key_R:
            reverseFlowDirection();
            break;

    }
}

void ViewerWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)   // left click for rotating
    {
        startPos = event->pos();
        rotateMode = true;
    }
    else if (event->button() == Qt::RightButton) // right click for zooming
    {
        startPos = event->pos();
        zoomMode = true;
    }
    else if (event->button() == Qt::MidButton) // middle click for moving
    {
        startPos = event->pos();
        moveMode = true;
    }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->button() && Qt::LeftButton && rotateMode)
    {
        rotate(event->pos());
        startPos = event->pos();
    }
    else if (event->buttons() && Qt::RightButton && zoomMode)
    {
        zoom(event->pos());
        startPos = event->pos();
    }
    else if (event->buttons() && Qt::MidButton && moveMode)
    {
        moveTo(event->pos());
        startPos = event->pos();
    }
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && rotateMode)
    {
        rotate(event->pos());
        rotateMode = false;
    }
    else if (event->button() == Qt::RightButton && zoomMode)
    {
        zoom(event->pos());
        zoomMode = false;
    }
    else if (event->button() == Qt::MidButton && moveMode)
    {
        moveTo(event->pos());
        moveMode = false;
    }
}

void ViewerWidget::rotate(QPoint curPos)
{
    QPoint rotateVector = curPos-startPos;
    cout << "DebugPt: rotateVector=(" << rotateVector.x() << ", " << rotateVector.y() << ")\n";
    float curRotAngle = 360.0f * sqrt((double)(rotateVector.x()*rotateVector.x()+rotateVector.y()*rotateVector.y()) / (QWidget::width()*QWidget::width()+QWidget::height()*QWidget::height()));
    QQuaternion curQ = QQuaternion::fromAxisAndAngle(rotateVector.x(), rotateVector.y(), 0.0f, curRotAngle);
    quaternion *= curQ;
    //quaternion.getAxisAndAngle(&rotX, &rotY, &rotZ, &rotAngle);
    QVector4D curRotation = quaternion.toVector4D();
    rotX = curRotation.x()/curRotation.w();
    rotY = curRotation.y()/curRotation.w();
    rotZ = curRotation.z()/curRotation.w();
    rotAngle = curRotation.w();

    update();
}

void ViewerWidget::zoom(QPoint curPos)
{
    float zoomFactor = (float)(curPos.y()-startPos.y()) / QWidget::height(); // + zoom in, - zoom out

    float origDepth = farVal-nearVal;
    float zoomAmt = zoomFactor*farVal; //zoomFactor*origDepth;   //zoomFactor*nearVal;
    zoomIn += zoomAmt;
    if (nearVal-zoomAmt >= 1.0f && overZoom <= 0.0f)
    {
        nearVal = nearVal - zoomAmt - overZoom;
        overZoom = 0.0f;
        farVal = nearVal+origDepth;
        float newTop = nearVal*fovFactor; // change the frustum while retaining field of view
        right *= newTop/top;
        top = newTop;
        left = -right;
        bottom = -top;
    }
    else    // zoom in beyond nearVal plane (nearVal shouldn't be <1.0f for over-distorted frustum)
        overZoom += zoomAmt;

    update();
}

void ViewerWidget::moveTo(QPoint curPos)
{
    QPoint moveFactor = curPos-startPos; // + move to right/bottom, - move to left/top

    float overZoomFactor = max(1.0f, 1.0f+overZoom/(float)nearVal); // handle overzoom condition beyond nearVal plane
    moveX += overZoomFactor*((right-left) * moveFactor.x() / QWidget::width());
    moveY -= overZoomFactor*((top-bottom) * moveFactor.y() / QWidget::height());

    update();
}
