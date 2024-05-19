#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QMap>

#include "cpp/DebugPrint/DebugPrint.h"


class Filter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(const QVariantMap &fields READ get_fields WRITE set_fields NOTIFY fieldsChanged FINAL)
public:
    explicit Filter(QObject *parent = nullptr);

    const QVariantMap &get_fields() const;
    QVariantMap &get_fields_ref();

    void set_fields(const QVariantMap &qv_map);

signals:
    void fieldsChanged();

private:
    QVariantMap m_fields;
};

#endif // FILTER_H
