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
    Q_PROPERTY(SongList*    playlistModel       READ getPlaylistModel       NOTIFY playlistModelLoaded FINAL)
public:
    explicit Playlist(QObject *parent = nullptr);

signals: // models signals
    void playlistModelLoaded();                 /// emited when playlist model was loaded
    void playlistModelLoadError(QString desc);  /// emited when loading playlist model failed

public slots: // build models
    void loadPlaylistModel();

public: // get models
    SongList* getPlaylistModel() const;


public slots: // methods
    void loadPlaylist(SongDetailsList *list);
    void shufflePlaylist();

signals: // methods signals
    void playlistLoaded();      /// emited when loadPlaylist finished
    void playlistShuffled();    /// emited when shufflePlaylist finished

public: // support methods
    static std::vector<int> getUniqueRandomNumbers(int count);
    // static SongList shuffleList(const SongList &songs);

private:
    SongList *m_playlistModel;
    SongDetailsList *m_playlist;

};

#endif // PLAYLIST_H
