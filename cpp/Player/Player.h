#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop>
#include <QFile>

#include <QTimer>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetails.h"

#define REFRESH_DISPLAY_POSITION_MS 1000

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool         isPlaying       READ getIsPlaying                       NOTIFY playingChanged       FINAL)

    Q_PROPERTY(qsizetype    songID          READ getSongID                          NOTIFY songDataChanged      FINAL)
    Q_PROPERTY(QString      title           READ getTitle                           NOTIFY songDataChanged      FINAL)
    Q_PROPERTY(QString      thumbnail       READ getThumbnail                       NOTIFY songDataChanged      FINAL)

    Q_PROPERTY(qsizetype    realDuration    READ getRealDuration                    NOTIFY songDataChanged      FINAL)
    Q_PROPERTY(qsizetype    begin           READ getBegin                           NOTIFY songDataChanged      FINAL)
    Q_PROPERTY(qsizetype    position        READ getPosition    WRITE setPosition   NOTIFY songPositionChanged  FINAL)

    Q_PROPERTY(QString      displayDuration     READ getDisplayDuration     NOTIFY displayDurationChanged   FINAL)
    Q_PROPERTY(QString      displayPosition     READ getDisplayPosition     NOTIFY displayPositionChanged   FINAL)
public:
    explicit Player(QObject *parent = nullptr);

    void buildParametersConnections();

public slots:
    void play();
    void nextSong();
    void restartSong();
    void setVolume(int volume);

    void changeSong(const SongDetails *song); /// emited by Playlist, only when Player ask to (by emiting songEnded)
    void resetPlayer();

signals:
    void songEnded();           /// emited when player finish playing the song

    void songChanged();         /// emited by changeSong
    void songDataChanged();
    void playingChanged();      ///
    // void songStarted();
    void songPositionChanged();
    void displayDurationChanged();
    void displayPositionChanged();

public slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updatePlayer();        /// truggered by songChanged
    void updatePlayerProgress(qsizetype position);

private: // support methods
    QString getSongTagValueByID(qsizetype id) const;
    QString validThumbnailPath(QString thumbnail) const;
    static QString createDisplayTime(qsizetype time);

public: // qml getters
    bool getIsPlaying() const;
    qsizetype getSongID() const;
    QString getTitle() const;
    QString getThumbnail() const;
    qsizetype getRealDuration() const;
    qsizetype getBegin() const;
    qsizetype getPosition() const;
    QString getDisplayDuration() const;
    QString getDisplayPosition() const;

    void setPosition(qsizetype position);

private:
    bool m_playerStarted;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    SongDetails *m_song;

    struct{
        qsizetype songID;
        QString title;
        QString thumbnail;
        qsizetype begin;
        qsizetype end;
        qsizetype realDuration;
        qsizetype position;
    } m_songData;

    qsizetype m_lastUpdatedDisplayValues;
};

#endif // PLAYER_H
