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
}
