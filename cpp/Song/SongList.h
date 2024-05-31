#ifndef SONGLIST_H
#define SONGLIST_H

#include <QObject>
#include <QList>

#include "cpp/Song/Song.h"

class SongList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Song *> songs  READ const_songs FINAL CONSTANT)
    Q_PROPERTY(qsizetype songsCount READ songs_count FINAL CONSTANT)
public:
    explicit SongList(QObject *parent = nullptr);

    QList<Song *> &songs();// for setting up list
    const QList<Song *> &c_ref_songs() const; // const reference for iteration etc.
    QList<Song *> const_songs() const; // for qml

    qsizetype songs_count() const;

signals:

private:
    QList<Song *> m_songs;
};

#endif // SONGLIST_H
