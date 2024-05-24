#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>

#include "cpp/DebugPrint/DebugPrint.h"
#include "cpp/Tag/TagList.h"

class Playlist : public QObject
{
    Q_OBJECT

public:
    explicit Playlist(QObject *parent = nullptr);

public slots:
    void loadNewPlaylistList(TagList *list);

signals:
    void newPlaylistListLoaded();

private:

};

#endif // PLAYLIST_H
