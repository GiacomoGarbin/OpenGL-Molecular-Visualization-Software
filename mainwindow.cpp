#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* -------------------------------------------------------------------------------- */

    gl = ui->OpenGL;

    InitUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitUI()
{
    // MouseHover();

    AtomGroup();
    ResidueGroup();
    TrajectoryGroup();

    OutlineGroup();
    // progress();
    playback();
    framerate();

    SSAOTab();
    LightTab();
    MaterialTab();
    ColorLerp();

    ResiduesWindowInit();

    auto widgets = ui->CentralWidget->findChildren<QWidget*>();
    for (auto widget : widgets)
    {
        widget->setFont(QFont("Consolas"));
    }

    ui->AtomGroup->hide();
    ui->ResidueGroup->hide();
    ui->LerpTab->hide();

    QVector<QString> names;

    for (auto residue : gl->trajectory.residues)
    {
        if (!names.contains(residue.name))
        {
            names += residue.name;
        }
    }

    qDebug() << names;
}

/*
void MainWindow::progress()
{
    auto label = ui->ProgressLabel;

    // set text signal
    {
        auto lambda = [=] (QString text)
        {
            label->setText(text);
        };
        connect(&gl->trajectory, &Trajectory::ProgressLabelSetTextSignal, this, lambda);
    }

    auto progressbar = ui->ProgressBar;

    // set max signal
    {
        auto lambda = [=] (int value)
        {
            progressbar->setMaximum(value);
        };
        connect(&gl->trajectory, &Trajectory::ProgressBarSetMaxSignal, this, lambda);
    }

    // set value signal
    {
        auto lambda = [=] (int value)
        {
            progressbar->setValue(value);
        };
        connect(&gl->trajectory, &Trajectory::ProgressBarSetValueSignal, this, lambda);
    }

    // reset signal
    {
        auto lambda = [=] ()
        {
            progressbar->reset();
        };
        connect(&gl->trajectory, &Trajectory::ProgressBarResetSignal, this, lambda);
    }

    auto button = ui->LoadData;

    {
        auto lambda = [=] ()
        {
            gl->trajectory.LoadCookedData();
        };
        connect(button, &QPushButton::clicked, lambda);
    }
}
*/

void MainWindow::AtomGroup()
{
    // mouse hover signal
    {
        auto lambda = [=] (Atom atom)
        {
            QVector<QLabel*> labels;
            labels += ui->AtomName;
            labels += ui->AtomElement;
            labels += ui->AtomRMSDSquaredNorm;
            labels += ui->AtomRMSDDirection;
            labels += ui->AtomMinRMSD;
            labels += ui->AtomMaxRMSD;
            labels += ui->AtomRMSF;

            auto element = ChemicalElements[atom.element];
            auto RMSD = atom.RMSDs[gl->playback.step];

            int precision = 7;
            int width = precision + 3;

            QVector<QString> values;
            values += atom.name;
            values += QString("%1 (%2)").arg(element.symbol).arg(element.name);
            values += FormatNumber(RMSD.lengthSquared(), width, precision);
            values += FormatVector(RMSD.normalized());
            values += FormatNumber(atom.MinRMSD, width, precision);
            values += FormatNumber(atom.MaxRMSD, width, precision);
            values += FormatNumber(atom.RMSF, width, precision);

            while (labels.size() > 0)
            {
                auto label = labels.takeFirst();
                auto value = values.takeFirst();

                QString caption = label->text().split(" :").first();
                label->setText(QString("%1 : %2").arg(caption).arg(value));
            }
        };
        connect(gl, &OpenGLWidget::MouseHoverSignal, this, lambda);
    }

    // mouse not hover signal
    {
        auto lambda = [=] ()
        {
            // auto labels = ui->AtomGroup->findChildren<QLabel*>();
            auto labels = ui->AtomTab->findChildren<QLabel*>();

            for (auto label : labels)
            {
                if (label->objectName().endsWith("Title"))
                {
                    continue;
                }

                if (label->objectName() == "AtomRMSDDirection")
                {
                    QString text = label->text().split(" :").first();
                    label->setText(text + " : (     ,     ,     )");
                    continue;
                }

                QString text = label->text().split(" :").first();
                label->setText(text + " :");
            }
        };
        connect(gl, &OpenGLWidget::MouseNotHoverSignal, this, lambda);
    }
}

