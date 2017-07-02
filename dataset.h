#ifndef DATASET_H
#define DATASET_H

#include <QMap>
#include <QGraphicsItem>
#include <QPixmap>
#include <QVector>

class Dataset
{
public:
    class Label {
    public:
        float x = 0, y = 0, w = 0, h = 0;
    };

    class Image {
    public:
        Image() {}
        QString name;
        Label label;
    };

    static Dataset &getInstance();

    void fromStringList(QStringList images);

    QMap<QString, Image *> images;

private:
    Dataset();
    Dataset(const Dataset& other) = delete;
    Dataset& operator=(Dataset const&) = delete;

};

#endif // DATASET_H
