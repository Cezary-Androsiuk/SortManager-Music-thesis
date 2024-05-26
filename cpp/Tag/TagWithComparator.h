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

    struct IntegerCompare{
        enum{
            DO_NOT_COMPARE,
            IS_EQUAL_TO,
            IS_DIFFERENT_THAN,
            IS_LESS_OR_EQUAL_TO,
            IS_LESS_THAN,
            IS_GREATER_OR_EQUAL_TO,
            IS_GREATER_THAN
        };
    };

    struct TextCompare{
        enum{
            DO_NOT_COMPARE,
            IS_EQUAL_TO,
            IS_DIFFERENT_THAN,
            IS_APPROXIMATELY_EQUAL_TO,
            IS_APPROXIMATELY_DIFFERENT_THAN,
            IS_LIKE,
            IS_EQUAL_REGEX
        };
    };

    struct BoolCompare{
        enum{
            NOT_BELONG_TO_TAG,
            DO_NOT_COMPARE,
            BELONG_TO_TAG
        };
    };


    const int &get_comparison_way() const;
    const QString get_comparison_value() const;

    void set_comparison_way(const int &way);
    void set_comparison_value(const QString &value);

private:
    int m_comparison_way;
    QString m_comparison_value;
};

#endif // TAGWITHCOMPARATOR_H
