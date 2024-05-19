#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>

#include "cpp/DebugPrint/DebugPrint.h"

#include "cpp/Filter/Filter.h"

class Playlist : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Filter* filter READ get_filter WRITE set_filter NOTIFY filterChanged FINAL)
public:
    explicit Playlist(QObject *parent = nullptr);

    Filter *get_filter() const;

    void set_filter(Filter * filter);

signals:
    void filterChanged();

private:

    Filter *m_filter;
};

#endif // PLAYLIST_H
