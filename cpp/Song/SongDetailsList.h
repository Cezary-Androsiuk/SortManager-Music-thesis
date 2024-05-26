#ifndef SONGDETAILSLIST_H
#define SONGDETAILSLIST_H

#include <QObject>
#include <QList>

#include "cpp/Song/SongDetails.h"

class SongDetailsList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<SongDetails *> songs READ const_songs FINAL CONSTANT)
public:
    explicit SongDetailsList(QObject *parent = nullptr);


    QList<SongDetails *> &songs();// for setting up list
    const QList<SongDetails *> &c_ref_songs() const; // const reference for iteration etc.
    QList<SongDetails *> const_songs() const; // for qml

signals:

private:
    QList<SongDetails *> m_songs;
signals:
};

#endif // SONGDETAILSLIST_H