void MainWindow::ResidueGroup()
{
    // mouse hover signal
    {
        auto lambda = [=] (Atom atom)
        {
            QVector<QLabel*> labels;
            labels += ui->ResidueChain;
            labels += ui->ResidueName;
            labels += ui->ResidueSequence;
            labels += ui->ResidueAtoms;
            labels += ui->ResidueRMSDSquaredNorm;
            labels += ui->ResidueMinRMSD;
            labels += ui->ResidueMaxRMSD;
            labels += ui->ResidueRMSF;

            auto residue = gl->trajectory.residues[atom.residue];

            auto aminoacid = AminoAcids[residue.name];
            auto RMSD = residue.RMSDs[gl->playback.step];

            int precision = 7;
            int width = precision + 3;

            QVector<QString> values;
            values += residue.chain;
            values += QString("%1 (%2)").arg(aminoacid.symbol).arg(aminoacid.name);
            values += QString::number(residue.sequence);
            values += QString::number(residue.atoms.size());
            values += FormatNumber(RMSD, width, precision);
            values += FormatNumber(residue.MinRMSD, width, precision);
            values += FormatNumber(residue.MaxRMSD, width, precision);
            values += FormatNumber(residue.RMSF, width, precision);

            while (labels.size() > 0)
            {
                auto label = labels.takeFirst();
                auto value = values.takeFirst();

                QString caption = label->text().split(" :").first();
                label->setText(QString("%1 : %2").arg(caption).arg(value));
            }
        };
        connect(gl, &OpenGLWidget::MouseHoverSignal, this, lambda);
    }

    // mouse not hover signal
    {
        auto lambda = [=] ()
        {
            // auto labels = ui->ResidueGroup->findChildren<QLabel*>();
            auto labels = ui->ResidueTab->findChildren<QLabel*>();

            for (auto label : labels)
            {
                if (label->objectName().endsWith("Title"))
                {
                    continue;
                }

                QString text = label->text().split(" :").first();
                label->setText(text + " :");
            }
        };
        connect(gl, &OpenGLWidget::MouseNotHoverSignal, this, lambda);
    }
}

void MainWindow::TrajectoryGroup()
{
    QVector<QLabel*> labels;
    labels += ui->AbsResidueMinRMSF;
    labels += ui->AbsResidueMaxRMSF;
    labels += ui->AbsResidueMinRMSD;
    labels += ui->AbsResidueMaxRMSD;
    labels += ui->AbsAtomMinRMSF;
    labels += ui->AbsAtomMaxRMSF;
    labels += ui->AbsAtomMinRMSD;
    labels += ui->AbsAtomMaxRMSD;

    QVector<QString> values;
    values += FormatNumber(gl->trajectory.MinResiduesRMSF);
    values += FormatNumber(gl->trajectory.MaxResiduesRMSF);
    values += FormatNumber(gl->trajectory.MinResiduesRMSD);
    values += FormatNumber(gl->trajectory.MaxResiduesRMSD);
    values += FormatNumber(gl->trajectory.MinAtomsRMSF);
    values += FormatNumber(gl->trajectory.MaxAtomsRMSF);
    values += FormatNumber(gl->trajectory.MinAtomsRMSD);
    values += FormatNumber(gl->trajectory.MaxAtomsRMSD);

    while (labels.size() > 0)
    {
        auto label = labels.takeFirst();
        auto value = values.takeFirst();
        label->setText(value);
    }
}

