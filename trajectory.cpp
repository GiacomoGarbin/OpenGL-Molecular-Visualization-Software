#include "trajectory.h"

void Trajectory::LoadCookedData()
{
    auto path = paths.last();

    QFile file(path);
    assert(file.open(QIODevice::Text | QIODevice::ReadOnly));

    QString document = file.readAll().trimmed();
    file.close();

    atoms.clear();
    residues.clear();
    models.clear();

    auto lines = document.split("\n");

    Model model;

    int step = 0;
    int size = lines.size();
    emit ProgressBarResetSignal();
    emit ProgressBarSetMaxSignal(size);
    emit ProgressLabelSetTextSignal("loading...");

    for (auto line : lines)
    {
        if (line.startsWith("ATOM "))
        {
            auto record = line.split(" ");
            int i = 1;

            Atom atom;

            atom.number = record[i++].toInt();
            atom.name = record[i++];
            atom.element = record[i++];
            atom.residue = record[i++].toInt();
            atom.RMSDs = UnpackVectors(record[i++]);
            atom.MinRMSD = record[i++].toFloat();
            atom.MaxRMSD = record[i++].toFloat();
            atom.RMSF = record[i++].toFloat();

            atoms[atom.number] = atom;
        }

        if (line.startsWith("RESIDUE "))
        {
            auto record = line.split(" ");
            int i = 1;

            Residue residue;

            residue.number = record[i++].toInt();
            residue.chain = record[i++];
            residue.name = record[i++];
            residue.sequence = record[i++].toInt();
            residue.atoms = UnpackInts(record[i++]);
            residue.RMSDs = UnpackFloats(record[i++]);
            residue.MinRMSD = record[i++].toFloat();
            residue.MaxRMSD = record[i++].toFloat();
            residue.RMSF = record[i++].toFloat();

            residues[residue.number] = residue;
        }

        if (line.startsWith("NEW MODEL"))
        {
            model = Model();
        }

        if (line.startsWith("POSITION "))
        {
            auto record = line.split(" ");
            int i = 1;

            int AtomNumber = record[i++].toInt();
            QVector3D AtomPosition = UnpackVector(record[i++]);

            model[AtomNumber] = AtomPosition;
        }

        if (line.startsWith("END MODEL"))
        {
            models += model;
        }

        if (line.startsWith("MINMAX "))
        {
            auto record = line.split(" ");
            int i = 1;

            MinResiduesRMSD = record[i++].toFloat();
            MaxResiduesRMSD = record[i++].toFloat();
            MinResiduesRMSF = record[i++].toFloat();
            MaxResiduesRMSF = record[i++].toFloat();
            MinAtomsRMSD = record[i++].toFloat();
            MaxAtomsRMSD = record[i++].toFloat();
            MinAtomsRMSF = record[i++].toFloat();
            MaxAtomsRMSF = record[i++].toFloat();
        }

        step += 1;
        emit ProgressBarSetValueSignal(step);
    }

    QString text = QString("load %1complete").arg((step < size) ? "in" : "");
    emit ProgressLabelSetTextSignal(text);
}

QVector<ModelData> Trajectory::GetVAOsData()
{
    QVector<ModelData> VAOsData;

    for (auto model : models)
    {
        ModelData data;

        QVector3D centroid = GetCentroid(model.values().toVector());

        for (auto AtomNumber : model.keys())
        {
            Atom atom = atoms[AtomNumber];
            auto element = ChemicalElements[atom.element];

            VertexData vertex;

            vertex.center = model[AtomNumber] - centroid;
            vertex.radius = element.radius * 0.75f;
            vertex.albedo = FromQColorToQVector3D(element.albedo);

            /*
            // highlight ligand
            {
                Residue residue = residues[atom.residue];

                if (residue.chain == "L")
                {
                    // #ff007f
                    // #55ffff
                    // #ffff7f
                    // #ff557f
                    // #ff55ff

                    vertex.albedo = FromQColorToQVector3D("#55ff7f");
                }
                else
                {
                    vertex.albedo = FromQColorToQVector3D(Qt::GlobalColor::white);
                }
            }
            */

            // vertex.albedo = FromQColorToQVector3D(Qt::GlobalColor::white);

            // vertex.AtomNumber = AtomNumber;
            // vertex.AtomNumber = static_cast<unsigned int>(AtomNumber);
            // vertex.AtomNumber = static_cast<float>(AtomNumber);

            vertex.number.setX(static_cast<float>(AtomNumber));
            vertex.number.setY(static_cast<float>(atom.residue));

            data += vertex;
        }

        VAOsData += data;
    }

    return VAOsData;
}

