#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QStandardPaths>
#include <QDebug>
#include <QLineF>
#include <QGraphicsRectItem>
#include <QGraphicsItem>
#include <QPalette>

#include "mygraphicsitem.h"
#include "dataset.h"
#include "mydelegate.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Ui initial setup
    ui->centralSplitter->setStretchFactor(0, 25);
    ui->centralSplitter->setStretchFactor(1, 75);
    resize(1000, 700);

    // init model
    imageListModel = new QStringListModel(this);

    graphicsScene = new QGraphicsScene(this);
    ui->imageGraphicsView->setScene(graphicsScene);

    // Get dataset reference
    //dataset = Dataset::getInstance();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseButton_clicked()
{
    QString imageDirectory = QFileDialog::getExistingDirectory(
                this, "Select images directory",
                QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).last());

    if (imageDirectory.isEmpty())
    {
        ui->startLabelingButton->setEnabled(false);
        ui->nextImageButton->setEnabled(false);
        ui->previousImageButton->setEnabled(false);
        return;
    }

    // Get list of images in directory
    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.bmp";

    currentImagesDir = QDir(imageDirectory);
    currentImagesDir.setNameFilters(filters);
    currentImagesDir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);

    QStringList imageFilenames = currentImagesDir.entryList();

    if (imageFilenames.empty())
    {
        ui->startLabelingButton->setEnabled(false);
        ui->nextImageButton->setEnabled(false);
        ui->previousImageButton->setEnabled(false);
        return;
    }

    ui->startLabelingButton->setEnabled(true);

    imageListModel->setStringList(imageFilenames);

    ui->dirContentList->setModel(imageListModel);

    // Add directory path to line edit
    ui->dirLineEdit->setText(currentImagesDir.absolutePath());

    ui->nextImageButton->setEnabled(true);
    ui->previousImageButton->setEnabled(true);

    dataset.fromStringList(imageFilenames);
}

void MainWindow::on_startLabelingButton_clicked()
{
    currentImageIndex = 0;
    ui->previousImageButton->setEnabled(false);
    ui->nextImageButton->setEnabled(true);
    showCurrentImage();
    ui->dirContentList->setCurrentIndex(imageListModel->index(currentImageIndex, 0));
}

void MainWindow::on_nextImageButton_clicked()
{
    if (currentImageIndex + 1 == imageListModel->rowCount()) {
        return;
    }

    currentImageIndex++;

    ui->previousImageButton->setEnabled(true);

    if (currentImageIndex >= imageListModel->rowCount() - 1) {
        ui->nextImageButton->setEnabled(false);
    }

    showCurrentImage();

    ui->dirContentList->setCurrentIndex(imageListModel->index(currentImageIndex, 0));
}

void MainWindow::on_previousImageButton_clicked()
{
    if (currentImageIndex - 1 < 0) {
        return;
    }

    currentImageIndex--;

    ui->nextImageButton->setEnabled(true);

    if (currentImageIndex == 0) {
        ui->previousImageButton->setEnabled(false);
    }

    showCurrentImage();

    ui->dirContentList->setCurrentIndex(imageListModel->index(currentImageIndex, 0));
}

void MainWindow::showCurrentImage()
{
    QStringList imageList = imageListModel->stringList();
    QString imageName = imageList[currentImageIndex];
    QString imagePath = currentImagesDir.filePath(imageName);

    qDebug() << "Opening image" << imagePath;
    QPixmap image(imagePath);

    if (itemsOnGraphicsScene.contains("active_image")) {
        qDebug() << "Deleting previous image";
        for (QString key : itemsOnGraphicsScene.keys()) {
            delete itemsOnGraphicsScene[key];
        }
        itemsOnGraphicsScene.clear();
        graphicsScene->clear();

    }
    MyGraphicsItem *item = new MyGraphicsItem();
    item->setPixmap(image);

    itemsOnGraphicsScene["active_image"] = (QGraphicsItem *) item;

    graphicsScene->clear();
    graphicsScene->addItem(item);
    fitImageInView();

    // Check if rect exists
    Dataset &dataset = Dataset::getInstance();
    Dataset::Label label = dataset.images[imageName]->label;
    bool labelExists = !(label.x == 0 && label.y == 0 && label.w == 0 && label.h == 0);
    if (labelExists) {
        itemsOnGraphicsScene["rect"] = (QGraphicsItem *) graphicsScene->addRect(
                    label.x, label.y, label.w, label.h, QPen(QColor(255, 0, 0)));
    }

    // Count number of labeled images
    int numImages = imageList.size();
    int numLabeled = std::count_if(imageList.begin(), imageList.end(), [&](QString imgPath) {
            Dataset::Label &label = dataset.images[imgPath]->label;
            return !(label.x == 0 && label.y == 0 && label.w == 0 && label.h == 0);
    });

    ui->labelImagesDone->setText(
                "Labeled Images: " + QString::number(numLabeled) + "/" + QString::number(numImages));


    // Hook handlers to image
    connect(item, &MyGraphicsItem::mousePressed, this, &MainWindow::onImageMouseDown);
    connect(item, &MyGraphicsItem::mouseRelease, this, &MainWindow::onImageMouseUp);
    connect(item, &MyGraphicsItem::mouseClickAndDrag, this, &MainWindow::onImageMouseMove);
}

