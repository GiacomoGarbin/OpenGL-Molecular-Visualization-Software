#ifndef UTILITY_H
#define UTILITY_H

#include <QDebug>
#include <QString>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMap>
#include <QTime>
#include <QColor>
#include <QLabel>
#include <QRandomGenerator>

#include <Eigen/Dense>

namespace utility
{

// gl namespaces

namespace ProgramIndex
{
enum
{
    POINT,
    IMPOSTOR,
    SSAO_OCCLUSION,
    SSAO_BLUR,
    LIGHTING,
    SILHOUETTE,
    OUTLINE,
    MARKER,
    QUAD
};
}

// namespace VAOIndex { enum {  }; }

namespace FBOIndex
{
enum
{
    GEOMETRY,
    SSAO_OCCLUSION,
    SSAO_BLUR,
    SILHOUETTE
};
}

namespace TextureIndex
{
enum
{
    CENTER,
    NORMAL,
    ALBEDO,
    COLOUR,
    SSAO_KERNEL,
    SSAO_NOISE,
    SSAO_OCCLUSION,
    SSAO_BLUR,
    SILHOUETTE,
    OUTLINE
};
}

namespace RBOIndex
{
enum
{
    GEOMETRY,
    SILHOUETTE
};
}

// random

static float RandomZeroOne()
{
    unsigned int max = static_cast<unsigned int>(pow(2, 32) - 1);
    unsigned int n = static_cast<unsigned int>(QRandomGenerator::securelySeeded().generate());
    // returns a random float value between 0.0f and 1.0f (extremes included)
    return static_cast<float>(n) / static_cast<float>(max);
};

// lerp

static float LerpFloat(float a, float b, float t)
{
    t = qBound(0.0f, t, 1.0f);
    return a * (1 - t) + b * t;
};

static QColor  LerpQColor(QColor a, QColor b, float t)
{
    QColor c;
    c.setRedF(LerpFloat(a.redF(), b.redF(), t));
    c.setGreenF(LerpFloat(a.greenF(), b.greenF(), t));
    c.setBlueF(LerpFloat(a.blueF(), b.blueF(), t));
    return c;
};

// lighting and SSAO

struct Light
{
    // geometry
    QVector3D position;
    QVector3D direction;

    bool free;
    QMatrix4x4 model;

    // properties
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
    // attenuation
    bool attenuation;
    float constant;
    float linear;
    float quadratic;
};

class ScreenSpaceAmbientOcclusion
{
public:
    bool active;
    bool blur;
    int KernelSize;
    int NoiseSize;
    QVector<QVector3D> kernel;
    QVector<QVector3D> noise;
    QVector2D NoiseScale;
    float radius;
    float bias;

    bool KernelTextureFlag = false;
    bool NoiseTextureFlag = false;

    /*
    void SetKernel()
    {
        auto lerp = [] (float a, float b, float f)
        {
            return a + f * (b - a);
        };

        kernel.clear();

        for (int i = 0; i < KernelSize * KernelSize; i++)
        {
            float x = RandomZeroOne();
            float y = RandomZeroOne();
            float z = RandomZeroOne();

            QVector3D sample = { x * 2.0f - 1.0f, y * 2.0f - 1.0f, z };
            sample.normalize();
            sample *= RandomZeroOne();

            float scale = static_cast<float>(i) / static_cast<float>(KernelSize * KernelSize);
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            kernel.append(sample);
        }

        KernelTextureFlag = true;
    }
    */

    void SetKernel()
    {
        QRandomGenerator generator = QRandomGenerator::securelySeeded();

        auto lerp = [] (float a, float b, float f)
        {
            return a + f * (b - a);
        };

        kernel.clear();

        float size = KernelSize * KernelSize;

        for (int i = 0; i < size; i++)
        {
            QVector3D sample;
            sample.setX(generator.generateDouble() * 2.0f - 1.0f);
            sample.setY(generator.generateDouble() * 2.0f - 1.0f);
            sample.setZ(generator.generateDouble());
            sample.normalize();
            sample *= generator.generateDouble();

            float scale = i / size;
            sample *= lerp(0.1f, 1.0f, scale * scale);

            kernel.append(sample);
        }

        KernelTextureFlag = true;
    }

    /*
    void SetNoise(int w, int h)
    {
        for (int i = 0; i < NoiseSize * NoiseSize; i++)
        {
            float x = RandomZeroOne() * 2.0f - 1.0f;
            float y = RandomZeroOne() * 2.0f - 1.0f;
            noise.append({x, y, 0});
        }
        SetNoiseScale(w, h);

        NoiseTextureFlag = true;
    }
    */

