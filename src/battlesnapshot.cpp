#include "battlesnapshot.h"
#include "playscene.h"
#include "yellowdogs.h"
#include "singingwhite.h"
#include "dblsingwhite.h"
#include "linewhite.h"
#include "heart.h"
#include "bullet.h"
#include "enemyprojectile.h"
#include "combateffect.h"
#include "battlebanner.h"
#include "lawn.h"
#include <QJsonArray>
#include <QLabel>
#include <QSignalBlocker>
#include <QBuffer>
#include <QImageReader>
#include <cmath>

namespace {
QString encodedFrame(const QPixmap& frame) {
    QByteArray bytes; QBuffer buffer(&bytes); buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer,"PNG");
    return QString::fromLatin1(bytes.toBase64());
}
QPixmap decodedFrame(const QJsonValue& value) {
    if(!value.isString() || value.toString().size()>2*1024*1024) return {};
    auto bytes=QByteArray::fromBase64(value.toString().toLatin1());
    if(bytes.isEmpty()) return {};
    QBuffer buffer(&bytes); buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer,"PNG"); const auto size=reader.size();
    if(size.isEmpty() || size.width()>1024 || size.height()>1024) return {};
    return QPixmap::fromImage(reader.read());
}
QJsonArray point(const QPointF& p) { return {p.x(),p.y()}; }
QPointF position(const QJsonValue& v) { const auto a=v.toArray(); return {a[0].toDouble(),a[1].toDouble()}; }
QJsonArray integers(const QVector<int>& values) {
    QJsonArray result; for(int value : values) result.append(value); return result;
}
QVector<int> numbers(const QJsonValue& value) {
    QVector<int> result; for(const auto v : value.toArray()) result.append(v.toInt()); return result;
}
QJsonValue variant(const QVariant& v) {
    if(v.userType()==QMetaType::QPointF) return QJsonObject{{"point",point(v.toPointF())}};
    return QJsonValue::fromVariant(v);
}
QVariant value(const QJsonValue& v) {
    if(v.isObject()) return position(v.toObject()["point"]);
    return v.toVariant();
}
QJsonObject activities(QObject *root,const GamePause& pause,int rate) {
    QJsonArray timers,animations;
    for(auto *timer : root->findChildren<GameTimer*>(QString(),Qt::FindDirectChildrenOnly))
        timers.append(QJsonObject{{"interval",timer->gameInterval()},{"active",timer->willResume()},
                                 {"remaining",timer->remainingGameTime()}});
    for(auto *animation : root->findChildren<QAbstractAnimation*>(QString(),Qt::FindDirectChildrenOnly)) {
        QJsonObject item{{"state",int(animation->state())},
                         {"resume",animation->state()==QAbstractAnimation::Running || pause.resumesAnimation(animation)},
                         {"time",animation->currentTime()*double(rate)},{"loops",animation->loopCount()}};
        if(auto *property=dynamic_cast<GamePropertyAnimation*>(animation)) item["duration"]=property->gameDuration();
        else if(auto *variable=dynamic_cast<GameVariantAnimation*>(animation)) item["duration"]=variable->gameDuration();
        if(auto *variable=qobject_cast<QVariantAnimation*>(animation)) {
            QJsonArray keys;
            for(const auto& key : variable->keyValues()) keys.append(QJsonArray{key.first,variant(key.second)});
            item["keys"]=keys; item["easing"]=int(variable->easingCurve().type());
        }
        animations.append(item);
    }
    return {{"timers",timers},{"animations",animations}};
}
void restoreActivities(QObject *root,const QJsonObject& saved,int rate) {
    const auto timers=root->findChildren<GameTimer*>(QString(),Qt::FindDirectChildrenOnly);
    const auto timerStates=saved["timers"].toArray();
    for(int i=0;i<timers.size();++i) {
        const auto state=timerStates[i].toObject();
        timers[i]->restoreTiming(state["interval"].toInt(),state["active"].toBool(),state["remaining"].toDouble());
    }
    const auto animations=root->findChildren<QAbstractAnimation*>(QString(),Qt::FindDirectChildrenOnly);
    const auto animationStates=saved["animations"].toArray();
    for(int i=0;i<animations.size();++i) {
        auto *animation=animations[i]; const auto state=animationStates[i].toObject();
        const QSignalBlocker blocker(animation);
        animation->stop(); animation->setCurrentTime(0);
        animation->setLoopCount(state["loops"].toInt());
        if(auto *property=dynamic_cast<GamePropertyAnimation*>(animation)) property->setDuration(state["duration"].toInt());
        else if(auto *variable=dynamic_cast<GameVariantAnimation*>(animation)) variable->setDuration(state["duration"].toInt());
        if(auto *variable=qobject_cast<QVariantAnimation*>(animation)) {
            QVariantAnimation::KeyValues keys;
            for(const auto key : state["keys"].toArray()) {
                const auto pair=key.toArray(); keys.append({pair[0].toDouble(),value(pair[1])});
            }
            variable->setKeyValues(keys);
            variable->setEasingCurve(QEasingCurve::Type(state["easing"].toInt()));
        }
        if(state["resume"].toBool() || state["state"].toInt()==QAbstractAnimation::Paused) animation->start();
        animation->setCurrentTime(qRound(state["time"].toDouble()/rate));
        if(!state["resume"].toBool() && animation->state()==QAbstractAnimation::Running) animation->pause();
    }
}
QJsonObject graphic(QGraphicsItem *item,QObject *owner,const GamePause& pause,int rate) {
    return {{"pos",point(item->pos())},{"opacity",item->opacity()},
            {"activity",activities(owner,pause,rate)},{"name",owner->objectName()}};
}
void restoreGraphic(QGraphicsItem *item,QObject *owner,const QJsonObject& saved,int rate) {
    restoreActivities(owner,saved["activity"].toObject(),rate);
    owner->setObjectName(saved["name"].toString());
    item->setPos(position(saved["pos"])); item->setOpacity(saved["opacity"].toDouble());
}
bool integer(const QJsonValue& v,int low,int high) {
    return v.isDouble() && std::isfinite(v.toDouble()) && v.toDouble()==v.toInt(low-1) && v.toDouble()>=low && v.toDouble()<=high;
}
bool validPoint(const QJsonValue& v) {
    const auto a=v.toArray();
    return a.size()==2 && a[0].isDouble() && a[1].isDouble()
        && std::isfinite(a[0].toDouble()) && std::isfinite(a[1].toDouble())
        && qAbs(a[0].toDouble())<100000 && qAbs(a[1].toDouble())<100000;
}
bool validActivities(const QJsonValue& v) {
    if(!v.isObject()) return false;
    const auto s=v.toObject();
    if(!s["timers"].isArray() || !s["animations"].isArray()
        || s["timers"].toArray().size()>16 || s["animations"].toArray().size()>16) return false;
    for(const auto t : s["timers"].toArray()) {
        const auto o=t.toObject();
        if(!integer(o["interval"],0,3600000) || !o["active"].isBool()
            || !o["remaining"].isDouble() || o["remaining"].toDouble()<0
            || o["remaining"].toDouble()>3600000) return false;
    }
    for(const auto a : s["animations"].toArray()) {
        const auto o=a.toObject();
        if(!integer(o["state"],0,2) || !o["resume"].isBool() || !integer(o["loops"],-1,10000)
            || o["loops"].toInt()==0 || !integer(o["time"],0,1000000000)) return false;
        if(o.contains("duration") && !integer(o["duration"],1,3600000)) return false;
        if(o.contains("keys")) {
            if(!integer(o["easing"],0,QEasingCurve::NCurveTypes-1) || o["keys"].toArray().size()>32) return false;
            for(const auto key : o["keys"].toArray()) {
                const auto pair=key.toArray();
                if(pair.size()!=2 || !pair[0].isDouble() || pair[0].toDouble()<0 || pair[0].toDouble()>1
                    || !(pair[1].isDouble() || pair[1].isNull() || (pair[1].isObject() && validPoint(pair[1].toObject()["point"])))) return false;
            }
        }
    }
    return true;
}
bool validGraphic(const QJsonObject& item) {
    return validPoint(item["pos"]) && item["opacity"].isDouble()
        && item["opacity"].toDouble()>=0 && item["opacity"].toDouble()<=1 && validActivities(item["activity"]);
}
bool activityCount(const QJsonValue& v,int timers,int animations) {
    const auto o=v.toObject();
    return o["timers"].toArray().size()==timers && o["animations"].toArray().size()==animations;
}
bool roster(const QJsonValue& v) {
    if(!v.isArray() || v.toArray().size()>20000) return false;
    for(const auto type : v.toArray()) if(!integer(type,0,2)) return false;
    return true;
}
}