void MainWindow::OutlineGroup()
{
    // outline checkbox
    {
        auto group = ui->OutlineGroup;
        group->setChecked(gl->outline.active);

        auto lambda = [=] (bool on)
        {
            // update value
            gl->outline.active = (on == true);
        };
        connect(group, &QGroupBox::toggled, lambda);
    }

    // mode ButtonGroup
    {
        auto group = new QButtonGroup();
        group->setObjectName("OutlineModeGroup");

        // Residue RMSF Button
        {
            auto button = ui->ResidueRMSFButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.mode = OutlineMode::RESIDUE_RMSF;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }

        // Residue RMSD Button
        {
            auto button = ui->ResidueRMSDButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.mode = OutlineMode::RESIDUE_RMSD;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }

        // Atom RMSF Button
        {
            auto button = ui->AtomRMSFButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.mode = OutlineMode::ATOM_RMSF;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }

        // Atom RMSD Button
        {
            auto button = ui->AtomRMSDButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.mode = OutlineMode::ATOM_RMSD;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }
    }
    ui->ModeLayout->parentWidget()->hide();

    // mode
    {
        auto combobox = ui->OutlineModeComboBox;

        QMap<QString, OutlineMode> modes;
        modes["Residue RMSF"] = OutlineMode::RESIDUE_RMSF;
        modes["Residue RMSD"] = OutlineMode::RESIDUE_RMSD;
        modes["Atom RMSF"] = OutlineMode::ATOM_RMSF;
        modes["Atom RMSD"] = OutlineMode::ATOM_RMSD;

        combobox->setCurrentText(modes.key(gl->outline.mode));

        auto lambda = [=] (const QString &text)
        {
            gl->outline.mode = modes[text];
            gl->SetOutlineColor();
        };
        connect(combobox, &QComboBox::currentTextChanged, lambda);
    }

    // graysclae
    {
        auto checkbox = ui->GrayscaleCheckbox;

        auto lambda = [=] (int state)
        {
            // update value
            gl->outline.grayscale = (state == Qt::Checked);
        };
        connect(checkbox, &QCheckBox::stateChanged, lambda);
    }

    // Boundary Values ButtonGroup
    {
        auto group = new QButtonGroup();
        group->setObjectName("BoundaryValuesGroup");

        // Absolute Boundary Button
        {
            auto button = ui->AbsoluteBoundaryButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.boundary = BoundaryValues::absolute;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }

        // Relative Boundary Button
        {
            auto button = ui->RelativeBoundaryButton;
            group->addButton(button);

            auto lambda = [=] ()
            {
                gl->outline.boundary = BoundaryValues::relative;
                gl->SetOutlineColor();
            };
            connect(button, &QPushButton::clicked, lambda);
        }
    }
    ui->BoundaryLayout->parentWidget()->hide();

    // boundary values
    {
        auto combobox = ui->OutlineBoundaryComboBox;

        QMap<QString, BoundaryValues> options;
        options["absolute"] = BoundaryValues::absolute;
        options["relative"] = BoundaryValues::relative;

        combobox->setCurrentText(options.key(gl->outline.boundary));

        auto lambda = [=] (const QString &text)
        {
            gl->outline.boundary = options[text];
            gl->SetOutlineColor();
        };
        connect(combobox, &QComboBox::currentTextChanged, lambda);
    }

    // Thickness Slider
    {
        auto slider = ui->ThicknessSlider;

        int min = 2;
        slider->setMinimum(min);
        slider->setMaximum(min + 4);

        // set value
        slider->setSliderPosition(gl->outline.thickness);

        // value changed event
        {
            auto lambda = [=] (int value)
            {
                // update value
                gl->outline.thickness = value;
            };
            connect(slider, &QSlider::valueChanged, lambda);
        }
    }

    // filter value slider
    {
        auto slider = ui->OutlineFilterValue;

        slider->setMinimum(0);
        slider->setMaximum(gl->outline.size);

        slider->setSliderPosition(0);

        // value changed event
        {
            auto lambda = [=] (int value)
            {
                gl->outline.filter = value;
                gl->SetOutlineColor();
            };
            connect(slider, &QSlider::valueChanged, lambda);
        }

        // ColorSchemeSignal
        {
            auto lambda = [=] (int value)
            {
                slider->setMaximum(gl->outline.size);
            };
            connect(gl, &OpenGLWidget::ColorSchemeSignal, this, lambda);
        }
    }

    // color scheme : palette
    {
        auto combobox = ui->PaletteComboBox;

        for (auto palette : ColorPalettes)
        {
            combobox->addItem(palette.name);
        }

        combobox->setCurrentText(gl->outline.palette.name);

        auto lambda = [=] (const QString &text)
        {
            auto lambda = [=] (ColorPalette palette) { return (palette.name == text); };
            auto palette = *std::find_if(ColorPalettes.begin(), ColorPalettes.end(), lambda);

            gl->outline.palette = palette;
            gl->SetOutlineColor();
        };
        connect(combobox, &QComboBox::currentTextChanged, lambda);
    }

    // color scheme : size
    {
        auto combobox = ui->ColorSchemeSizeComboBox;

        for (int i = 3; i <= 7; i++)
        {
            combobox->addItem(QString::number(i));
        }

        combobox->setCurrentText(QString::number(gl->outline.size));

        auto lambda = [=] (const QString &text)
        {
            gl->outline.size = text.toInt();
            gl->SetOutlineColor();
        };
        connect(combobox, &QComboBox::currentTextChanged, lambda);
    }

    // color scheme : caption
    {
        class ColorStepLabel : public QLabel
        {
        public:
            ColorStepLabel(ColorStep step, QLabel* label, OutlineData outline)
            {
                this->step = step;
                this->label = label;
                // this->boundary = boundary;
                this->outline = outline;
            }

        private:
            ColorStep step;
            QLabel* label;
            // BoundaryValues boundary;
            OutlineData outline;

        protected:
            bool event(QEvent *e)
            {
                // if mode is residue/atom RMSD and boundary values are relative,
                // the steps of the color scheme change for each residue/atom

                QVector<bool> flags;
                flags += (outline.mode == RESIDUE_RMSF);
                flags += (outline.mode == ATOM_RMSF);
                flags += (outline.boundary == absolute);

                if (std::any_of(flags.begin(), flags.end(), [] (bool flag) { return flag; }))
                {
                    switch (e->type())
                    {
                    case QEvent::Enter:
                    {
                        /*
                        QStringList text;

                        int precision = 5;
                        int width = precision + 3;

                        if (step.min > 0)
                        {
                            text += QString("%1 <=").arg(FormatNumber(step.min, width, precision));
                        }

                        text += "x";

                        if (step.max < FLT_MAX)
                        {
                            text += QString("< %2").arg(FormatNumber(step.max, width, precision));
                        }

                        label->setText(text.join(" "));
                        */
                        label->setText(GetColorStepCaption(step));
                        break;
                    }
                    case QEvent::Leave:
                    {
                        label->setText(QString());
                        break;
                    }
                    default:
                    {
                        break;
                    }
                    }
                }

                return false;
            }
        };

        auto UpdateLayout = [=] (bool flag = false)
        {
            // auto scheme = gl->outline.scheme;
            // auto scheme = gl->outline.schemes.first();
            auto scheme = gl->outline.schemes.last();

            /*
            // update tab
            {
                auto tab = ui->ColorSchemeTab;
                auto OldLayout = tab->layout();
                // qDebug() << OldLayout;

                // clear OldLayout
                QLayoutItem *child;
                while ((child = OldLayout->takeAt(0)) != nullptr)
                {
                    delete child->widget();
                    delete child;
                }
                delete OldLayout;

                auto NewLayout = new QGridLayout();
                NewLayout->setSpacing(0);

                for (int i = 0; i < scheme.size(); i++)
                {
                    auto step = scheme[i];

                    // color
                    {
                        auto label = new QLabel();

                        QStringList rules;
                        rules += QString("width: 50px");
                        rules += QString("background-color: %1").arg(step.color.name());
                        rules += QString("border-style: solid");
                        rules += QString("border-width: 1px");
                        rules += QString("border-color: #000000");
                        if (i + 1 < scheme.size())
                        {
                            rules += QString("border-bottom: none");
                        }
                        label->setStyleSheet(rules.join("; "));

                        NewLayout->addWidget(label, i, 0);
                    }

                    // caption
                    {
                        auto label = new QLabel();
                        label->setFont(QFont("Consolas"));

                        QString text = GetColorStepCaption(step);

                        label->setText((i == 0) ? text.rightJustified(8 + 4 + 1 + 3 + 8) : text);

                        NewLayout->addWidget(label, i, 1);
                    }
                }

                tab->setLayout(NewLayout);
                // qDebug() << tab->layout();

                for (int i = 0; i < NewLayout->count(); i++)
                {
                    NewLayout->itemAt(i)->widget()->show();
                }
            }
            */

            if (flag)
            {
                return;
            }

            // update layout

            auto widget = ui->ColorSchemePreview;
            auto OldLayout = widget->layout();

            // clear OldLayout
            QLayoutItem *child;
            while ((child = OldLayout->takeAt(0)) != nullptr)
            {
                delete child->widget();
                delete child;
            }
            delete OldLayout;

            auto NewLayout = new QHBoxLayout();
            NewLayout->setContentsMargins(6, 0, 6, 0);
            NewLayout->setSpacing(0);

            for (int i = 0; i < scheme.size(); i++)
            {
                auto step = scheme[i];

                auto label = new ColorStepLabel(step, ui->ColorSchemePreviewLabel, gl->outline);
                label->setObjectName(QString("ColorStepLabel%1").arg(i));

                QStringList rules;
                rules += QString("background-color: %1").arg(step.color.name());
                rules += QString("border-style: solid");
                rules += QString("border-width: 1px");
                rules += QString("border-color: #000000");
                if (i + 1 < scheme.size())
                {
                    rules += QString("border-right: none");
                }
                label->setStyleSheet(rules.join("; "));

                NewLayout->addWidget(label);
            }

            widget->setLayout(NewLayout);
            // qDebug() << widget->layout();

            for (int i = 0; i < NewLayout->count(); i++)
            {
                NewLayout->itemAt(i)->widget()->show();
            }

            // qDebug() << NewLayout->contentsMargins() << NewLayout->spacing();
        };

        UpdateLayout();

        // Color Scheme Signal

        connect(gl, &OpenGLWidget::ColorSchemeSignal, this, UpdateLayout);
    }

    // color scheme : tab
    {
        // mouse hover
        {
            auto lambda = [=] (Atom atom)
            {
                int number;

                if (gl->outline.schemes.size() == 1)
                {
                    number = 0;
                }
                else
                {
                    switch (gl->outline.mode)
                    {
                    case RESIDUE_RMSF:
                    case RESIDUE_RMSD:
                    {
                        number = atom.residue;
                        break;
                    }
                    case ATOM_RMSF:
                    case ATOM_RMSD:
                    {
                        number = atom.number;
                        break;
                    }
                    }
                }

                auto scheme = gl->outline.schemes[number];

                auto tab = ui->ColorSchemeTab;
                auto OldLayout = tab->layout();
                // qDebug() << OldLayout;


                if (OldLayout == nullptr)
                {
                    qDebug() << "NULL PTR HERE!!";
                }


                // clear OldLayout
                QLayoutItem *child;
                while ((child = OldLayout->takeAt(0)) != nullptr)
                {
                    delete child->widget();
                    delete child;
                }
                delete OldLayout;

                auto NewLayout = new QGridLayout();
                NewLayout->setSpacing(0);

                for (int i = 0; i < scheme.size(); i++)
                {
                    auto step = scheme[i];

                    // color
                    {
                        auto label = new QLabel();

                        QStringList rules;
                        rules += QString("background-color: %1").arg(step.color.name());
                        rules += QString("border-style: solid");
                        rules += QString("border-width: 1px");
                        rules += QString("border-color: #000000");
                        if (i + 1 < scheme.size())
                        {
                            rules += QString("border-bottom: none");
                        }
                        label->setStyleSheet(rules.join("; "));

                        NewLayout->addWidget(label, i, 0);
                    }

                    // caption
                    {
                        auto label = new QLabel();
                        label->setFont(QFont("Consolas"));

                        QString text = GetColorStepCaption(step);
                        label->setText((i == 0) ? text.rightJustified(8 + 4 + 1 + 3 + 8) : text);

                        NewLayout->addWidget(label, i, 1);
                    }
                }

                tab->setLayout(NewLayout);
                // qDebug() << tab->layout();

                for (int i = 0; i < NewLayout->count(); i++)
                {
                    NewLayout->itemAt(i)->widget()->show();
                }
            };
            connect(gl, &OpenGLWidget::MouseHoverSignal, this, lambda);
        }

        // mouse hot hover
        {
            auto lambda = [=] ()
            {
                auto scheme = gl->outline.schemes.last();

                auto tab = ui->ColorSchemeTab;
                auto OldLayout = tab->layout();
                // qDebug() << OldLayout;

                // clear OldLayout
                QLayoutItem *child;
                while ((child = OldLayout->takeAt(0)) != nullptr)
                {
                    delete child->widget();
                    delete child;
                }
                delete OldLayout;

                auto NewLayout = new QGridLayout();
                NewLayout->setSpacing(0);

                for (int i = 0; i < scheme.size(); i++)
                {
                    auto step = scheme[i];

                    // color
                    {
                        auto label = new QLabel();

                        QStringList rules;
                        rules += QString("background-color: %1").arg(step.color.name());
                        rules += QString("border-style: solid");
                        rules += QString("border-width: 1px");
                        rules += QString("border-color: #000000");
                        if (i + 1 < scheme.size())
                        {
                            rules += QString("border-bottom: none");
                        }
                        label->setStyleSheet(rules.join("; "));

                        NewLayout->addWidget(label, i, 0);
                    }

                    // caption
                    {
                        auto label = new QLabel();
                        label->setFont(QFont("Consolas"));

                        label->setText(QString().rightJustified(8 + 4 + 1 + 3 + 8));

                        NewLayout->addWidget(label, i, 1);
                    }
                }

                tab->setLayout(NewLayout);
                // qDebug() << tab->layout();

                for (int i = 0; i < NewLayout->count(); i++)
                {
                    NewLayout->itemAt(i)->widget()->show();
                }
            };
            connect(gl, &OpenGLWidget::MouseNotHoverSignal, this, lambda);
        }
    }
}