    void SetNoise(int w, int h)
    {
        QRandomGenerator generator = QRandomGenerator::securelySeeded();

        noise.clear();

        for (int i = 0; i < NoiseSize * NoiseSize; i++)
        {
            float x = generator.generateDouble() * 2.0f - 1.0f;
            float y = generator.generateDouble() * 2.0f - 1.0f;
            noise.append({x, y, 0});
        }

        SetNoiseScale(w, h);

        NoiseTextureFlag = true;
    }

    void SetNoiseScale(int w, int h)
    {
        NoiseScale.setX(static_cast<float>(w));
        NoiseScale.setY(static_cast<float>(h));
        NoiseScale /= static_cast<float>(NoiseSize);
    }
};

// formatting

static QString FormatNumber(float value, int width = 5, int precision = 2, float threshold = FLT_MIN)
{
    if (value > 0 && value < threshold)
    {
        return QString("~0");
    }

    return QString("%1").arg(value, width, 'f', precision, ' ');
};

static QString FormatVector(QVector3D vector, int width = 5, int precision = 2)
{
    QStringList list;
    list += QString("%1").arg(vector.x(), width, 'f', precision, ' ');
    list += QString("%1").arg(vector.y(), width, 'f', precision, ' ');
    list += QString("%1").arg(vector.z(), width, 'f', precision, ' ');
    return QString("(%1)").arg(list.join(","));
};

// tables

// map : field -> value
typedef QMap<QString, QString> Record;
typedef QVector<Record> Table;

static Table CookCSV(QString text, QChar sep = ';')
{
    Table table;

    auto lines = text.split("\n");

    auto keys = lines.takeFirst().split(sep);
    int length = keys.length();

    auto trim = [] (QString string) { return string.trimmed(); };
    std::for_each(keys.begin(), keys.end(), trim);

    int step = 0;
    int size = lines.size();

    for (auto line : lines)
    {
        auto values = line.split(sep);
        assert(values.length() == length);

        Record record;
        for (int i = 0; i < length; i++)
        {
            record[keys[i]] = values[i].trimmed();
        }
        table += record;

        step += 1;
    }

    return table;
}

// conversions

static Eigen::Vector3f FromQVector3DToVector3f(QVector3D vector)
{
    return Eigen::Vector3f(vector.x(), vector.y(), vector.z());
}

static QVector3D FromVector3fToQVector3D(Eigen::Vector3f vector)
{
    return QVector3D(vector.x(), vector.y(), vector.z());
}

static QVector3D FromQColorToQVector3D(QColor color)
{
    QVector3D vector;
    vector.setX(static_cast<float>(color.redF()));
    vector.setY(static_cast<float>(color.greenF()));
    vector.setZ(static_cast<float>(color.blueF()));
    return vector;
}

// mathematics

template <class T>
static T GetMean (QVector<T> v)
{
    // T sum = [] (T a, T b) { return a + b; };
    std::function<T(T, T)> sum = [] (T a, T b) { return a + b; };

    T total = std::accumulate(std::next(v.begin()), v.end(), v[0], sum);
    return  total / v.size();
}

/*
static QVector3D GetCentroid(QVector<QVector3D> v)
{
    auto lambda = [] (QVector3D a, QVector3D b) { return a + b; };
    return std::accumulate(std::next(v.begin()), v.end(), v[0], lambda) / v.size();
}
*/

static QVector3D GetCentroid(QVector<QVector3D> v)
{
    // return GetMean<QVector3D>(v);
    return GetMean(v);
}

static float GetAverage(QVector<float> v)
{
    // return GetMean<float>(v);
    return GetMean(v);
}

// collections

template <class T>
static QString PackNumbers(QVector<T> collection, QString sep = ";")
{
    QStringList list;
    for (auto number : collection)
    {
        list += QString::number(number);
    }
    return list.join(sep);
}

static QVector<int> UnpackInts(QString PackedData, QString sep = ";")
{
    QVector<int> collection;

    QStringList list = PackedData.split(sep);
    for (auto number : list)
    {
        collection += number.toInt();
    }

    return collection;
}

static QVector<float> UnpackFloats(QString PackedData, QString sep = ";")
{
    QVector<float> collection;

    QStringList list = PackedData.split(sep);
    for (auto number : list)
    {
        collection += number.toFloat();
    }

    return collection;
}

template <class T>
static QString PackVector(T vector, QString sep = ",")
{
    QStringList list;
    list += QString::number(vector.x());
    list += QString::number(vector.y());
    list += QString::number(vector.z());
    return list.join(sep);
}

template <class T>
static QString PackVectors(QVector<T> collection, QString InnerSep = ",", QString OuterSep = ";")
{
    QStringList list;
    for (auto vector : collection)
    {
        list += PackVector(vector, InnerSep);
    }
    return list.join(OuterSep);
}

// template <class T>
// static T UnpackVector(QString PackedData, QString sep = ",")
static QVector3D UnpackVector(QString PackedData, QString sep = ",")
{
    QStringList list = PackedData.split(sep);

    QVector3D vector;
    vector.setX(list[0].toFloat());
    vector.setY(list[1].toFloat());
    vector.setZ(list[2].toFloat());

    return vector;
}

// template <class T>
// static QVector<T> UnpackVectors(QString PackedData, QString InnerSep = ",", QString OuterSep = ";")
static QVector<QVector3D> UnpackVectors(QString PackedData, QString InnerSep = ",", QString OuterSep = ";")
{
    QVector<QVector3D> collection;

    QStringList list = PackedData.split(OuterSep);
    for (auto vector : list)
    {
        collection += UnpackVector(vector, InnerSep);
    }

    return collection;
}

// structures

struct CameraData
{
    float pitch;
    float yaw;
    float radius;

