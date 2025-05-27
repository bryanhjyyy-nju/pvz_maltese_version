#include "map.h"
#include <cmath>

Map::Map(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{}

//网格初始化
Map::Map(int cols, int rows, QSize cellSize, QPointF originLoc):
    mapCols(cols),
    mapRows(rows),
    mapCellSize(cellSize),
    mapOriginLoc(originLoc){}

//重写两个纯虚函数
QRectF Map::boundingRect() const
{
    return QRectF(mapOriginLoc, QSizeF(mapCols * mapCellSize.width(), mapRows * mapCellSize.height()));
}

void Map::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    painter->setPen(QPen(Qt::red, 1, Qt::SolidLine));

    //绘制垂直线
    for(int x = 0; x <= mapCols; x++){
        qreal lineX = mapOriginLoc.x() + x * mapCellSize.width();
        painter->drawLine(lineX, mapOriginLoc.y(), lineX, mapOriginLoc.y() + mapRows * mapCellSize.height());
    }

    //绘制水平线
    for(int y = 0; y <= mapRows; y++){
        qreal lineY = mapOriginLoc.y() + y * mapCellSize.height();
        painter->drawLine(mapOriginLoc.x(), lineY, mapOriginLoc.x() +mapCols * mapCellSize.width(), lineY);
    }

}


//将坐标为位置转化成网格索引
bool Map::turnPosToMap(const QPointF& mousePos,int& col,int& row) const{
    QPointF relativePos = mousePos - mapOriginLoc;
    col = relativePos.x() / mapCellSize.width();
    row = relativePos.y() / mapCellSize.height();
    return (col >= 0 && col < mapCols && row >= 0 && row < mapRows);
}

//获取网格中心坐标
QPointF Map::cellCenter(int col, int row) const{
    return mapOriginLoc + QPointF(col * mapCellSize.width() + mapCellSize.width()/2, row * mapCellSize.height() + mapCellSize.width() / 2);
}
