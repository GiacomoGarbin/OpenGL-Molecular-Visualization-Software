#include "residueswindow.h"
#include "ui_residueswindow.h"

ResiduesWindow::ResiduesWindow(QWidget *parent) : QWidget(parent), ui(new Ui::ResiduesWindow)
{
    ui->setupUi(this);
}

ResiduesWindow::~ResiduesWindow()
{
    delete ui;
}

void ResiduesWindow::InitUI()
{
    qDebug() << "ResiduesWindow::InitUI()";

    auto layout = new QGridLayout();

    // qDebug() << this << this->layout();
    // qDebug() << this << this->layout();

    int r = 0;
    int c = 0;

    for (auto residue : residues)
    {
        auto label = new QLabel(QString::number(residue.number));
        layout->addWidget(label, r, c);

        r = (r + 1) % 11;

        if (r == 0)
        {
            c = (c + 1) % 11;
        }
    }

    setLayout(layout);


}