/* -------------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const Trajectory& trajectory)
{
    QStringList attributes;
    attributes += QString("%1").arg(trajectory.atoms.size());
    attributes += QString("%1").arg(trajectory.residues.size());
    attributes += QString("%1").arg(trajectory.models.size());
    attributes += QString("%1").arg(trajectory.MinResiduesRMSD);
    attributes += QString("%1").arg(trajectory.MaxResiduesRMSD);
    attributes += QString("%1").arg(trajectory.MinResiduesRMSF);
    attributes += QString("%1").arg(trajectory.MaxResiduesRMSF);
    attributes += QString("%1").arg(trajectory.MinAtomsRMSD);
    attributes += QString("%1").arg(trajectory.MaxAtomsRMSD);
    attributes += QString("%1").arg(trajectory.MinAtomsRMSF);
    attributes += QString("%1").arg(trajectory.MaxAtomsRMSF);

    os << QString("Trajectory (%1)").arg(attributes.join(", ")).toStdString();
    return os;
}

Trajectory::Trajectory()
{
    // raw data
    paths += "../../aspirin_data_no_water/ain_trajectory_3_no_water.ag";
    paths += "../../aspirin_data_no_water/ain_trajectory_3_no_water.pdb";
    paths += "../../aspirin_data_no_water/ain_trajectory_3_no_water.alphas";
    // cooked data
    paths += "../../cooked.txt";
}

QVector<Table> Trajectory::CookPDB(QString text)
{
    QVector<Table> tables;

    auto lines = text.split("\n");

    Table table;

    QMap<QString, std::vector<int>> header;
    header["AtomSerial"] = {6, 10-6 +1};
    header["AtomName"] = {12, 15-12 +1};
    header["AlternateLocation"] = {16, +1};
    header["ResidueName"] = {17, 19-17 +1};
    header["chain"] = {21, +1};
    header["ResidueSequence"] = {22, 25-22 +1};
    header["InsertionCode"] = {26, +1};
    header["PositionX"] = {30, 37-30 +1};
    header["PositionY"] = {38, 45-38 +1};
    header["PositionZ"] = {46, 53-46 +1};
    header["occupancy"] = {54, 59-54 +1};
    header["temperature"] = {60, 65-60 +1};
    header["element"] = {76, 77-76 +1};
    header["charge"] = {78, 79-78 +1};

    for (auto line : lines)
    {
        if (line.startsWith("MODEL "))
        {
            table = Table();
        }

        if (line.startsWith("ATOM  ") || line.startsWith("HETATM"))
        {
            Record record;

            for (auto key : header.keys())
            {
                int p = header[key][0];
                int n = header[key][1];
                QString value = line.mid(p, n).trimmed();
                record[key] = value;
            }

            table += record;
        }

        if (line.startsWith("ENDMDL"))
        {
            tables += table;
        }
    }

    return tables;
}

void Trajectory::LoadRawData()
{
    for (auto path : paths.mid(0, paths.size() - 1))
    {
        QFile file(path);
        assert(file.open(QIODevice::Text | QIODevice::ReadOnly));

        QString text = file.readAll().trimmed();
        file.close();

        switch (paths.indexOf(file.fileName()))
        {
        case 0:
        {
            AtomsTable = CookCSV(text);
            break;
        }
        case 1:
        {
            ModelsTable = CookPDB(text);
            break;
        }
        case 2:
        {
            // alphas
            // ??? = CookCSV(text);
            break;
        }
        }
    }
}

void Trajectory::CookRawData()
{
    QMap<QString, int> ResidueLookupTable;

    // atoms and residues data

    for (auto record : AtomsTable)
    {
        Atom atom;

        // serial number
        atom.number = record["atom_serial"].toInt();

        // residue
        {
            QString key = record["chainID"] + record["resName"] + record["resSeq"];

            if (!ResidueLookupTable.contains(key))
            {
                Residue residue;

                residue.number = 1 + residues.size();

                residue.chain = record["chainID"][0];
                residue.name = record["resName"];
                residue.sequence = record["resSeq"].toInt();

                residues[residue.number] = residue;

                ResidueLookupTable[key] = residue.number;
            }

            int ResideuNumber = ResidueLookupTable[key];

            residues[ResideuNumber].atoms += atom.number;
            atom.residue = ResideuNumber;
        }

        // name
        atom.name = record["atom_name"];

        // element
        atom.element = record["element"];

        atoms[atom.number] = atom;
    }

    // models data

    for (auto table : ModelsTable)
    {
        Model model;
        for (auto record : table)
        {
            int AtomNumber = record["AtomSerial"].toInt();
            float x = record["PositionX"].toFloat();
            float y = record["PositionY"].toFloat();
            float z = record["PositionZ"].toFloat();
            model[AtomNumber] = {x, y, z};
        }
        models += model;
    }
}

// a : reference conformation
// b : movement conformation
// both conformations should be centered with respect to their centroid
// the returned matrix is in column-major order
Eigen::Matrix3f Trajectory::GetRotateMatrix(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b)
{
    int size = a.size();
    assert(size == b.size());

    Eigen::MatrixXf A(3, size);
    Eigen::MatrixXf B(3, size);

    for (int i = 0; i < size; i++)
    {
        A.block(0, i, 3, 1) << a[i];
        B.block(0, i, 3, 1) << b[i];
    }

    // covariance matrix
    Eigen::MatrixXf C = B * A.transpose();

    // singular value decomposition
    Eigen::JacobiSVD<Eigen::MatrixXf> SVD(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

    // left and right singular vectors
    Eigen::Matrix3f U = SVD.matrixU();
    Eigen::Matrix3f V = SVD.matrixV();

    // proper rotation test
    int d = signbit(C.determinant()) ? -1 : +1;

    // optimal rotation
    Eigen::Matrix3f D = Eigen::DiagonalMatrix<float, 3>(1, 1, d);
    Eigen::Matrix3f R = V * D * U.transpose();

    return R;
}

// a : reference conformation
// b : movement conformation
// R : optimal rotation matrix
// both conformations should be centered with respect to their centroid
float Trajectory::GetRMSD(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b, Eigen::Matrix3f R)
{
    int size = a.size();
    assert(size == b.size());

    float RMSD = 0;
    for (int i = 0; i < size; i++)
    {
        RMSD += (R * b[i] - a[i]).squaredNorm();
    }
    RMSD /= size;

    return RMSD;
}

/*
Eigen::Vector3f Trajectory::GetRMSD(QVector<Eigen::Vector3f> a, QVector<Eigen::Vector3f> b, Eigen::Matrix3f R)
{
    int size = a.size();
    assert(size == b.size());

    Eigen::Vector3f RMSD = Eigen::Vector3f::Zero();
    for (int i = 0; i < size; i++)
    {
        RMSD += R * b[i] - a[i];
    }

    return RMSD;
}
*/

