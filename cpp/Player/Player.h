#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop>

#include "cpp/DebugPrint/DebugPrint.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);

public slots:
    void play();
    void pause();
    void play2();
    void pause2();

signals:

private:
    QMediaPlayer *m_player;
    QMediaPlayer *m_player_2;
    QAudioOutput *m_audio_output;
    QAudioOutput *m_audio_output_2;
};

#endif // PLAYER_H
