#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop>
#include <QFile>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetails.h"

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying   READ getIsPlaying   NOTIFY playingChanged   FINAL)

    Q_PROPERTY(QString title        READ getTitle       NOTIFY songChanged      FINAL)
    Q_PROPERTY(QString thumbnail    READ getThumbnail   NOTIFY songChanged      FINAL)
public:
    explicit Player(QObject *parent = nullptr);

    void buildParametersConnections();

public slots:
    void play();
    // void pause();
    void nextSong();
    void restartSong();

    void changeSong(const SongDetails *song); /// emited by Playlist, only when Player ask to (by emiting songEnded)
    void resetPlayer();

signals:
    void songEnded();           /// emited when player finish playing the song

    void songChanged();         /// emited by changeSongToNext
    void playingChanged();      ///

public slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updatePlayer();        /// truggered by songChanged
    void updatePlayerProgress(qsizetype position);

private: // support methods
    QString getSongTagValueByID(qsizetype id) const;
    QString validThumbnailPath(QString thumbnail) const;

public: // qml getters
    bool getIsPlaying() const;
    QString getTitle() const;
    QString getThumbnail() const;

private:
    bool m_playerStarted;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    SongDetails *m_song;

    struct{
        QString title;
        QString thumbnail;
        qsizetype begin;
        qsizetype end;
    } m_songData;

};

#endif // PLAYER_H
