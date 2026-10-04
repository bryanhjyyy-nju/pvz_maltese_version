#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>

class Map : public QObject, public QGraphicsItem
{
    Q_OBJECT
    // Q_INTERFACES(QGraphicsItem)
public:
    explicit Map(QGraphicsItem *parent = nullptr);

    //网格初始化
    Map(int cols, int rows, QSize cellSize, QPointF originLoc = QPointF(0, 0));

    //重写两个纯虚函数
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;

    //将坐标为位置转化成网格索引
    bool turnPosToMap(const QPointF& mousePos,int& col,int& row) const;

    //获取网格中心坐标
    QPointF cellCenter(int col, int row) const;
    void setPlantableRows(int first,int last) { firstPlantable=first; lastPlantable=last; }

private:
    int mapCols; //地图上总列数
    int mapRows; //地图上总行数
    QSize mapCellSize; //地图上每一格的大小
    QPointF mapOriginLoc; //网格左上角的坐标
    int firstPlantable=0,lastPlantable=4;
signals:
};

#endif // MAP_H
