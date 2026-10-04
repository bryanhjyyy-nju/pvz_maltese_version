#pragma once
#include <QObject>
#include <QSoundEffect>
#include <QHash>

class QWidget;
class AudioManager : public QObject {
public:
    static AudioManager& instance();
    void stopAll();
    void stopMusic();
    void playVictory();
    void play(const QString& name);
    void setBattle(bool battle);
    void setPaused(bool paused);
    void showSettings(QWidget *parent);
    void setMusicVolume(int volume);
    void setEffectsVolume(int volume);
    int musicVolume() const { return m_musicVolume; }
    int effectsVolume() const { return m_effectsVolume; }
private:
    explicit AudioManager(QObject *parent);
    void updateMusic();
    QSoundEffect *m_menu, *m_battle;
    QHash<QString,QSoundEffect*> m_effects;
    int m_musicVolume = 30, m_effectsVolume = 60;
    bool m_inBattle = false, m_paused = false, m_stopped = false, m_result=false;
};
