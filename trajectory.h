#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <iostream>
#include <QDebug>
#include <QFile>
#include <Eigen/Dense>

#include "atom.h"
#include "residue.h"
#include "utility.h"
using namespace utility;

class Trajectory : public QObject
{
    Q_OBJECT

public:
    Trajectory();

    // map : atom -> position
    typedef QMap<int, QVector3D> Model;

    void LoadCookedData();

    // cooked data

    QMap<int, Atom> atoms;
    QMap<int, Residue> residues;
    QVector<Model> models;

    // Min and Max residues RMSD
    float MinResiduesRMSD;
    float MaxResiduesRMSD;
    // Min and Max residues RMSF
    float MinResiduesRMSF;
    float MaxResiduesRMSF;

    // Min and Max atoms RMSD
    float MinAtomsRMSD;
    float MaxAtomsRMSD;
    // Min and Max atoms RMSF
    float MinAtomsRMSF;
    float MaxAtomsRMSF;

    QVector<ModelData> GetVAOsData();

private:
    QVector<QString> paths;

    // raw data
    Table AtomsTable;
    QVector<Table> ModelsTable;


    QVector<Table> CookPDB(QString text);
    void LoadRawData();
    void CookRawData();

    QMap<int, QVector<QVector<Eigen::Vector3f>>> ConformationLookupTable;
    QMap<int, QVector<Eigen::Matrix3f>> RotateMatrixLookupTable;

    Eigen::Matrix3f GetRotateMatrix(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b);
    float GetRMSD(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b, Eigen::Matrix3f R);
    // Eigen::Vector3f GetRMSD(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b, Eigen::Matrix3f R);
    float GetRMSF(QVector<Eigen::Vector3f> RMSDs);
    float GetRMSF(QVector<QVector3D> RMSDs);

    // in the search for the minimum value for RMSD (both for atoms and residues),
    // it is important to exclude the first configuration (if this is
    // the reference configuration), which always has a null value

    const float MinInitValue = FLT_MAX;
    const float MaxInitValue = 0; // FLT_MIN

    void CookResidues();
    void CookAtoms();

    void SaveCookedData();

    void ClearAllData();

    friend std::ostream& operator<<(std::ostream& os, const Trajectory& trajectory);

signals:
    void ProgressBarSetMaxSignal(int value);
    void ProgressBarResetSignal();
    void ProgressBarSetValueSignal(int value);
    void ProgressLabelSetTextSignal(QString text);
};

#endif // TRAJECTORY_H
