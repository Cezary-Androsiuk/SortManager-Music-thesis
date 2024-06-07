#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetails.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);

public slots:
    void play();
    void pause();
    void nextSong();

    void changeSong(const SongDetails *song); /// emited by Playlist, only when Player ask to (by emiting songEnded)
    void resetPlayer();

private:
    void updatePlayer();        /// truggered by songChanged
    void startPlayer();         /// called by updatePlayer

signals:
    void songEnded();           /// emited when player finish playing the song
    void songChanged();         /// emited by changeSongToNext

private: // support methods
    QString getSongTagValueByID(qsizetype id) const;

private:
    bool m_playerStarted;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    SongDetails *m_song;


};

#endif // PLAYER_H
