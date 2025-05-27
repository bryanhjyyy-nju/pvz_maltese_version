#include "mygamescene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include "singingwhite.h"
#include "heartwhite.h"

MyGameScene::MyGameScene(QMainWindow *parent)
    : QGraphicsScene(parent)
{
    //设置有效操作范围
    setSceneRect(0, 0, 1650, 900);

    //加载背景
    QPixmap backgroundPixmap(":/others/Image/grass.jpg");
    // qDebug() << backgroundPixmap.height() << backgroundPixmap.width();
    backgroundPixmap = backgroundPixmap.scaled((900.0 / backgroundPixmap.height()) * backgroundPixmap.width(),900);
    //采用backGroundItem管理场景界面
    QGraphicsPixmapItem *backGroundItem = addPixmap(backgroundPixmap);
    backGroundItem->setPos(0, 0);
    backGroundItem->setScale(1);

    mapGrid = new Map(9, 5, QSize(121,145), QPointF(380,130));
    addItem(mapGrid);
    initMapOccupied(9, 5);
}

bool MyGameScene::initMapOccupied(int cols, int rows){
    if(!mapOccupied){
        mapOccupied = new bool[rows * cols];
        for (int i = 0; i < rows * cols; i++){
            mapOccupied[i] = false;
        }
        return true;
    }
    else return false;
}

MyGameScene::~MyGameScene(){
    if(mapOccupied) {
        delete mapOccupied;
        mapOccupied = NULL;
    }
}

void MyGameScene::mousePressEvent(QGraphicsSceneMouseEvent * event){
    //转换到坐标网格系统
    int col, row;
    //转换成坐标网格系统
    if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
        //检查是否被占用
        if(!mapOccupied[row * 9 + col]){
            //计算中心的坐标
            QPointF centerLoc = mapGrid->cellCenter(col,row);
            //创建植物并定位
            HeartWhite * myDog = new HeartWhite;
            myDog->setPos(centerLoc - QPointF(myDog->pixmap().width() / 2.0 * myDog->getMyScale(), myDog->pixmap().height() / 2.0 * myDog->getMyScale()));
            addItem(myDog);

            //标记已经占用
            mapOccupied[row * 9 + col] = true;

            //用于调试
            qDebug() << col << " " << row;
        }
        else{
            qDebug() << "已被占用";
        }

        QGraphicsScene::mousePressEvent(event);
    }
}
