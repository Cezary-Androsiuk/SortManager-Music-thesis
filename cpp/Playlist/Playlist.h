#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Song/SongDetailsList.h"

class Playlist : public QObject
{
    Q_OBJECT

public:
    explicit Playlist(QObject *parent = nullptr);

signals:
    void playlistModelLoaded();
    void playlistModelLoadError(QString desc);

public slots:
    void loadPlaylistModel();

public slots:
    void loadNewPlaylistList(SongDetailsList *list);

signals:
    void newPlaylistListLoaded();

private:
    SongDetailsList *m_playlistList;

};

#endif // PLAYLIST_H
