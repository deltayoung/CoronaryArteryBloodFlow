#include "meshprocessor.h"

#include <queue>

MeshProcessor::MeshProcessor()
{

}

MeshProcessor::~MeshProcessor()
{

}

/*void MeshProcessor::setFilenames(QStringList f)
{
    filenames = f;
}*/

void MeshProcessor::loadFilesToMeshes(QStringList f)
{
    //setFilenames(f);
    feMesh* newMesh;

    if (meshList.size() == 0)   // first object
    {
        for (int i=0; i<f.size(); i++)
        {
            newMesh = new feMesh();
            readVTK(f.at(i).toLocal8Bit().data(), newMesh);
            meshList.push_back(newMesh);
        }
    }
    else
    {
        vector<feMesh*> secMeshList;
        for (int i=0; i<f.size(); i++)
        {
            newMesh = new feMesh();
            readVTK(f.at(i).toLocal8Bit().data(), newMesh);
            secMeshList.push_back(newMesh);
        }
        secondaryMeshLists.push_back(secMeshList);
    }

    //findMeshesBoundary();
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
    //Disable boundary checking of secondary objects to make the main object the focus of the scene
    /*for (int n=0; n<secondaryMeshLists.size(); n++)
    {
        for (int i=0; i<secondaryMeshLists[n].size(); i++)
            for (int j=0; j<secondaryMeshList[n][i]->NodeList.size(); j++)
            {
                if (cornerMin.x > secondaryMeshLists[n][i]->NodeList[j]->xyz[0])
                    cornerMin.x = secondaryMeshLists[n][i]->NodeList[j]->xyz[0];
                if (cornerMin.y > secondaryMeshLists[n][i]->NodeList[j]->xyz[1])
                    cornerMin.y = secondaryMeshLists[n][i]->NodeList[j]->xyz[1];
                if (cornerMin.z > secondaryMeshLists[n][i]->NodeList[j]->xyz[2])
                    cornerMin.z = secondaryMeshLists[n][i]->NodeList[j]->xyz[2];

                if (cornerMax.x < secondaryMeshLists[n][i]->NodeList[j]->xyz[0])
                    cornerMax.x = secondaryMeshLists[n][i]->NodeList[j]->xyz[0];
                if (cornerMax.y < secondaryMeshLists[n][i]->NodeList[j]->xyz[1])
                    cornerMax.y = secondaryMeshLists[n][i]->NodeList[j]->xyz[1];
                if (cornerMax.z < secondaryMeshLists[n][i]->NodeList[j]->xyz[2])
                    cornerMax.z = secondaryMeshLists[n][i]->NodeList[j]->xyz[2];
            }
    }*/

    cVector maxLengthVector = cornerMax - cornerMin;
    maxLength = maxLengthVector.length();
}

void MeshProcessor::traversePolygonsOntoMeshesAllObjects()
{
    traversePolygonsOntoMeshes(meshList);
    for (int i=0; i<secondaryMeshLists.size(); i++)
        traversePolygonsOntoMeshes(secondaryMeshLists[i]);
}

void MeshProcessor::traversePolygonsOntoMeshes(vector<feMesh*> aList)
{
    //Objective: assuming all the meshes form an ordered sequence of frames for animation (1-to-1 mapping for all meshes), mark each frame as we traverse from the topmost point of the mesh at the first frame towards the bottom at the last frame
    //Strategy: find a point with the maximum Z value as the seed for the initial traversion vertically downwards
    //Subsequently, repeat the above for the pool of remaining untraversed points, until all points are traversed
    //Face states: 0 = untraversed, 1 = traversed, 2 = processed

    int seedIndex = -1;
    int levelCount=0;
    //float curVal = 0.0f;
    // search for the seed, i.e., the point with maximum Z value
    while (foundNewSeed(seedIndex, aList))
    {
        //levelCount = depthFirstTraverse(aList[0]->FaceList[seedIndex], 0); // maximum: 2log(n-2), with n=number of faces per mesh
        levelCount = breadthFirstTraverse(aList[0]->FaceList[seedIndex]);    // BFT has better spread/flow than DFT
        cout << "DebugPt, levelCount=" << levelCount << "\n";
        int halfSize = aList.size()/2;
        if (levelCount == 0 && aList[0]->FaceList[seedIndex]->state == 1)    // exceptional branch: single unconnected face
        {
            aList[0]->FaceList[seedIndex]->state = 2; // processed
            for (int curFrame=0; curFrame<=halfSize; curFrame++)
                aList[curFrame]->FaceList[seedIndex]->scalarAttrib = curFrame/halfSize;
            for (int curFrame=aList.size()-1; curFrame>halfSize; curFrame--)
                aList[curFrame]->FaceList[seedIndex]->scalarAttrib = (1-curFrame+halfSize)/halfSize;
            continue;
        }

        float trailFactor = 0.3f, trail, head, minVal = 0.2f, maxVal = 1.0f, avgVal = 0.8f;
        for (int faceIndex=0; faceIndex<aList[0]->FaceList.size(); faceIndex++)
        {
            if (aList[0]->FaceList[faceIndex]->state == 1)    // traversed faces only
            {
                aList[0]->FaceList[faceIndex]->state = 2; // processed
                float curLevel = aList[0]->FaceList[faceIndex]->scalarAttrib/levelCount;
                for (int curFrame=0; curFrame<=halfSize; curFrame++)
                {   
                    head = (float)curFrame/halfSize;
                    trail = (1.0f+trailFactor)*head;

                    if (curLevel <= head)   // flow front
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = avgVal+(curLevel/head)*(maxVal-avgVal);
                    else if (curLevel > head && curLevel < trail)      // trail
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal-(curLevel-head)/(trail-head)*(maxVal-minVal);
                    else    // clamp
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = minVal;
                }
                for (int curFrame=halfSize+1; curFrame<aList.size(); curFrame++)
                {
                    head = 2.0f-2.0f*curFrame/aList.size();
                    trail = (1.0f+trailFactor)*head;
                    if (curLevel <= head)     // clamp
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = avgVal+(curLevel/head)*(maxVal-avgVal);
                    else if (curLevel > head && curLevel < trail)  // trail
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = maxVal-(curLevel-head)/(trail-head)*(maxVal-minVal);
                    else    // clamp
                        aList[curFrame]->FaceList[faceIndex]->scalarAttrib = minVal;
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

bool MeshProcessor::foundNewSeed(int &seedIndex, vector<feMesh*> aList)
{
    float maxZ;
    bool found = false;
    for (int i=0; i<aList[0]->NodeList.size(); i++)
    {
        for (list<feFace*>::const_iterator j=aList[0]->NodeList[i]->pFace.begin(); j!=aList[0]->NodeList[i]->pFace.end(); ++j)
        {
            if ((*j)->state == 0)   // untraversed
            {
                maxZ = aList[0]->NodeList[i]->xyz[2];    // initial maximum Z value
            }
        }
    }
    for (int i=0; i<aList[0]->FaceList.size(); i++)
    {
        if (aList[0]->FaceList[i]->state == 0)   // untraversed
        {
            for (int j=0; j<3; j++)
            {
                if (aList[0]->FaceList[i]->pNode[j]->xyz[2] > maxZ)
                {
                    maxZ = aList[0]->FaceList[i]->pNode[j]->xyz[2];
                    seedIndex = i;
                    found = true;
                }
            }
        }
    }
    return found;
}
