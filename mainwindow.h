#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QGraphicsScene>
#include <QDir>
#include <QPointF>
#include "dataset.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_browseButton_clicked();

    void on_startLabelingButton_clicked();

    void on_nextImageButton_clicked();
    void on_previousImageButton_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel *imageListModel;
    QGraphicsScene *graphicsScene;
    QMap<QString, QGraphicsItem *> itemsOnGraphicsScene;
    void showCurrentImage();
    void fitImageInView();
    int currentImageIndex = 0;
    QDir currentImagesDir;
    Dataset &dataset = Dataset::getInstance();
    bool draggingT = false;
    bool draggingR = false;
    bool draggingB = false;
    bool draggingL = false;

private slots:
    void onImageMouseDown(QPointF pos);
    void onImageMouseUp(QPointF pos);
    void onImageMouseMove(QPointF pos);

    void on_dirContentList_clicked(const QModelIndex &index);

    void on_saveToFileButton_clicked();

protected:
    void resizeEvent(QResizeEvent * event) override;

};

#endif // MAINWINDOW_H