void MainWindow::fitImageInView()
{
    if (itemsOnGraphicsScene.contains("active_image")) {
        ui->imageGraphicsView->fitInView(itemsOnGraphicsScene["active_image"], Qt::KeepAspectRatio);
    }
}

void MainWindow::onImageMouseDown(QPointF pos)
{
    QString currentImagePath = imageListModel->stringList().at(currentImageIndex);

    Dataset::Label &label = dataset.images[currentImagePath]->label;

    if (label.x == 0 && label.y == 0 && label.w == 0 && label.h == 0) {
        // Label doesn't yet exist
        qDebug() << "label doesn't yet exist...";
        label.x = pos.x();
        label.y = pos.y();
    }
    else {
        qDebug() << "label exists...";
        float distToT = qAbs(pos.y() - label.y);
        float distToB = qAbs(pos.y() - (label.y + label.h));
        float distToL = qAbs(pos.x() - label.x);
        float distToR = qAbs(pos.x() - (label.x + label.w));

        if (distToR < 20 && distToR <= distToL) {
            draggingR = true;
        }
        else if (distToL < 20 && distToL <= distToR) {
            draggingL = true;
        }

        if (distToT < 20 && distToT <= distToB) {
            draggingT = true;
        }
        else if (distToB < 20 && distToB <= distToT) {
            draggingB = true;
        }

        if (distToT > 20 && distToB > 20 && distToL > 20 && distToR > 20) {
            qDebug() << "label changing x and y...";
            label.x = pos.x();
            label.y = pos.y();
            label.w = 1;
            label.h = 1;
        }
    }

    qDebug() << "Label x y : " << label.x << label.y;

    if (itemsOnGraphicsScene.contains("rect")) {
        graphicsScene->removeItem(itemsOnGraphicsScene["rect"]);
    }

    itemsOnGraphicsScene["rect"] = (QGraphicsItem *) graphicsScene->addRect(
                label.x, label.y, label.w, label.h, QPen(QColor(255, 0, 0)));
}

void MainWindow::onImageMouseUp(QPointF /*pos*/)
{
    draggingR = draggingL = draggingT = draggingB = false;
}

void MainWindow::onImageMouseMove(QPointF pos)
{
    QString currentImagePath = imageListModel->stringList().at(currentImageIndex);
    Dataset::Label &label = dataset.images[currentImagePath]->label;

    if (!draggingB && !draggingT && !draggingR && !draggingL) {
        label.w = pos.x() - label.x;
        label.h = pos.y() - label.y;
    }
    else {
        if (draggingT) {
            qDebug() << "T";
            label.h -= (pos.y() - label.y);
            label.y = pos.y();
        }
        if (draggingB) {
            qDebug() << "B";
            label.h += pos.y() - (label.y + label.h);
        }
        if (draggingL) {
            qDebug() << "L";
            qDebug() << "incW by" << (label.x - pos.x());
            label.w -= (pos.x() - label.x);
            label.x = pos.x();
        }
        if (draggingR) {
            qDebug() << "R";
            label.w += pos.x() - (label.x + label.w);
        }
    }

    QGraphicsRectItem *rect = (QGraphicsRectItem *) itemsOnGraphicsScene["rect"];
    rect->setRect(label.x, label.y, label.w, label.h);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    fitImageInView();
}


void MainWindow::on_dirContentList_clicked(const QModelIndex &index)
{
    currentImageIndex = index.row();

    if (currentImageIndex == 0) {
        ui->previousImageButton->setEnabled(false);
        ui->nextImageButton->setEnabled(true);
    }
    else if (currentImageIndex == index.model()->rowCount() - 1) {
        ui->previousImageButton->setEnabled(true);
        ui->nextImageButton->setEnabled(false);
    }

    showCurrentImage();
}

void MainWindow::on_saveToFileButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save To File"), QString(), tr("JSON (*.json)"));

    if (filename.isEmpty()) {
        return;
    }

    QString output = "dataset {\n";

    Dataset  &dataset = Dataset::getInstance();
    QStringList imageList = this->imageListModel->stringList();

    for (int i = 0;i < imageList.size(); i++) {
        QString imageName = imageList[i];
        Dataset::Label &label = dataset.images[imageName]->label;

        if (label.x == 0 && label.y == 0 && label.w == 0 && label.h == 0) {
            continue;
        }
        output += "  image { \n";
        output += "    name: \"" + imageName + "\",\n";
        output += "    label: {\n";
        output += "      x: " + QString::number(label.x) + ",\n";
        output += "      y: " + QString::number(label.y) + ",\n";
        output += "      w: " + QString::number(label.w) + ",\n";
        output += "      h: " + QString::number(label.h) + "\n";
        output += "    }\n";
        output += "  },\n";
    }

    output.chop(2);
    output += "\n";

    output += "}";

    QFile outputFile(filename);

    qDebug() << "Trying to open " << filename;
    if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Couldnt open file";
        return;
    }

    QTextStream fileStream(&outputFile);
    fileStream << output;
    outputFile.close();
}