    QVector3D eye;
    QVector3D center;
    QVector3D up;

    QVector3D right;
    QVector3D front;

    float FieldOfView;
    float AspectRatio;
    float NearPlane;
    float FarPlane;
};

struct MouseEventData
{
    int lastX;
    int lastY;
    bool MouseRightButton;
    bool MouseLeftButton;
    bool MouseMiddleButton;
    bool MouseWheel;
};

struct PlaybackData
{
    bool active;
    int step;
    int size;
    QTime time;
    float speed; // frames per second
};

struct FrameRateData
{
    QTime time;
    int frames;
    float fps; // frames per second
};

struct VertexData
{
    QVector3D center;
    float radius;
    // QColor albedo;
    QVector3D albedo;

    // int AtomNumber;
    // unsigned int AtomNumber;
    // float AtomNumber;

    // unsigned int AtomNumber;
    // unsigned int ResidueNumber;

    QVector2D number;
};

typedef QVector<VertexData> ModelData;

// elements

struct ChemicalElement
{
    QString symbol;
    QString name;
    float radius; // Angstrom
    QColor albedo; // Jmol
};

// map : symbol -> element
static QMap<QString, ChemicalElement> ChemicalElements
{
    {"N", {"N", "Nitrogen", 1.55f, "#3050f8"}},
    {"C", {"C", "Carbon", 1.7f, "#909090"}},
    {"O", {"O", "Oxygen", 1.52f, "#ff0d0d"}},
    {"H", {"H", "Hydrogen", 1.2f, "#ffffff"}},
    {"S", {"S", "Sulfur", 1.8f, "#ffff30"}},
    {"Ca", {"Ca", "Calcium", 2.31f, "#3dff00"}}
};

struct AminoAcid
{
    QString symbol;
    QString name;
};

// "ASN", "LEU", "TYR", "GLN", "PHE", "LYS", "MET", "ILE", "CYX", "THR", "VAL", "PRO", "SER", "ARG", "TRP", "ASP", "ALA", "GLY", "HIP", "GLU", "AIN", "CA"

// map : symbol -> amino acid
static QMap<QString, AminoAcid> AminoAcids
{
    {}
};

/*
// map : element -> radius (in angstrom)
static QMap<QString, float> AtomRadius
{
    {"N", 1.55f}, // Nitrogen
    {"C", 1.7f}, // Carbon
    {"O", 1.52f}, // Oxygen
    {"H", 1.2f}, // Hydrogen
    {"S", 1.8f}, // Sulfur
    {"Ca", 2.31f} // Calcium
};

// map : element -> albedo (Jmol)
static QMap<QString, QColor> AtomAlbedo
{
    {"N", "#3050f8"}, // Nitrogen
    {"C", "#909090"}, // Carbon
    {"O", "#ff0d0d"}, // Oxygen
    {"H", "#ffffff"}, // Hydrogen
    {"S", "#ffff30"}, // Sulfur
    {"Ca", "#3dff00"} // Calcium
};
*/

// color palette

enum ColorPaletteType { sequential, diverging, qualitative };

struct ColorPalette
{
    QString name;
    ColorPaletteType type;
    QMap<int, QString> colors;
};

static QVector<ColorPalette> ColorPalettes
{
    /*
    {
        "",
        ,
        {
            {3, ""},
            {4, ""},
            {5, ""},
            {6, ""},
            {7, ""}
        }
    },
    */

    {
        "RdPu",
        sequential,
        {
            {3, "#fde0dd #fa9fb5 #c51b8a"},
            {4, "#feebe2 #fbb4b9 #f768a1 #ae017e"},
            {5, "#feebe2 #fbb4b9 #f768a1 #c51b8a #7a0177"},
            {6, "#feebe2 #fcc5c0 #fa9fb5 #f768a1 #c51b8a #7a0177"},
            {7, "#feebe2 #fcc5c0 #fa9fb5 #f768a1 #dd3497 #ae017e #7a0177"}
        }
    },
    {
        "BrBG",
        diverging,
        {
            {3, "#d8b365 #f5f5f5 #5ab4ac"},
            {4, "#a6611a #dfc27d #80cdc1 #018571"},
            {5, "#a6611a #dfc27d #f5f5f5 #80cdc1 #018571"},
            {6, "#8c510a #d8b365 #f6e8c3 #c7eae5 #5ab4ac #01665e"},
            {7, "#8c510a #d8b365 #f6e8c3 #f5f5f5 #c7eae5 #5ab4ac #01665e"}
        }
    },
    {
        "Set1",
        qualitative,
        {
            {3, "#e41a1c #377eb8 #4daf4a"},
            {4, "#e41a1c #377eb8 #4daf4a #984ea3"},
            {5, "#e41a1c #377eb8 #4daf4a #984ea3 #ff7f00"},
            {6, "#e41a1c #377eb8 #4daf4a #984ea3 #ff7f00 #ffff33"},
            {7, "#e41a1c #377eb8 #4daf4a #984ea3 #ff7f00 #ffff33 #a65628"}
        }
    },
    {
        "RdYlBu",
        diverging,
        {
            {3, "#fc8d59 #ffffbf #91bfdb"},
            {4, "#d7191c #fdae61 #abd9e9 #2c7bb6"},
            {5, "#d7191c #fdae61 #ffffbf #abd9e9 #2c7bb6"},
            {6, "#d73027 #fc8d59 #fee090 #e0f3f8 #91bfdb #4575b4"},
            {7, "#d73027 #fc8d59 #fee090 #ffffbf #e0f3f8 #91bfdb #4575b4"}
        }
    },
};

// color scheme

struct ColorStep
{
    int number;
    float min;
    float max;
    QColor color;
};

/*
struct ColorScheme
{
    ColorPalette palette;
    int size;
    QVector<ColorStep> steps;
};
*/

typedef QVector<ColorStep> ColorScheme;

static ColorScheme GetColorScheme(float min, float max, ColorPalette palette, int size)
{
    ColorScheme scheme;

    QStringList colors = palette.colors[size].split(" ");
    float span = (max - min) / size;

    for (int i = 0; i < size; i++)
    {
        ColorStep step;
        step.number = i;
        step.min = min + span * i;
        step.max = min + span * (i+1);
        step.color = colors[i];
        scheme += step;
    }

    scheme.first().min = 0; // FLT_MIN
    scheme.last().max = FLT_MAX;

    return scheme;
}

static ColorStep GetColorStep(ColorScheme scheme, float value)
{
    for (auto step : scheme)
    {
        if (step.min <= value && value < step.max)
        {
            return step;
        }
    }

    ColorStep InvalidStep;
    InvalidStep.number = -1;
    InvalidStep.min = NAN;
    InvalidStep.max = NAN;
    InvalidStep.color = QColor::Invalid;

    return InvalidStep;
}

static QString GetColorStepCaption(ColorStep step, int width = 8, int precision = 5, QString VarName = "x")
{
    QStringList text;

    if (step.min > 0)
    {
        text += QString("%1 <=").arg(FormatNumber(step.min, width, precision));
    }

    text += VarName;

    if (step.max < FLT_MAX)
    {
        text += QString("< %2").arg(FormatNumber(step.max, width, precision));
    }

    return text.join(" ");
}

// outline

enum OutlineMode { RESIDUE_RMSF, RESIDUE_RMSD, ATOM_RMSF, ATOM_RMSD };
enum BoundaryValues { absolute, relative };

struct OutlineData
{
    bool active;
    OutlineMode mode;
    bool grayscale;
    ColorPalette palette;
    int size;
    // ColorScheme scheme;
    QVector<ColorScheme> schemes;
    int thickness;
    BoundaryValues boundary;
    QVector<QVector3D> colours;

    int filter;

    bool OutlineTextureFlag = false;
};

}

#endif // UTILITY_H