QJsonObject BattleSnapshot::capture(PlayScene& play) {
    auto& scene=*play.myGameScene;
    if(play.finished || !scene.started) return {};
    play.gamePaused();
    const int rate=play.speedMultiplier();
    QJsonArray plan,rows,plants,enemies,hearts,bullets,notes,effects,cards;
    for(const auto& wave : scene.wavePlan) plan.append(integers(wave));
    for(int count : scene.waveRowCounts) rows.append(count);
    const QStringList plantClasses{"SingingWhite","HeartWhite","WallWhite","LineWhite","DancingWhite","AllHeartWhite","DblSingWhite","MoneyWhite"};
    for(auto *plant : scene.findChildren<WhiteDogs*>()) {
        if(plant->scene()!=&scene) continue;
        auto item=graphic(plant,plant,play.pausedActivity,rate);
        item["type"]=plantClasses.indexOf(plant->metaObject()->className());
        item["row"]=plant->itRow; item["col"]=plant->itCol; item["hp"]=plant->hp;
        item["moving"]=plant->memIsMoving;
        item["occupied"]=scene.dogMap[plant->itRow*9+plant->itCol]==plant;
        if(auto *singer=qobject_cast<SingingWhite*>(plant)) item["shooting"]=singer->isZombieOnYourLawn;
        if(auto *singer=qobject_cast<DblSingWhite*>(plant)) item["shooting"]=singer->isZombieOnYourLawn;
        if(auto *charger=qobject_cast<LineWhite*>(plant)) item["vacated"]=charger->plantingCellVacated;
        plants.append(item);
    }
    for(auto *enemy : scene.findChildren<YellowDogs*>()) {
        if(enemy->scene()!=&scene) continue;
        auto item=graphic(enemy,enemy,play.pausedActivity,rate);
        item["type"]=enemy->enemyType; item["row"]=enemy->itRow; item["hp"]=enemy->hp;
        item["initialHp"]=enemy->initialHealth; item["initialSpeed"]=enemy->initialSpeed;
        item["speed"]=enemy->speed; item["attack"]=enemy->atkPower;
        item["accelerated"]=enemy->dashAccelerated; item["dying"]=enemy->removed;
        item["moving"]=enemy->memIsMoving; item["hit"]=enemy->m_hitFlash;
        item["bite"]=enemy->m_biteProgress; item["death"]=enemy->m_deathProgress;
        enemies.append(item);
    }
    for(auto *heart : scene.findChildren<Heart*>()) {
        if(heart->scene()!=&scene) continue;
        auto item=graphic(heart,heart,play.pausedActivity,rate);
        item["end"]=point(heart->memEndPos); item["collectable"]=heart->isCollectable;
        item["disappearing"]=heart->isDisappearing; hearts.append(item);
    }
    for(auto *bullet : scene.findChildren<Bullet*>()) {
        if(bullet->scene()!=&scene) continue;
        auto item=graphic(bullet,bullet,play.pausedActivity,rate); item["row"]=bullet->itRow; bullets.append(item);
    }
    for(auto *note : scene.findChildren<EnemyProjectile*>()) {
        if(note->scene()!=&scene) continue;
        auto item=graphic(note,note,play.pausedActivity,rate);
        item["row"]=note->m_row; item["damage"]=note->m_damage; notes.append(item);
    }
    for(auto *effect : scene.findChildren<CombatEffect*>()) {
        if(effect->scene()!=&scene) continue;
        auto item=graphic(effect,effect,play.pausedActivity,rate);
        item["kind"]=int(effect->m_kind); item["progress"]=effect->m_progress; effects.append(item);
    }
    for(auto *card : play.myCards)
        cards.append(QJsonObject{{"cooling",card->coolingState},{"progress",card->memCoolProgress},
                                 {"activity",activities(card,play.pausedActivity,rate)}});
    const QJsonObject state{{"hearts",scene.restHeart},{"chosen",scene.chosenNum},{"nextWave",scene.nextWave},
        {"plan",plan},{"pending",integers(scene.pendingWave)},{"delays",integers(scene.pendingSpawnDelays)},
        {"pendingIndex",scene.pendingIndex},{"rows",rows},{"total",scene.m_totalZombiesForLevel},
        {"spawned",scene.m_zombiesSpawned},{"killed",scene.m_zombiesKilled},
        {"shovelPos",point(scene.shovel ? scene.shovel->pos() : QPointF{})},
        {"showPlants",scene.showPlantHealth},{"showEnemies",scene.showEnemyHealth},
        {"activity",activities(&scene,play.pausedActivity,rate)}};
    QJsonObject glove;
    if(scene.glove) glove={{"pos",point(scene.glove->pos())},
        {"row",scene.plantToMove ? scene.plantToMove->getItRow() : -1},
        {"col",scene.plantToMove ? scene.plantToMove->getItCol() : -1},
        {"frame",scene.plantToMove ? encodedFrame(scene.plantGhost->pixmap()) : QString{}}};
    return {{"format",1},{"level",play.levelIndex},{"endless",play.endlessMode},{"speed",rate},{"glove",glove},
        {"scene",state},{"plants",plants},{"enemies",enemies},{"hearts",hearts},
        {"bullets",bullets},{"notes",notes},{"effects",effects},{"cards",cards},
        {"selection",int(play.interactionBeforePause)},{"selectedWhite",Card::selectedWhite()},
        {"preview",play.previewBeforePause},{"previewPos",point(play.previewScenePosition)},
        {"banner",QJsonObject{{"visible",!play.banner->isHidden()},{"text",play.banner->message},
            {"flash",play.banner->flash},{"progress",play.banner->progress},
            {"activity",activities(play.banner,play.pausedActivity,rate)}}}};
}

