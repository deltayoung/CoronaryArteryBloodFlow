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
