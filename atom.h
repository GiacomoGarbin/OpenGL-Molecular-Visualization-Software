#ifndef ATOM_H
#define ATOM_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVector3D>

#include <Eigen/Dense>

#include "utility.h"
using namespace utility;

class Atom
{
public:
    Atom();

    int number;

    QString name;
    QString element;

    int residue;

    // QVector<Eigen::Vector3f> RMSDs;
    QVector<QVector3D> RMSDs;
    float MinRMSD;
    float MaxRMSD;

    float RMSF;

    QString PackedData();

    friend std::ostream& operator<<(std::ostream& os, const Atom& atom);
};

#endif // ATOM_H
