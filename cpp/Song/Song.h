#ifndef SONG_H
#define SONG_H

#include <QObject>

// #include "DebugPrint/DebugPrint.h"

class Song : public QObject
{
    /*
     * This class can store song for listing purposes, where only id and title are needed.
    */

    Q_OBJECT
    Q_PROPERTY(int id           READ get_id     CONSTANT FINAL)
    Q_PROPERTY(QString title    READ get_title  CONSTANT FINAL)
    Q_PROPERTY(QString value    READ get_value  CONSTANT FINAL)
public:
    explicit Song(QObject *parent = nullptr);
    ~Song();

    int get_id() const;
    QString get_title() const;
    QString get_value() const;

    void set_id(const int &id);
    void set_title(const QString &title);
    void set_value(const QString &value);

signals:

private:
    int m_id;
    QString m_title;
    QString m_value;
};

#endif // SONG_H
