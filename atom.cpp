#include "atom.h"

Atom::Atom()
{

}

std::ostream& operator<<(std::ostream& os, const Atom& atom)
{
    QStringList attributes;
    attributes += QString("%1").arg(atom.name);
    attributes += QString("%1").arg(atom.element);
    attributes += QString("%1").arg(atom.residue);
    attributes += QString("%1").arg(atom.RMSDs.size());
    attributes += QString("%1").arg(atom.MinRMSD);
    attributes += QString("%1").arg(atom.MaxRMSD);
    attributes += QString("%1").arg(atom.RMSF);

    os << QString("Atom #%1: %2").arg(atom.number).arg(attributes.join(", ")).toStdString();
    return os;
}

QString Atom::PackedData()
{
    QStringList list;

    list += QString::number(number);
    list += name;
    list += element;
    list += QString::number(residue);
    list += PackVectors(RMSDs);
    list += QString::number(MinRMSD);
    list += QString::number(MaxRMSD);
    list += QString::number(RMSF);

    return list.join(" ");
}