void MainWindow::playback()
{
    auto label = ui->StepLabel;
    QString text = QString("%1 / %2").arg(gl->playback.step + 1, 3, 10, QChar(' ')).arg(gl->playback.size);
    label->setText(text);

    auto slider = ui->StepSlider;
    slider->setMinimum(0);
    slider->setMaximum(gl->trajectory.models.size() - 1);
    slider->setSliderPosition(0);

    // value changed event
    {
        auto lambda = [=] (int value)
        {
            // update step
            gl->playback.step = value;
            // update label
            QString text = QString("%1 / %2").arg(gl->playback.step + 1, 3, 10, QChar(' ')).arg(gl->playback.size);
            label->setText(text);
            // update outline color
            // qDebug() << "SetOutlineColor";
            gl->SetOutlineColor(true);
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }

    // next step signal
    {
        auto lambda = [=] ()
        {
            // update slider
            slider->setSliderPosition(gl->playback.step);
            /*
            // update label
            QString text = QString("%1 / %2").arg(gl->playback.step + 1, 3, 10, QChar(' ')).arg(gl->playback.size);
            label->setText(text);
            // update outline color
            gl->SetOutlineColor();
            */
        };
        connect(gl, &OpenGLWidget::NextStepSignal, this, lambda);
    }

    // play button
    {
        auto button = ui->PlayButton;

        button->setIcon(QIcon("../FinalProject/icons/play-solid.svg"));

        auto lambda = [=] ()
        {
            gl->playback.time.start();
            gl->playback.active = true;
        };
        connect(button, &QPushButton::clicked, lambda);
    }

    // pause button
    {
        auto button = ui->PauseButton;

        button->setIcon(QIcon("../FinalProject/icons/pause-solid.svg"));

        auto lambda = [=] ()
        {
            gl->playback.active = false;
            gl->playback.time = QTime();
        };
        connect(button, &QPushButton::clicked, lambda);
    }

    // step backward button
    {
        auto button = ui->StepBackwardButton;

        button->setIcon(QIcon("../FinalProject/icons/step-backward-solid.svg"));

        auto lambda = [=] ()
        {
            // update step
            gl->playback.step = (gl->playback.step == 0) ? (gl->playback.size - 1) : (gl->playback.step - 1);
            // update slider
            slider->setSliderPosition(gl->playback.step);
            /*
            // update label
            QString text = QString("%1 / %2").arg(gl->playback.step + 1, 3, 10, QChar(' ')).arg(gl->playback.size);
            label->setText(text);
            // update outline color
            gl->SetOutlineColor();
            */
        };
        connect(button, &QPushButton::clicked, lambda);
    }

    // step forward button
    {
        auto button = ui->StepForwardButton;

        button->setIcon(QIcon("../FinalProject/icons/step-forward-solid.svg"));

        auto lambda = [=] ()
        {
            // update step
            gl->playback.step = (gl->playback.step + 1) % gl->playback.size;
            // update slider
            slider->setSliderPosition(gl->playback.step);
            /*
            // update label
            QString text = QString("%1 / %2").arg(gl->playback.step + 1, 3, 10, QChar(' ')).arg(gl->playback.size);
            label->setText(text);
            // update outline color
            gl->SetOutlineColor();
            */
        };
        connect(button, &QPushButton::clicked, lambda);
    }
}

void MainWindow::ColorLerp()
{
    QColor albedo = Qt::GlobalColor::red;

    auto label = ui->ColorLerpLabel;

    auto slider = ui->ColorLerpSlider;
    float size = 10;
    slider->setMinimum(-size);
    slider->setMaximum(+size);
    slider->setSliderPosition(0);

    auto lerp = [] (float a, float b, float t)
    {
        t = qBound(0.0f, t, 1.0f);
        return a * (1 - t) + b * t;
    };

    auto LerpQColor = [=] (QColor a, QColor b, float t)
    {
        QColor c;
        c.setRedF(lerp(a.redF(), b.redF(), t));
        c.setGreenF(lerp(a.greenF(), b.greenF(), t));
        c.setBlueF(lerp(a.blueF(), b.blueF(), t));
        return c;
    };

    auto lambda = [=] (int value)
    {
        QColor colour = albedo;

        if (value < 0)
        {
            float t = (size + value) / size;
            colour = LerpQColor(Qt::GlobalColor::black, albedo, t);
        }

        if (value > 0)
        {
            float t = (size - value) / size;
            colour = LerpQColor(Qt::GlobalColor::white, albedo, t);
        }

        QStringList rules;
        rules += QString("background-color: %1").arg(colour.name());
        label->setStyleSheet(rules.join("; "));

        label->setText(colour.name());
    };
    connect(slider, &QSlider::valueChanged, lambda);

}

void MainWindow::SSAOTab()
{
    // active checkbox
    {
        auto checkbox = ui->SSAOActive;
        checkbox->setChecked(gl->SSAO.active);

        auto lambda = [=] (int state)
        {
            // update value
            gl->SSAO.active = (state == Qt::Checked);
        };
        connect(checkbox, &QCheckBox::stateChanged, lambda);
    }

    // blur checkbox
    {
        auto checkbox = ui->SSAOBlur;
        checkbox->setChecked(gl->SSAO.blur);

        qDebug() << checkbox;

        auto lambda = [=] (int state)
        {
            // update value
            gl->SSAO.blur = (state == Qt::Checked);
        };
        connect(checkbox, &QCheckBox::stateChanged, lambda);
    }

    // kernel size
    {
        int size = gl->SSAO.KernelSize;

        auto label = ui->SSAOKernelSizeLabel;
        QString caption = label->text().split(" :").first();
        label->setText(QString("%1 : %2").arg(caption).arg(size, 2));

        auto slider = ui->SSAOKernelSizeSlider;
        slider->setMinimum(1);
        slider->setMaximum(30);

        // update value
        slider->setSliderPosition(size);

        // value changed event
        auto lambda = [=] (int value)
        {
            // update value
            gl->SSAO.KernelSize = value;
            gl->SSAO.SetKernel();
            // gl->KernelTextureFlag = true;
            // update label
            QString caption = label->text().split(" :").first();
            label->setText(QString("%1 : %2").arg(caption).arg(value, 2));
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }

    // noise size
    {
        int size = gl->SSAO.NoiseSize;

        auto label = ui->SSAONoiseSizeLabel;
        QString caption = label->text().split(" :").first();
        label->setText(QString("%1 : %2").arg(caption).arg(size, 2));

        auto slider = ui->SSAONoiseSizeSlider;
        slider->setMinimum(1);
        slider->setMaximum(30);

        // update value
        slider->setSliderPosition(size * 0.5f);

        // value changed event
        auto lambda = [=] (int value)
        {
            int w = gl->geometry().width();
            int h = gl->geometry().height();

            int NoiseSize = value * 2.0f;
            // update value
            gl->SSAO.NoiseSize = NoiseSize;
            gl->SSAO.SetNoise(w, h);
            // update label
            QString caption = label->text().split(" :").first();
            label->setText(QString("%1 : %2").arg(caption).arg(value, 2));
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }

    // radius
    {
        float radius = gl->SSAO.radius;

        auto label = ui->SSAORadiusLabel;
        QString caption = label->text().split(" :").first();
        label->setText(QString("%1 : %2").arg(caption).arg(radius, 6, 'f', 2));

        auto slider = ui->SSAORadiusSlider;
        slider->setMinimum(1);
        slider->setMaximum(10000);

        // update value
        // slider->setSliderPosition(radius * 100);
        slider->setSliderPosition(radius * 100);

        // value changed event
        auto lambda = [=] (int value)
        {
            float radius = value * 0.01f;
            // update value
            gl->SSAO.radius = radius;
            // update label
            QString caption = label->text().split(" :").first();
            label->setText(QString("%1 : %2").arg(caption).arg(radius, 6, 'f', 2));
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }

    // bias
    {
        float bias = gl->SSAO.bias;

        auto label = ui->SSAOBiasLabel;
        QString caption = label->text().split(" :").first();
        label->setText(QString("%1 : %2").arg(caption).arg(bias, 5, 'f', 3));

        auto slider = ui->SSAOBiasSlider;
        slider->setMinimum(1);
        slider->setMaximum(100);

        // update value
        slider->setSliderPosition(bias * 1000);

        // value changed event
        auto lambda = [=] (int value)
        {
            float bias = value * 0.001f;
            // update value
            gl->SSAO.bias = bias;
            // update label
            QString caption = label->text().split(" :").first();
            label->setText(QString("%1 : %2").arg(caption).arg(bias, 5, 'f', 3));
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }
}

void MainWindow::LightTab()
{
    auto checkbox = ui->LightAttenuation;
    checkbox->setChecked(gl->light.attenuation);

    qDebug() << checkbox;

    auto lambda = [=] (int state)
    {
        // update value
        gl->light.attenuation = (state == Qt::Checked);
    };
    connect(checkbox, &QCheckBox::stateChanged, lambda);
}

void MainWindow::MaterialTab()
{
    float size = gl->factors.w();

    auto GetFactorString = [=] (int value)
    {
        QString factor;
        int width = 4;

        if (value < 0)
        {
            factor = QString("-%1").arg(1 - (size + value) / size, width, 'f', 2);
        }

        if (value >= 0)
        {
            factor = QString("+%1").arg(1 - (size - value) / size, width, 'f', 2);
        }

        return factor;
    };

    QVector<QSlider*> sliders;
    sliders += ui->Material_Ambient_Slider;
    sliders += ui->Material_Diffuse_Slider;
    sliders += ui->Material_Specular_Slider;
    sliders += ui->Material_Shininess_Slider;

    QVector<QLabel*> labels;
    labels += ui->Material_Ambient_Label;
    labels += ui->Material_Diffuse_Label;
    labels += ui->Material_Specular_Label;
    labels += ui->Material_Shininess_Label;

    for (int i = 0; i < 3; i++)
    {
        auto label = labels[i];
        auto slider = sliders[i];

        slider->setMinimum(-size);
        slider->setMaximum(+size);
        slider->setSliderPosition(gl->factors[i]);

        QString caption = label->text().split(" :").first();
        QString factor = GetFactorString(gl->factors[i]);
        label->setText(QString("%1 : %2").arg(caption).arg(factor));

        auto lambda = [=] (int value)
        {
            gl->factors[i] = value;

            QString caption = label->text().split(" :").first();
            QString factor = GetFactorString(gl->factors[i]);
            label->setText(QString("%1 : %2").arg(caption).arg(factor));
        };
        connect(slider, &QSlider::valueChanged, lambda);
    }

    // shininess

    auto label = labels.last();
    auto slider = sliders.last();

    slider->setMinimum(0);
    slider->setMaximum(10);
    slider->setSliderPosition(5);

    QString caption = label->text().split(" :").first();
    label->setText(QString("%1 : %2").arg(caption).arg(gl->shininess, 4));

    auto lambda = [=] (int value)
    {
        float shininess = pow(2, value);
        gl->shininess = shininess;

        QString caption = label->text().split(" :").first();
        label->setText(QString("%1 : %2").arg(caption).arg(shininess, 4));
    };
    connect(slider, &QSlider::valueChanged, lambda);
}

void MainWindow::framerate()
{
    auto label = ui->FrameRateLabel;

    auto text = QString("fps : %1").arg(gl->FrameRate.fps, 5, 'f', 2);
    label->setText(text);

    // auto lambda = [=] (float fps)
    auto lambda = [=] ()
    {
        // update value
        auto text = QString("fps : %1").arg(gl->FrameRate.fps, 5, 'f', 2);
        label->setText(text);
    };
    connect(gl, &OpenGLWidget::FrameRateSignal, this, lambda);
}

void MainWindow::ResiduesWindowInit()
{
    /*
    window.setAttribute(Qt::WA_QuitOnClose, false);
    window.setWindowFlag(Qt::WindowMinMaxButtonsHint, false);
    */

    window.setParent(this);
    window.setWindowFlag(Qt::Window);

    window.residues = gl->trajectory.residues;
    window.InitUI();

    auto button = ui->ResiduesWindowButton;
    connect(button, &QPushButton::clicked, [=] () { window.show(); });
}

// keyboard events

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_M:
    {
        qDebug() << "OutlineMode" << gl->outline.mode;
        break;
    }
    case Qt::Key_S:
    {
        gl->SamplerIndex = ((gl->SamplerIndex + 2) % (gl->samplers.size() + 1)) - 1;
        qDebug() << "SamplerIndex" << gl->SamplerIndex;
        break;
    }
    case Qt::Key_T:
    {
        gl->TestDrawingMode = !gl->TestDrawingMode;
        qDebug() << "TestDrawingMode" << gl->TestDrawingMode;
        break;
    }
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
    {
        float t = (event->key() - Qt::Key_0) / static_cast<float>(Qt::Key_9 - Qt::Key_0);
        gl->BackgroundColor = LerpQColor(Qt::GlobalColor::black, Qt::GlobalColor::white, t);
        qDebug() << "BackgroundColor" << gl->BackgroundColor;
        break;
    }
    case Qt::Key_L:
    {
        gl->light.free = !gl->light.free;
        qDebug() << "light.free" << gl->light.free;
        break;
    }
    case Qt::Key_Shift:
    {
        gl->ShiftDown = true;
        qDebug() << "ShiftDown" << gl->ShiftDown;
        break;
    }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Shift:
    {
        gl->ShiftDown = false;
        qDebug() << "ShiftDown" << gl->ShiftDown;
        break;
    }
    }
}
