#ifndef TAGLIST_H
#define TAGLIST_H

#include <QObject>
#include <QList>

#include "cpp/Tag/Tag.h"

class TagList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Tag *> tags    READ const_tags FINAL CONSTANT)
    Q_PROPERTY(qsizetype tagsCount  READ tags_count FINAL CONSTANT)
public:
    explicit TagList(QObject *parent = nullptr);

    QList<Tag *> &tags(); // for setting up list
    const QList<Tag *> &c_ref_tags() const; // const reference for iteration etc.
    QList<Tag *> const_tags() const; // for qml

    qsizetype tags_count() const;

signals:

private:
    QList<Tag *> m_tags;
};

#endif // TAGLIST_H