float Trajectory::GetRMSF(QVector<Eigen::Vector3f> RMSDs)
{
    float RMSF = 0;
    for (auto RMSD : RMSDs)
    {
        RMSF += RMSD.squaredNorm();
    }
    RMSF /= RMSDs.size();

    return RMSF;
}

float Trajectory::GetRMSF(QVector<QVector3D> RMSDs)
{
    float RMSF = 0;
    for (auto RMSD : RMSDs)
    {
        RMSF += RMSD.lengthSquared();
    }
    RMSF /= RMSDs.size();

    return RMSF;
}

void Trajectory::CookResidues()
{
    auto FirstModel = models.first();

    // initialize residues Min and Max RMSD
    MinResiduesRMSD = MinInitValue;
    MaxResiduesRMSD = MaxInitValue;

    // initialize residues Min and Max RMSF
    MinResiduesRMSF = MinInitValue;
    MaxResiduesRMSF = MaxInitValue;

    for (auto &residue : residues)
    {
        // reference conformation : 1st model

        QVector<QVector3D> x;
        QVector<Eigen::Vector3f> a;

        // get conformation atoms positions
        for (auto AtomNumber : residue.atoms)
        {
            x += FirstModel[AtomNumber];
        }

        // get conformation centroid
        QVector3D c = GetCentroid(x);

        for (auto &v : x)
        {
            // center positions with respect to centroid
            v = v - c;
            // from QVector3D to Vector3f
            a += FromQVector3DToVector3f(v);
        }
        ConformationLookupTable[residue.number] += a;

        // get conformation rotate matrix
        Eigen::Matrix3f R = GetRotateMatrix(a, a);
        RotateMatrixLookupTable[residue.number] += R;

        // get conformation RMSD
        residue.RMSDs += GetRMSD(a, a, R);

        // initialize residue Min and Max RMSD
        residue.MinRMSD = MinInitValue; // nella ricerca del valore min per RMSD non viene considerata la RMSD della prima configurazione essendo sempre nulla
        residue.MaxRMSD = MaxInitValue;

        for (auto model : models.mid(1))
        {
            // movement conformation

            QVector<QVector3D> y;
            QVector<Eigen::Vector3f> b;

            // get conformation atoms positions
            for (auto AtomNumber : residue.atoms)
            {
                y += model[AtomNumber];
            }

            // get conformation centroid
            QVector3D c = GetCentroid(y);

            for (auto &v : y)
            {
                // center positions with respect to centroid
                v = v - c;
                // from QVector3D to Vector3f
                b += FromQVector3DToVector3f(v);
            }
            ConformationLookupTable[residue.number] += b;

            // get conformation rotate matrix
            Eigen::Matrix3f R = GetRotateMatrix(a, b);
            RotateMatrixLookupTable[residue.number] += R;

            // get conformation RMSD
            float RMSD = GetRMSD(a, b, R);
            // Eigen::Vector3f RMSD = GetRMSD(a, b, R);
            residue.RMSDs += RMSD;

            // update residue Min and Max RMSD
            residue.MinRMSD = (RMSD < residue.MinRMSD) ? RMSD : residue.MinRMSD;
            residue.MaxRMSD = (RMSD > residue.MaxRMSD) ? RMSD : residue.MaxRMSD;
            // float SquaredNorm = RMSD.squaredNorm();
            // residue.MinRMSD = (SquaredNorm < residue.MinRMSD) ? SquaredNorm : residue.MinRMSD;
            // residue.MaxRMSD = (SquaredNorm > residue.MaxRMSD) ? SquaredNorm : residue.MaxRMSD;
        }

        // get residue RMSF
        residue.RMSF = GetAverage(residue.RMSDs);
        // residue.RMSF = GetRMSF(residue.RMSDs);

        // update residues Min and Max RMSD
        MinResiduesRMSD = (residue.MinRMSD < MinResiduesRMSD) ? residue.MinRMSD : MinResiduesRMSD;
        MaxResiduesRMSD = (residue.MaxRMSD > MaxResiduesRMSD) ? residue.MaxRMSD : MaxResiduesRMSD;

        // update residues Min and Max RMSF
        MinResiduesRMSF = (residue.RMSF < MinResiduesRMSF) ? residue.RMSF : MinResiduesRMSF;
        MaxResiduesRMSF = (residue.RMSF > MaxResiduesRMSF) ? residue.RMSF : MaxResiduesRMSF;
    }
}

