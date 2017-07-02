#include "dataset.h"

void Dataset::fromStringList(QStringList images)
{
    this->images.clear();

    for (QString imagePath : images)
    {
        Image *image = new Image();
        image->name = imagePath;

        this->images[imagePath] = image;
    }
}

Dataset::Dataset()
{

}

Dataset &Dataset::getInstance()
{
    static Dataset instance;
    return instance;
}
