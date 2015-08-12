#include "meshprocessor.h"

MeshProcessor::MeshProcessor()
{

}

MeshProcessor::~MeshProcessor()
{

}

void MeshProcessor::setFilenames(QStringList f)
{
    filenames = f;
}

void MeshProcessor::loadFilesToMeshes(QStringList f)
{
    setFilenames(f);
    feMesh* newMesh;
    for (int i=0; i<filenames.size(); i++)
    {
        newMesh = new feMesh();
        readVTK(filenames.at(i).toLocal8Bit().data(), newMesh);
        meshList.push_back(newMesh);
    }

    findMeshesBoundary();
}

void MeshProcessor::findMeshesBoundary()
{
    cornerMin.set(meshList[0]->NodeList[0]->xyz[0], meshList[0]->NodeList[0]->xyz[1], meshList[0]->NodeList[0]->xyz[2]);
    cornerMax.set(meshList[0]->NodeList[0]->xyz[0], meshList[0]->NodeList[0]->xyz[1], meshList[0]->NodeList[0]->xyz[2]);

    for (int i=0; i<meshList.size(); i++)
    {
        for (int j=0; j<meshList[i]->NodeList.size(); j++)
        {
            if (cornerMin.x > meshList[i]->NodeList[j]->xyz[0])
                cornerMin.x = meshList[i]->NodeList[j]->xyz[0];
            if (cornerMin.y > meshList[i]->NodeList[j]->xyz[1])
                cornerMin.y = meshList[i]->NodeList[j]->xyz[1];
            if (cornerMin.z > meshList[i]->NodeList[j]->xyz[2])
                cornerMin.z = meshList[i]->NodeList[j]->xyz[2];

            if (cornerMax.x < meshList[i]->NodeList[j]->xyz[0])
                cornerMax.x = meshList[i]->NodeList[j]->xyz[0];
            if (cornerMax.y < meshList[i]->NodeList[j]->xyz[1])
                cornerMax.y = meshList[i]->NodeList[j]->xyz[1];
            if (cornerMax.z < meshList[i]->NodeList[j]->xyz[2])
                cornerMax.z = meshList[i]->NodeList[j]->xyz[2];
        }
    }
}

void MeshProcessor::traversePolygonsOntoMeshes()
{
    //Objective: assuming all the meshes form an ordered sequence of frames for animation, mark each frame as we traverse from the topmost point of the mesh at the first frame towards the bottom at the last frame
    //Strategy: find a point with the maximum Z value as the seed for the initial traversion vertically downwards
    //Subsequently, repeat the above for the pool of remaining untraversed points, until all points are traversed
    //Face states: 0 = untraversed, 1 = traversed, 2 = processed

    int seedIndex = -1;
    int levelCount=0;
    //float curVal = 0.0f;
    // search for the seed, i.e., the point with maximum Z value
    while (foundNewSeed(seedIndex))
    {
        cout << "DebugPt1\n";
        levelCount = recursiveTraverse(meshList[0]->FaceList[seedIndex], 0); // maximum: 2log(n-2), with n=number of faces per mesh
        cout << "DebugPt2, levelCount=" << levelCount << "\n";
        int halfSize = meshList.size()/2;
        for (int curFrame=0; curFrame<=halfSize; curFrame++)
        {
            for (int faceIndex=0; faceIndex<meshList[curFrame]->FaceList.size(); faceIndex++)
            {
                //problem: multiple executions every time a new seed is found, with the new overall scale
                //curVal = meshList[curFrame]->FaceList[faceIndex]->scalarAttrib;
                if (meshList[curFrame]->FaceList[faceIndex]->state == 1)    // traversed faces only
                {
                    meshList[curFrame]->FaceList[faceIndex]->state = 2; // processed
                    if (meshList[curFrame]->FaceList[faceIndex]->scalarAttrib/levelCount <= curFrame/halfSize)
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = 1.0f;
                    else
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = 0.0f;
                }
            }
        }
        for (int curFrame=meshList.size()-1; curFrame>halfSize; curFrame--)
        {
            for (int faceIndex=0; faceIndex<meshList[curFrame]->FaceList.size(); faceIndex++)
            {
                //curVal = meshList[curFrame]->FaceList[faceIndex]->scalarAttrib;
                if (meshList[curFrame]->FaceList[faceIndex]->state == 1) // traversed faces only
                {
                    meshList[curFrame]->FaceList[faceIndex]->state = 2; // processed
                    if (meshList[curFrame]->FaceList[faceIndex]->scalarAttrib/levelCount >= curFrame/halfSize)
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = 0.0f;
                    else
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = 1.0f;
                }
            }
        }
    }
    cout << "DebugPt3\n";
}

int MeshProcessor::recursiveTraverse(feFace *f, int level)
{
    f->scalarAttrib = (float)level;
    f->state = 1;   // mark traversed

    int highestLevel = level, curLevel;
    vector<feFace*> neighbours = f->getNeighbours();
    for (int i=0; i<neighbours.size(); i++)
    {
        if (neighbours[i]->state == 0)    //untraversed neighbours
        {
            curLevel = recursiveTraverse(neighbours[i], level+1);
            if (curLevel > highestLevel)
                highestLevel = curLevel;
        }
    }
    return highestLevel;
}

bool MeshProcessor::foundNewSeed(int &seedIndex)
{
    float maxZ;
    bool found = false;
    for (int i=0; i<meshList[0]->NodeList.size(); i++)
    {
        for (list<feFace*>::const_iterator j=meshList[0]->NodeList[i]->pFace.begin(); j!=meshList[0]->NodeList[i]->pFace.end(); ++j)
        {
            if ((*j)->state == 0)   // untraversed
            {
                maxZ = meshList[0]->NodeList[i]->xyz[2];    // initial maximum Z value
            }
        }
    }
    for (int i=0; i<meshList[0]->FaceList.size(); i++)
    {
        if (meshList[0]->FaceList[i]->state == 0)   // untraversed
        {
            for (int j=0; j<3; j++)
            {
                if (meshList[0]->FaceList[i]->pNode[j]->xyz[2] > maxZ)
                {
                    maxZ = meshList[0]->FaceList[i]->pNode[j]->xyz[2];
                    seedIndex = i;
                    found = true;
                }
            }
        }
    }
    return found;
}
