#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QGraphicsPixmapItem>

class MyGraphicsItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

    using QGraphicsPixmapItem::QGraphicsPixmapItem;
public:
    MyGraphicsItem();
signals:
    void mousePressed(QPointF pos);
    void mouseRelease(QPointF pos);
    void mouseClickAndDrag(QPointF pos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
};

#endif // MYGRAPHICSITEM_H
