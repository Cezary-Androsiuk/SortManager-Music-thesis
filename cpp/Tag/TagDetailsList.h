#ifndef TAGDETAILSLIST_H
#define TAGDETAILSLIST_H

#include <QObject>
#include <QList>

#include "cpp/Tag/TagDetails.h"

class TagDetailsList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<TagDetails *> tags READ const_tags FINAL CONSTANT)
public:
    explicit TagDetailsList(QObject *parent = nullptr);

    QList<TagDetails *> &tags(); // for setting up list
    const QList<TagDetails *> &c_ref_tags() const; // const reference for iteration etc.
    QList<TagDetails *> const_tags() const; // for qml

signals:

private:
    QList<TagDetails *> m_tags;
};

#endif // TAGDETAILSLIST_H
