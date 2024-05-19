#ifndef TAG_H
#define TAG_H

#include <QObject>

// #include "DebugPrint/DebugPrint.h"

class Tag : public QObject
{
    /*
     * This class can store tag for listing purposes, where only name and value are needed.
    */

    Q_OBJECT
    Q_PROPERTY(const int        &id             READ get_id             FINAL CONSTANT)
    Q_PROPERTY(const QString    &name           READ get_name           FINAL CONSTANT)
    Q_PROPERTY(const QString    &value          READ get_value          FINAL CONSTANT)
    Q_PROPERTY(const int        &type           READ get_type           FINAL CONSTANT)
    Q_PROPERTY(const bool       &is_immutable   READ get_is_immutable   FINAL CONSTANT)
    Q_PROPERTY(const bool       &is_editable    READ get_is_editable    FINAL CONSTANT)
    Q_PROPERTY(const bool       &is_required    READ get_is_required    FINAL CONSTANT)
public:
    explicit Tag(QObject *parent = nullptr);

    const int &get_id() const;
    const QString &get_name() const;
    const QString &get_value() const;
    const int &get_type() const;
    const bool &get_is_immutable() const;
    const bool &get_is_editable() const;
    const bool &get_is_required() const;


    void set_id(const int& id);
    void set_name(const QString &name);
    void set_value(const QString &value);
    void set_type(const int &type);
    void set_is_immutable(const bool &is_immutable);
    void set_is_editable(const bool &is_editable);
    void set_is_required(const bool &is_required);

signals:

private:
    int m_id;
    QString m_name;
    QString m_value;
    int m_type;
    bool m_is_immutable;
    bool m_is_editable;
    bool m_is_required;
};

#endif // TAG_H