bool BattleSnapshot::isValid(const QJsonObject& state) {
    if(state["format"].toInt()!=1 || !integer(state["level"],1,10) || !state["endless"].isBool()
        || !integer(state["speed"],1,2)
        || !(integer(state["selection"],0,2) || integer(state["selection"],int(GameState::MovingPlant),int(GameState::MovingPlant)))
        || !state["preview"].isBool() || !validPoint(state["previewPos"])) return false;
    const auto s=state["scene"].toObject();
    if(!integer(s["hearts"],0,1000000000) || !integer(s["chosen"],0,7)
        || !integer(s["nextWave"],0,100000000) || !roster(s["pending"])
        || !integer(s["pendingIndex"],0,s["pending"].toArray().size())
        || !integer(s["total"],0,100000000) || !integer(s["spawned"],0,100000000)
        || !integer(s["killed"],0,s["spawned"].toInt()) || !validActivities(s["activity"])
        || !s["showPlants"].isBool() || !s["showEnemies"].isBool()
        || !activityCount(s["activity"],5,0) || !validPoint(s["shovelPos"])) return false;
    const bool endless=state["endless"].toBool();
    const auto plan=s["plan"].toArray(); int total=0;
    if(plan.size()!=(endless ? 0 : GameCatalog::level(state["level"].toInt()).waves)) return false;
    for(const auto wave : plan) { if(!roster(wave)) return false; total+=wave.toArray().size(); }
    if(s["total"].toInt()!=total || (!endless && (s["nextWave"].toInt()>plan.size() || s["spawned"].toInt()>total))) return false;
    if(s["rows"].toArray().size()!=5) return false;
    for(const auto n : s["rows"].toArray()) if(!integer(n,0,20000)) return false;
    const auto delays=s["delays"].toArray();
    if(delays.size()!=(endless ? s["pending"].toArray().size() : 0)) return false;
    for(const auto n : delays) if(!integer(n,0,3600000)) return false;
    for(const auto& key : {"plants","enemies","hearts","bullets","notes","effects"}) {
        if(!state[key].isArray() || state[key].toArray().size()>20000) return false;
        for(const auto v : state[key].toArray()) if(!validGraphic(v.toObject())) return false;
    }
    if(state["enemies"].toArray().size()!=s["spawned"].toInt()-s["killed"].toInt()) return false;
    for(const auto v : state["plants"].toArray()) {
        const auto p=v.toObject();
        if(!integer(p["type"],0,7) || !integer(p["row"],0,4) || !integer(p["col"],0,8)
            || !integer(p["hp"],1,GameCatalog::plants()[p["type"].toInt()].health)
            || !p["moving"].isBool() || !p["occupied"].isBool()) return false;
        const int type=p["type"].toInt();
        if(!activityCount(p["activity"],type==6 ? 2 : (type==0 || type==1 || type==5 ? 1 : 0),type==3 ? 2 : 1)) return false;
    }
    const bool moving=state["selection"].toInt()==int(GameState::MovingPlant);
    const auto glove=state["glove"].toObject();
    if(moving && (!endless || glove.isEmpty())) return false;
    if(!glove.isEmpty()) {
        if(!endless || !validPoint(glove["pos"]) || !integer(glove["row"],-1,4)
            || !integer(glove["col"],-1,8) || !glove["frame"].isString()) return false;
        const int row=glove["row"].toInt(),col=glove["col"].toInt();
        if((row==-1)!=(col==-1)) return false;
        if(row>=0) {
            if(!moving || decodedFrame(glove["frame"]).isNull()) return false;
            int matches=0;
            for(const auto v : state["plants"].toArray()) {
                const auto p=v.toObject();
                if(p["row"].toInt()==row && p["col"].toInt()==col && p["occupied"].toBool()) {
                    if(p["type"].toInt()==3) return false;
                    ++matches;
                }
            }
            if(matches!=1) return false;
        } else if(!glove["frame"].toString().isEmpty()) return false;
    }
    for(const auto v : state["enemies"].toArray()) {
        const auto e=v.toObject();
        if(!integer(e["type"],0,2) || !integer(e["row"],0,4) || !integer(e["initialHp"],1,100000)
            || !integer(e["hp"],0,e["initialHp"].toInt()) || !integer(e["attack"],1,100000)
            || !e["dying"].isBool() || !e["moving"].isBool() || !e["accelerated"].isBool()
            || e["speed"].toDouble(-1)<=0 || e["speed"].toDouble()>1000
            || e["initialSpeed"].toDouble(-1)<=0 || e["initialSpeed"].toDouble()>1000
            || !activityCount(e["activity"],e["type"].toInt()==0 ? 0 : 1,6)) return false;
    }
    for(const auto v : state["hearts"].toArray()) {
        const auto h=v.toObject();
        if(!validPoint(h["end"]) || !h["collectable"].isBool() || !h["disappearing"].isBool()
            || !activityCount(h["activity"],1,h["disappearing"].toBool() ? 4 : 3)) return false;
    }
    for(const auto& key : {"bullets","notes"}) for(const auto v : state[key].toArray())
        if(!integer(v.toObject()["row"],0,4) || !activityCount(v.toObject()["activity"],QString(key)=="notes" ? 1 : 0,1)) return false;
    for(const auto v : state["notes"].toArray()) if(!integer(v.toObject()["damage"],1,100000)) return false;
    for(const auto v : state["effects"].toArray())
        if(!integer(v.toObject()["kind"],0,3) || !activityCount(v.toObject()["activity"],0,1)) return false;
    const auto cards=state["cards"].toArray();
    if(cards.size()!=(endless ? 8 : qMin(state["level"].toInt(),8))) return false;
    for(const auto v : cards) {
        const auto c=v.toObject();
        if(!c["cooling"].isBool() || !c["progress"].isDouble() || c["progress"].toDouble()<0
            || c["progress"].toDouble()>1 || !validActivities(c["activity"]) || !activityCount(c["activity"],1,0)) return false;
    }
    const auto banner=state["banner"].toObject();
    return banner["visible"].isBool() && validActivities(banner["activity"]) && activityCount(banner["activity"],0,1);
}

