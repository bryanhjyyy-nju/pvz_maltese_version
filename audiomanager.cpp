#include "audiomanager.h"
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
    for(const auto& name : {"pause","click","plant","collect","shoot","hit","shovel","win","lose"})
        m_effects.insert(name,create(name));
    updateMusic();
}
void AudioManager::play(const QString& name) {
    auto *sound = m_effects.value(name,nullptr);
    if(!sound || m_effectsVolume == 0 || sound->status() != QSoundEffect::Ready) return;
    sound->setVolume(m_effectsVolume/100.0);
    // Avoid restarting a sound on every simultaneous collision.
    if(!sound->isPlaying()) sound->play();
}
void AudioManager::setBattle(bool battle) { m_inBattle = battle; m_paused = false; updateMusic(); }
void AudioManager::setPaused(bool paused) { m_paused = paused; updateMusic(); }
void AudioManager::updateMusic() {
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
    dialog.setMinimumWidth(400);
    QFormLayout layout(&dialog);
    auto addSlider = [&](const QString& label, int value, bool music) {
        auto *slider = new QSlider(Qt::Horizontal,&dialog);
        slider->setRange(0,100); slider->setValue(value);
        layout.addRow(label,slider);
        connect(slider,&QSlider::valueChanged,&dialog,[this,music](int v) {
            if(music) setMusicVolume(v); else setEffectsVolume(v);
        });
    };
    addSlider("背景音乐",m_musicVolume,true);
    addSlider("游戏音效",m_effectsVolume,false);
    layout.addRow(new QLabel("拖到最左侧即可静音；设置会自动保存。",&dialog));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close,&dialog);
    buttons->button(QDialogButtonBox::Close)->setText("关闭");
    connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    layout.addRow(buttons);
    dialog.exec();
}
