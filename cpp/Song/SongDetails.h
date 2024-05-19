#ifndef SONGDETAILS_H
#define SONGDETAILS_H

#include <QObject>

// #include "DebugPrint/DebugPrint.h"

#include "cpp/Tag/TagList.h"

class SongDetails : public QObject
{
    /*
     * This class can store detailed info about song for display/edit purposes.
    */

    Q_OBJECT
    Q_PROPERTY(const int &      id      READ get_id     CONSTANT FINAL)
    Q_PROPERTY(const TagList *  tags    READ get_tags   CONSTANT FINAL)

public:
    explicit SongDetails(QObject *parent = nullptr);

    const int &get_id() const;
    const TagList *get_tags() const;

    void set_id(const int &id);
    void set_tags(const TagList *tags);

signals:

private:
    int m_id;
    const TagList *m_tags; // contains constant and editables
};

#endif // SONGDETAILS_H
