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

    //void setFilenames(QStringList f);
    void loadFilesToMeshes(QStringList f);
    void findMeshesBoundary();
    void traversePolygonsOntoMeshesAllObjects();

    vector<feMesh*> meshList;
    vector<vector<feMesh*>> secondaryMeshLists; // allow multiple objects if there are more than 1
    cPoint cornerMin, cornerMax;

private:
    //QStringList filenames;

    int depthFirstTraverse(feFace*, int);
    int breadthFirstTraverse(feFace*);
    bool foundNewSeed(int &, vector<feMesh*>);
    void traversePolygonsOntoMeshes(vector<feMesh*>);

};

#endif // MESHPROCESSOR_H
