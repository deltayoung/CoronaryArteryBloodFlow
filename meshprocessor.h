#ifndef MESHPROCESSOR_H
#define MESHPROCESSOR_H

#include <QStringList>

class MeshProcessor
{
public:
    MeshProcessor();
    ~MeshProcessor();

    void setFilenames(QStringList f);

private:
    QStringList filenames;
};

#endif // MESHPROCESSOR_H
