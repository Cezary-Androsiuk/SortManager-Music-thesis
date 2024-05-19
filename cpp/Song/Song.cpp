#include "Song.h"

Song::Song(QObject *parent)
    : QObject{parent},
    m_id(0),
    m_title(""),
    m_value("")
{
    // qDebug() << "\t\t+++ Song " << this;
}

Song::~Song()
{
    // qDebug() << "\t\t--- Song " << this;
}

int Song::get_id() const
{
    return m_id;
}

QString Song::get_title() const
{
    return m_title;
}

QString Song::get_value() const
{
    return m_value;
}

void Song::set_id(const int &id)
{
    m_id = id;
}

void Song::set_title(const QString &title)
{
    m_title = title;
}

void Song::set_value(const QString &value)
{
    m_value = value;
}
