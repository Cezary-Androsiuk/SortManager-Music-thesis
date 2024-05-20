#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>

#include "cpp/DebugPrint/DebugPrint.h"

#include "cpp/Filter/Filter.h"

class Playlist : public QObject
{
    Q_OBJECT

public:
    explicit Playlist(QObject *parent = nullptr);



signals:

private:

};

#endif // PLAYLIST_H
