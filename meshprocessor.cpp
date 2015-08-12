#include "meshprocessor.h"

#include <queue>

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
    //Objective: assuming all the meshes form an ordered sequence of frames for animation (1-to-1 mapping for all meshes), mark each frame as we traverse from the topmost point of the mesh at the first frame towards the bottom at the last frame
    //Strategy: find a point with the maximum Z value as the seed for the initial traversion vertically downwards
    //Subsequently, repeat the above for the pool of remaining untraversed points, until all points are traversed
    //Face states: 0 = untraversed, 1 = traversed, 2 = processed

    int seedIndex = -1;
    int levelCount=0;
    //float curVal = 0.0f;
    // search for the seed, i.e., the point with maximum Z value
    while (foundNewSeed(seedIndex))
    {
        //levelCount = depthFirstTraverse(meshList[0]->FaceList[seedIndex], 0); // maximum: 2log(n-2), with n=number of faces per mesh
        levelCount = breadthFirstTraverse(meshList[0]->FaceList[seedIndex]);    // BFT has better spread/flow than DFT
        cout << "DebugPt, levelCount=" << levelCount << "\n";
        int halfSize = meshList.size()/2;
        if (levelCount == 0 && meshList[0]->FaceList[seedIndex]->state == 1)    // exceptional branch: single unconnected face
        {
            meshList[0]->FaceList[seedIndex]->state = 2; // processed
            for (int curFrame=0; curFrame<=halfSize; curFrame++)
                meshList[curFrame]->FaceList[seedIndex]->scalarAttrib = curFrame/halfSize;
            for (int curFrame=meshList.size()-1; curFrame>halfSize; curFrame--)
                meshList[curFrame]->FaceList[seedIndex]->scalarAttrib = (1-curFrame+halfSize)/halfSize;
            continue;
        }

        float trailFactor = 0.3f, trail, full, minVal = 0.2f, maxVal = 1.0f;
        for (int faceIndex=0; faceIndex<meshList[0]->FaceList.size(); faceIndex++)
        {
            if (meshList[0]->FaceList[faceIndex]->state == 1)    // traversed faces only
            {
                meshList[0]->FaceList[faceIndex]->state = 2; // processed
                float curLevel = meshList[0]->FaceList[faceIndex]->scalarAttrib/levelCount;
                for (int curFrame=0; curFrame<=halfSize; curFrame++)
                {   
                    trail = (float)curFrame/halfSize;
                    full = (1.0f-trailFactor)*trail;

                    if (curLevel <= full)   // clamp
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal;
                    else if (curLevel > full && curLevel < trail)      // trail
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal-curLevel*(maxVal-minVal);
                    else    // clamp
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = minVal;
                }
                for (int curFrame=halfSize+1; curFrame<meshList.size(); curFrame++)
                {
                    trail = 2.0f-2.0f*curFrame/meshList.size();  // range [1, 0]
                    full = (1.0f-trailFactor)*trail;             // range [0.x, 0]
                    if (curLevel <= full)     // clamp
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal;
                    else if (curLevel > full && curLevel < trail)  // trail
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal-curLevel*(maxVal-minVal);
                    else    // clamp
                        meshList[curFrame]->FaceList[faceIndex]->scalarAttrib = minVal;
                }
            }
        }
    }
}

int MeshProcessor::depthFirstTraverse(feFace *f, int level)
{
    f->scalarAttrib = (float)level;
    f->state = 1;   // mark traversed

    int highestLevel = level, curLevel;
    vector<feFace*> neighbours = f->getNeighbours();
    for (int i=0; i<neighbours.size(); i++)
    {
        if (neighbours[i]->state == 0)    //untraversed neighbours
        {
            curLevel = depthFirstTraverse(neighbours[i], level+1);
            if (curLevel > highestLevel)
                highestLevel = curLevel;
        }
    }
    return highestLevel;
}

int MeshProcessor::breadthFirstTraverse(feFace *f)
{
    int highestLevel = 0;

    f->scalarAttrib = (float)highestLevel;
    f->state = 1;   // mark traversed

    std::queue<feFace*> q;
    q.push(f);
    feFace* curFace;

    while (!q.empty())
    {
        curFace = q.front();
        vector<feFace*> neighbours = curFace->getNeighbours();
        bool levelFlag = false;
        for (int i=0; i<neighbours.size(); i++)
        {
            if (neighbours[i]->state == 0)    //untraversed neighbours
            {
                levelFlag = true;

                neighbours[i]->scalarAttrib = curFace->scalarAttrib + 1;
                if ((int)neighbours[i]->scalarAttrib > highestLevel)
                    highestLevel = (int)neighbours[i]->scalarAttrib;
                neighbours[i]->state = 1;     // mark traversed
                q.push(neighbours[i]);
            }
        }
        q.pop();
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
