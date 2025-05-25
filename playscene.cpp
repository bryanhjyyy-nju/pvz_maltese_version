#include "playscene.h"

PlayScene::PlayScene(QWidget *parent)
    : QMainWindow{parent}
{}

PlayScene::PlayScene(int levelNum){

    this->levelIndex = levelNum;
}
