#ifndef MESHPROCESSOR_H
#define MESHPROCESSOR_H

#include <QStringList>

// headers from MeshProcToolkit
#include "Filters.h"
#include "cPoint.h"

class MeshProcessor
{
public:
    MeshProcessor();
    ~MeshProcessor();

    void setFilenames(QStringList f);
    void loadFilesToMeshes(QStringList f);
    void findMeshesBoundary();
    void traversePolygonsOntoMeshes();

    std::vector<feMesh*> meshList;
    cPoint cornerMin, cornerMax;

private:
    QStringList filenames;

    int recursiveTraverse(feFace*, int);
    bool foundNewSeed(int &);

};

#endif // MESHPROCESSOR_H