void Trajectory::CookAtoms()
{
    // initialize atoms Min and Max RMSD
    MinAtomsRMSD = MinInitValue;
    MaxAtomsRMSD = MaxInitValue;

    // initialize atoms Min and Max RMSF
    MinAtomsRMSF = MinInitValue;
    MaxAtomsRMSF = MaxInitValue;

    for (auto &atom : atoms)
    {
        // atom conformation index
        int AtomIndex = residues[atom.residue].atoms.indexOf(atom.number);

        // reference conformation : 1st model
        auto x = ConformationLookupTable[atom.residue].first();

        // get atom position
        Eigen::Vector3f a = x[AtomIndex];

        // get rotate matrix
        Eigen::Matrix3f R = RotateMatrixLookupTable[atom.residue][0];

        // get atom RMSD
        Eigen::Vector3f RMSD = (R * a - a);
        // atom.RMSDs += RMSD;
        atom.RMSDs += FromVector3fToQVector3D(RMSD);

        // initialize atom Min and Max RMSD
        atom.MinRMSD = MinInitValue;
        atom.MaxRMSD = MaxInitValue;

        for (int i = 1; i < models.size(); i++)
        {
            // movement conformation
            auto y = ConformationLookupTable[atom.residue][i];

            // get atom position
            Eigen::Vector3f b = y[AtomIndex];

            // get rotate matrix
            Eigen::Matrix3f R = RotateMatrixLookupTable[atom.residue][i];

            // get atom RMSD
            Eigen::Vector3f RMSD = (R * b - a);
            // atom.RMSDs += RMSD;
            atom.RMSDs += FromVector3fToQVector3D(RMSD);

            // update atom Min and Max RMSD
            float SquaredNorm = RMSD.squaredNorm();
            atom.MinRMSD = (SquaredNorm < atom.MinRMSD) ? SquaredNorm : atom.MinRMSD;
            atom.MaxRMSD = (SquaredNorm > atom.MaxRMSD) ? SquaredNorm : atom.MaxRMSD;
        }

        // get atom RMSF
        // atom.RMSF = GetAverage(atom.RMSDs);
        atom.RMSF = GetRMSF(atom.RMSDs);

        // update atoms Min and Max RMSD
        MinAtomsRMSD = (atom.MinRMSD < MinAtomsRMSD) ? atom.MinRMSD : MinAtomsRMSD;
        MaxAtomsRMSD = (atom.MaxRMSD > MaxAtomsRMSD) ? atom.MaxRMSD : MaxAtomsRMSD;

        // update atoms Min and Max RMSF
        MinAtomsRMSF = (atom.RMSF < MinAtomsRMSF) ? atom.RMSF : MinAtomsRMSF;
        MaxAtomsRMSF = (atom.RMSF > MaxAtomsRMSF) ? atom.RMSF : MaxAtomsRMSF;
    }
}

