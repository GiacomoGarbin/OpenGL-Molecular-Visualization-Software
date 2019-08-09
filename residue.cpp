#include "residue.h"

Residue::Residue()
{

}

std::ostream& operator<<(std::ostream& os, const Residue& residue)
{
    QStringList attributes;
    attributes += QString("%1").arg(residue.chain);
    attributes += QString("%1").arg(residue.name);
    attributes += QString("%1").arg(residue.sequence);
    attributes += QString("%1").arg(residue.atoms.size());
    attributes += QString("%1").arg(residue.RMSDs.size());
    attributes += QString("%1").arg(residue.MinRMSD);
    attributes += QString("%1").arg(residue.MaxRMSD);
    attributes += QString("%1").arg(residue.RMSF);

    os << QString("Residue #%1 (%2)").arg(residue.number).arg(attributes.join(", ")).toStdString();
    return os;
}

QString Residue::PackedData()
{
    QStringList list;

    list += QString::number(number);
    list += chain;
    list += name;
    list += QString::number(sequence);
    list += PackNumbers(atoms);
    // list += PackVectors(RMSDs);
    list += PackNumbers(RMSDs);
    list += QString::number(MinRMSD);
    list += QString::number(MaxRMSD);
    list += QString::number(RMSF);

    return list.join(" ");
}
