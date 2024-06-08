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
    Q_PROPERTY(QString title    READ getTitle   NOTIFY songChanged FINAL)
public:
    explicit Player(QObject *parent = nullptr);

    void buildParametersConnections();

public slots:
    void play();
    void pause();
    void nextSong();

    void changeSong(const SongDetails *song); /// emited by Playlist, only when Player ask to (by emiting songEnded)
    void resetPlayer();

signals:
    void songEnded();           /// emited when player finish playing the song
    void songChanged();         /// emited by changeSongToNext

public slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updatePlayer();        /// truggered by songChanged

private: // support methods
    QString getSongTagValueByID(qsizetype id) const;

public: // songData getters
    QString getTitle() const;

private:
    bool m_playerStarted;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    SongDetails *m_song;

    struct{
        QString title;
        QUrl thumbnail;
    } m_songData;

};

#endif // PLAYER_H
