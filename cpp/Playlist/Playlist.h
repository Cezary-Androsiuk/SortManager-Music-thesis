#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <vector> // std::vector
#include <QRandomGenerator>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetailsList.h"
#include "cpp/Song/SongList.h"

class Playlist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SongList*    playlistModel       READ getPlaylistModel       NOTIFY playlistModelLoaded  FINAL)
    Q_PROPERTY(qsizetype    currentPos          READ getCurrentPos          NOTIFY songStateChanged     FINAL)
    // Q_PROPERTY(qsizetype    currentID           READ getCurrentID           NOTIFY songStateChanged     FINAL)
    Q_PROPERTY(qsizetype    nextPos             READ getNextPos             NOTIFY songStateChanged     FINAL)
public:
    explicit Playlist(QObject *parent = nullptr);


signals: // models signals
    void playlistModelLoaded();                 /// is emited when playlist model was loaded
    void playlistModelLoadError(QString desc);  /// is emited when loading playlist model failed

public slots: // build models
    void loadPlaylistModel();

public: // get models
    SongList* getPlaylistModel() const;


public slots: // methods
    void loadPlaylist(SongDetailsList *list);   /// is triggered only by Database::signalPlaylistLoaded
    void shufflePlaylist();                     /// is triggered by loadPlaylist and by button in QML
    void updateSongState();                     /// is triggered when player finishes the song
    void playerSongEnded();                     /// is triggered by Player, when song finish

signals: // methods signals
    void playlistLoaded();          /// is emited when loadPlaylist finished
    void playlistShuffled();        /// is emited when shufflePlaylist finished
    void songStateChanged();        /// is emited by updateSongState when state changed

    void currentSongChanged(const SongDetails *song);    ///


public: // support methods
    static std::vector<int> getUniqueRandomNumbers(int count);
    void shufflePlaylistMethod();

    qsizetype getPosKnowingID(const qsizetype &id) const;
    qsizetype getIDKnowingPos(const qsizetype &pos) const;
    qsizetype getComputedNextSongPos() const;

public: // song state getters/setters
    qsizetype getCurrentPos() const;
    qsizetype getCurrentID() const;
    qsizetype getNextPos() const;

    void setCurrentPos(const qsizetype &pos);
    void setCurrentID(const qsizetype &id);
    void setNextPos(const qsizetype &pos);

private:
    SongList *m_playlistModel;
    SongDetailsList *m_playlist;

    struct{
        qsizetype m_currentPos;
        qsizetype m_currentID;
        qsizetype m_nextPos;
    } m_songState;

};

#endif // PLAYLIST_H
