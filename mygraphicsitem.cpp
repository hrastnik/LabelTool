#include "mygraphicsitem.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>

MyGraphicsItem::MyGraphicsItem()
{

}

void MyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    //QGraphicsPixmapItem::mousePressEvent(event); TODO: Bug in Qt?
    emit mousePressed(event->pos());
}

void MyGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseRelease(event->pos());
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void MyGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPixmapItem::mouseMoveEvent(event);
    emit mouseClickAndDrag(event->pos());
}
