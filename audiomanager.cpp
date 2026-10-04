#include "audiomanager.h"
#include "gameui.h"
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QSlider>

AudioManager& AudioManager::instance() {
    static auto *manager = new AudioManager(qApp);
    return *manager;
}
AudioManager::AudioManager(QObject *parent) : QObject(parent) {
    QSettings settings;
    m_musicVolume = qBound(0,settings.value("audio/music",30).toInt(),100);
    m_effectsVolume = qBound(0,settings.value("audio/effects",60).toInt(),100);
    auto create = [this](const QString& name) {
        auto *sound = new QSoundEffect(this);
        sound->setSource(QUrl("qrc:/audio/Media/" + name + ".wav"));
        return sound;
    };
    m_menu = create("menu"); m_battle = create("battle");
    for(auto *sound : {m_menu,m_battle}) {
        sound->setLoopCount(QSoundEffect::Infinite);
        connect(sound,&QSoundEffect::statusChanged,this,[this]{ updateMusic(); });
    }
    for(const auto& name : {"pause","click","plant","hit","shovel","win","lose",
                           "bite","shovelPickup","shovelPutdown","uproot","guitarHit","readyImpact","finalWave"})
        m_effects.insert(name,create(name));
    updateMusic();
}
void AudioManager::play(const QString& name) {
    auto *sound = m_effects.value(name,nullptr);
    if(m_stopped || !sound || m_effectsVolume == 0 || sound->status() != QSoundEffect::Ready) return;
    sound->setVolume(m_effectsVolume/100.0);
    // Avoid restarting a sound on every simultaneous collision.
    if(!sound->isPlaying()) sound->play();
}
void AudioManager::setBattle(bool battle) {
    m_effects.value("win")->stop();
    m_result=false; m_stopped=false; m_inBattle = battle; m_paused = false; updateMusic();
}
void AudioManager::stopMusic() { m_result=true; m_menu->stop(); m_battle->stop(); }
void AudioManager::playVictory() {
    auto *music=m_effects.value("win");
    if(m_stopped || m_musicVolume==0 || music->status()!=QSoundEffect::Ready) return;
    music->setVolume(m_musicVolume/100.0); music->play();
}
void AudioManager::setPaused(bool paused) { m_paused = paused; updateMusic(); }
void AudioManager::updateMusic() {
    if(m_stopped || m_result) return;
    auto *active = m_inBattle ? m_battle : m_menu;
    auto *inactive = m_inBattle ? m_menu : m_battle;
    inactive->stop();
    active->setVolume(m_musicVolume/100.0 * (m_paused ? 0.35 : 1.0));
    if(m_musicVolume == 0) active->stop();
    else if(active->status() == QSoundEffect::Ready && !active->isPlaying()) active->play();
}
void AudioManager::setMusicVolume(int volume) {
    m_musicVolume = qBound(0,volume,100);
    QSettings().setValue("audio/music",m_musicVolume);
    updateMusic();
}
void AudioManager::setEffectsVolume(int volume) {
    m_effectsVolume = qBound(0,volume,100);
    QSettings().setValue("audio/effects",m_effectsVolume);
    for(auto *sound : m_effects) sound->setVolume(m_effectsVolume/100.0);
}
void AudioManager::showSettings(QWidget *parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle("声音设置");
    dialog.setMinimumWidth(520);
    GameUi::apply(&dialog);
    QFormLayout layout(&dialog);
    layout.setContentsMargins(30,25,30,25);
    layout.setVerticalSpacing(22);
    auto *title = new QLabel("让草坪唱起来！",&dialog);
    title->setStyleSheet("font-size:26px; font-weight:bold; color:#805024;");
    layout.addRow(title);
    auto addSlider = [&](const QString& label, int value, bool music) {
        auto *slider = new QSlider(Qt::Horizontal,&dialog);
        slider->setRange(0,100); slider->setValue(value);
        auto *valueLabel = new QLabel(QString("%1  %2%").arg(label).arg(value),&dialog);
        valueLabel->setMinimumWidth(150);
        layout.addRow(valueLabel,slider);
        connect(slider,&QSlider::valueChanged,valueLabel,[valueLabel,label](int v) {
            valueLabel->setText(QString("%1  %2%").arg(label).arg(v));
        });
        connect(slider,&QSlider::valueChanged,&dialog,[this,music](int v) {
            if(music) setMusicVolume(v); else setEffectsVolume(v);
        });
    };
    addSlider("背景音乐",m_musicVolume,true);
    addSlider("游戏音效",m_effectsVolume,false);
    layout.addRow(new QLabel("拖到最左侧即可静音；设置会自动保存。",&dialog));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close,&dialog);
    buttons->button(QDialogButtonBox::Close)->setText("关闭");
    GameUi::styleButton(buttons->button(QDialogButtonBox::Close),"gold");
    connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    layout.addRow(buttons);
    dialog.exec();
}

void AudioManager::stopAll() {
    m_stopped=true;
    m_menu->stop(); m_battle->stop();
    for(auto *sound : m_effects) sound->stop();
}
