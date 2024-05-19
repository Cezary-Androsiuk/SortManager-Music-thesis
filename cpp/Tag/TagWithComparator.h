#ifndef TAGWITHCOMPARATOR_H
#define TAGWITHCOMPARATOR_H

#include <QObject>
#include "Tag.h"

class TagWithComparator : public Tag
{
    Q_OBJECT
    Q_PROPERTY(const int        &comparison_way     READ get_comparison_way       FINAL CONSTANT)
    Q_PROPERTY(const QString    &comparison_value   READ get_comparison_value     FINAL CONSTANT)

public:
    explicit TagWithComparator(QObject *parent = nullptr);

    const int &get_comparison_way() const;
    const QString get_comparison_value() const;

    void set_comparison_way(const int &way);
    void set_comparison_value(const QString &value);

private:
    int m_comparison_way;
    QString m_comparison_value;
};

#endif // TAGWITHCOMPARATOR_H