bool BattleSnapshot::restore(PlayScene& play,const QJsonObject& state) {
    if(!isValid(state) || state["level"].toInt()!=play.levelIndex || state["endless"].toBool()!=play.endlessMode) return false;
    auto& scene=*play.myGameScene; const auto s=state["scene"].toObject();
    const int rate=state["speed"].toInt(); play.setSpeedMultiplier(rate);
    scene.wavePlan.clear(); for(const auto wave : s["plan"].toArray()) scene.wavePlan.append(numbers(wave));
    scene.nextWave=s["nextWave"].toInt(); scene.pendingWave=numbers(s["pending"]);
    scene.pendingSpawnDelays=numbers(s["delays"]); scene.pendingIndex=s["pendingIndex"].toInt();
    const auto rows=s["rows"].toArray(); for(int i=0;i<5;++i) scene.waveRowCounts[i]=rows[i].toInt();
    scene.m_totalZombiesForLevel=s["total"].toInt();
    scene.showPlantHealth=s["showPlants"].toBool(); scene.showEnemyHealth=s["showEnemies"].toBool();
    QVector<QPair<WhiteDogs*,bool>> occupancy;
    for(const auto v : state["plants"].toArray()) {
        const auto p=v.toObject(); const int row=p["row"].toInt(),col=p["col"].toInt();
        scene.chosenNum=p["type"].toInt(); scene.placePlant(row,col,false);
        auto *plant=scene.dogMap[row*9+col];
        plant->hp=p["hp"].toInt(); plant->memIsMoving=p["moving"].toBool();
        if(auto *singer=qobject_cast<SingingWhite*>(plant)) singer->isZombieOnYourLawn=p["shooting"].toBool();
        if(auto *singer=qobject_cast<DblSingWhite*>(plant)) singer->isZombieOnYourLawn=p["shooting"].toBool();
        if(auto *charger=qobject_cast<LineWhite*>(plant)) charger->plantingCellVacated=p["vacated"].toBool();
        restoreGraphic(plant,plant,p,rate);
        plant->setPixmap(plant->movie->currentPixmap()); plant->updateHealthLabel();
        occupancy.append({plant,p["occupied"].toBool()});
    }
    scene.dogMap.fill(nullptr);
    for(const auto& entry : occupancy) if(entry.second) scene.dogMap[entry.first->itRow*9+entry.first->itCol]=entry.first;
    for(const auto v : state["enemies"].toArray()) {
        const auto e=v.toObject(); const int row=e["row"].toInt();
        scene.setAYellowDog(row,e["type"].toInt()); auto *enemy=static_cast<YellowDogs*>(scene.zombieMap[row].last());
        enemy->hp=e["hp"].toInt(); enemy->initialHealth=e["initialHp"].toInt(); enemy->initialSpeed=e["initialSpeed"].toDouble();
        enemy->speed=e["speed"].toDouble(); enemy->atkPower=e["attack"].toInt();
        enemy->dashAccelerated=e["accelerated"].toBool(); enemy->memIsMoving=e["moving"].toBool();
        enemy->prepareGeometryChange(); enemy->removed=e["dying"].toBool();
        restoreGraphic(enemy,enemy,e,rate); enemy->setPixmap(enemy->movie->currentPixmap());
        enemy->m_hitFlash=e["hit"].toDouble(); enemy->m_biteProgress=e["bite"].toDouble(); enemy->m_deathProgress=e["death"].toDouble();
        enemy->setHealthVisible(scene.showEnemyHealth && !enemy->removed);
        if(enemy->removed) scene.zombieMap[row].removeOne(enemy);
    }
    for(const auto v : state["hearts"].toArray()) {
        const auto h=v.toObject(); auto *heart=new Heart(position(h["pos"]),position(h["end"]),&scene,QEasingCurve::Linear,nullptr,true);
        scene.addHeartItem(heart);
        if(h["disappearing"].toBool()) heart->hasDisappear();
        heart->isCollectable=h["collectable"].toBool(); restoreGraphic(heart,heart,h,rate);
    }
    for(const auto v : state["bullets"].toArray()) {
        const auto b=v.toObject(); auto *bullet=new Bullet(b["row"].toInt(),0,scene.memGameTimer);
        bullet->setParent(&scene); scene.addItem(bullet); restoreGraphic(bullet,bullet,b,rate);
    }
    for(const auto v : state["notes"].toArray()) {
        const auto n=v.toObject(); auto *note=new EnemyProjectile(&scene,n["row"].toInt(),position(n["pos"]),n["damage"].toInt());
        restoreGraphic(note,note,n,rate);
    }
    for(const auto v : state["effects"].toArray()) {
        const auto e=v.toObject(); auto *effect=new CombatEffect(&scene,position(e["pos"]),CombatEffect::Kind(e["kind"].toInt()));
        restoreGraphic(effect,effect,e,rate); effect->m_progress=e["progress"].toDouble();
    }
    scene.restHeart=s["hearts"].toInt(); scene.chosenNum=s["chosen"].toInt();
    if(scene.shovel) scene.shovel->setPos(position(s["shovelPos"]));
    scene.m_zombiesSpawned=s["spawned"].toInt(); scene.m_zombiesKilled=s["killed"].toInt();
    restoreActivities(&scene,s["activity"].toObject(),rate);
    Card::setCurRestHeart(scene.restHeart); play.restHeartLabel->setText(QString::number(scene.restHeart));
    const auto cards=state["cards"].toArray();
    for(int i=0;i<play.myCards.size();++i) {
        auto *card=play.myCards[i]; const auto c=cards[i].toObject();
        card->coolingState=c["cooling"].toBool(); card->memCoolProgress=c["progress"].toDouble();
        restoreActivities(card,c["activity"].toObject(),rate); card->refreshAvailability();
    }
    const auto banner=state["banner"].toObject();
    play.banner->message=banner["text"].toString(); play.banner->flash=banner["flash"].toBool();
    play.banner->setAccessibleName(play.banner->message);
    restoreActivities(play.banner,banner["activity"].toObject(),rate);
    play.banner->progress=banner["progress"].toDouble(); play.banner->setVisible(banner["visible"].toBool());
    Card::setSelectedWhite(state["selectedWhite"].toString()); Card::setGameState(GameState(state["selection"].toInt()));
    if(Card::currentState()==GameState::MovingPlant) {
        const auto glove=state["glove"].toObject();
        if(glove["row"].toInt()>=0) {
            scene.selectPlantToMove(scene.dogMap[glove["row"].toInt()*9+glove["col"].toInt()]);
            scene.plantGhost->setPixmap(decodedFrame(glove["frame"]));
        }
        const auto size=scene.glove->pixmap().size();
        scene.updateGlovePosition(position(glove["pos"])+QPointF(size.width()/2.0,size.height()/2.0));
    }
    play.previewScenePosition=position(state["previewPos"]); play.fitBattlefield();
    play.preImageLabel->setVisible(state["preview"].toBool());
    play.showPauseMenu();
    return true;
}
