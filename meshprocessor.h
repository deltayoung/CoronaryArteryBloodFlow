#ifndef MESHPROCESSOR_H
#define MESHPROCESSOR_H

#include <QStringList>

// headers from MeshProcToolkit
#include "Filters.h"

class MeshProcessor
{
public:
    MeshProcessor();
    ~MeshProcessor();

    void setFilenames(QStringList f);
    void loadFilesToMeshes(QStringList f);

    std::vector<feMesh*> meshList;

private:
    QStringList filenames;



};

#endif // MESHPROCESSOR_H
