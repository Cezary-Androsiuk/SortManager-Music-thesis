#ifndef TAGDETAILS_H
#define TAGDETAILS_H

#include <QObject>

// #include "DebugPrint/DebugPrint.h"

// #include "Tag/TagProperty.h"
#include "cpp/Song/SongList.h"

class TagDetails : public QObject
{
    /*
     * It is possible to make this class inherit from Tag class, but i want to avoid problems and keep it simple
     * Especially when SongDetails can't and won't inherit from Song because of Song contains fields that are not in songs table
     * There should be another abstract class from which Tag and TagDetails would inherit, but i don't see any reason to implement that
     *
     * This class can store detailed info about song for display/edit purposes.
    */

    Q_OBJECT
    Q_PROPERTY(const int            &id             READ get_id             FINAL CONSTANT)
    Q_PROPERTY(const QString        &name           READ get_name           FINAL CONSTANT)
    Q_PROPERTY(const QString        &description    READ get_description    FINAL CONSTANT)
    Q_PROPERTY(const QString        &add_date       READ get_add_date       FINAL CONSTANT)
    Q_PROPERTY(const QString        &update_date    READ get_update_date    FINAL CONSTANT)
    Q_PROPERTY(const int            &type           READ get_type           FINAL CONSTANT)
    Q_PROPERTY(const bool           &is_immutable   READ get_is_immutable   FINAL CONSTANT)
    Q_PROPERTY(const bool           &is_editable    READ get_is_editable    FINAL CONSTANT)
    Q_PROPERTY(const bool           &is_required    READ get_is_required    FINAL CONSTANT)

    Q_PROPERTY(const SongList       *songs          READ get_songs          FINAL CONSTANT)

public:
    explicit TagDetails(QObject *parent = nullptr);

    const int &get_id() const;
    const QString &get_name() const;
    const QString &get_description() const;
    const QString &get_add_date() const;
    const QString &get_update_date() const;
    const int &get_type() const;
    const bool &get_is_immutable() const;
    const bool &get_is_editable() const;
    const bool &get_is_required() const;

    const SongList *get_songs() const;


    void set_id(const int& id);
    void set_name(const QString &name);
    void set_description(const QString &description);
    void set_add_date(const QString &add_date);
    void set_update_date(const QString &update_date);
    void set_type(const int &type);
    void set_is_immutable(const bool &is_immutable);
    void set_is_editable(const bool &is_editable);
    void set_is_required(const bool &is_required);

    void set_songs(const SongList *songs);

signals:

private:
    int m_id;
    QString m_name;
    QString m_description;
    QString m_add_date;
    QString m_update_date;
    int m_type;
    bool m_is_immutable;
    bool m_is_editable;
    bool m_is_required;

    const SongList *m_songs;
};

#endif // TAGDETAILS_H