void Trajectory::SaveCookedData()
{
    auto path = paths.last();

    QFile file(path);
    assert(file.open(QIODevice::Text | QIODevice::WriteOnly));

    QTextStream stream(&file);

    // atoms
    for (auto atom : atoms)
    {
        stream << "ATOM " << atom.PackedData() << "\n";
    }

    // residues
    for (auto residue : residues)
    {
        stream << "RESIDUE " << residue.PackedData() << "\n";
    }

    // models
    for (auto model : models)
    {
        stream << "NEW MODEL\n";
        for (auto AtomNumber : model.keys())
        {
            auto AtomPosition = PackVector(model[AtomNumber]);
            stream << "POSITION " << AtomNumber << " " << AtomPosition << "\n";
        }
        stream << "END MODEL\n";
    }

    // min and max values
    {
        QVector<float> values;

        values += MinResiduesRMSD;
        values += MaxResiduesRMSD;
        values += MinResiduesRMSF;
        values += MaxResiduesRMSF;
        values += MinAtomsRMSD;
        values += MaxAtomsRMSD;
        values += MinAtomsRMSF;
        values += MaxAtomsRMSF;

        stream << "MINMAX " << PackNumbers(values, " ") << "\n";
    }

    file.close();
}

void Trajectory::ClearAllData()
{
    // raw data
    AtomsTable.clear();
    ModelsTable.clear();

    // cooked data
    atoms.clear();
    residues.clear();
    models.clear();

    // lookup tables
    ConformationLookupTable.clear();
    RotateMatrixLookupTable.clear();

    // min and max values
    MinResiduesRMSD = NAN;
    MaxResiduesRMSD = NAN;
    MinResiduesRMSF = NAN;
    MaxResiduesRMSF = NAN;
    MinAtomsRMSD = NAN;
    MaxAtomsRMSD = NAN;
    MinAtomsRMSF = NAN;
    MaxAtomsRMSF = NAN;
}
