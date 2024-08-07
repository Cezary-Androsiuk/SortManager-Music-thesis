#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop>
#include <QFile>
#include <QImage>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetails.h"

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool         isPlaying       READ getIsPlaying                       NOTIFY playingChanged       FINAL)
    Q_PROPERTY(bool         isPlayerEmpty   READ getIsPlayerEmpty                   NOTIFY isPlayerEmptyChanged FINAL)

    Q_PROPERTY(qsizetype    songID          READ getSongID                          NOTIFY songLoaded           FINAL)
    Q_PROPERTY(QString      title           READ getTitle                           NOTIFY songLoaded           FINAL)
    Q_PROPERTY(QString      thumbnail       READ getThumbnail                       NOTIFY songLoaded           FINAL)
    Q_PROPERTY(int          thumbnailWidth  READ getThumbnailWidth                  NOTIFY songLoaded           FINAL)
    Q_PROPERTY(int          thumbnailHeight READ getThumbnailHeight                 NOTIFY songLoaded           FINAL)
    Q_PROPERTY(qsizetype    duration        READ getDuration                        NOTIFY songLoaded           FINAL)
    Q_PROPERTY(qsizetype    position        READ getPosition    WRITE setPosition   NOTIFY songPositionChanged  FINAL)

    Q_PROPERTY(QString      displayDuration READ getDisplayDuration             NOTIFY displayDurationChanged   FINAL)
    Q_PROPERTY(QString      displayPosition READ getDisplayPosition             NOTIFY displayPositionChanged   FINAL)
public:
    explicit Player(QObject *parent = nullptr);

    void buildParametersConnections();

public slots:
    void play();
    void nextSong();
    void prevSong();
    void setVolume(int volume);

    void changeSong(const SongDetails *song); /// emited by Playlist, only when Player ask to (by emiting songEnded)
    void clearPlayerNoSong();
    void resetPlayer();

signals:
    void songEnded();           /// emited when player finish playing the song
    void songPrevious();

    void songChanged();         /// emited by changeSongToNext
    void songLoaded();
    void playingChanged();      ///
    void songStarted();
    void songPositionChanged();
    void displayDurationChanged();
    void displayPositionChanged();
    void isPlayerEmptyChanged();

public slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updateDisplayPosition(qint64 position);
    void updateDisplayDuration(qint64 duration);

private: // support methods
    QString getSongTagValueByID(SongDetails *song, qsizetype id) const;
    QString validThumbnailPath(QString thumbnail) const;
    void readImageSize();
    static QString createDisplayTime(qsizetype time);

public: // qml getters
    bool getIsPlaying() const;
    qsizetype getSongID() const;
    QString getTitle() const;
    QString getThumbnail() const;
    int getThumbnailWidth() const;
    int getThumbnailHeight() const;
    qsizetype getDuration() const;
    qsizetype getPosition() const;
    QString getDisplayDuration() const;
    QString getDisplayPosition() const;
    bool getIsPlayerEmpty() const;

    void setPosition(qsizetype position);

private:
    bool m_playerStarted;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    struct{
        qsizetype songID;
        QString title;
        QString thumbnail;
        int thumbnailWidth;
        int thumbnailHeight;
    } m_songData;

    bool m_isPlayerEmpty;

    QString m_displayPosition;
    QString m_displayDuration;
    float m_volume;
};

#endif // PLAYER_H
