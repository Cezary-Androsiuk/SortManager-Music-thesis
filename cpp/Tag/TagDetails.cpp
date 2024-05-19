#include "TagDetails.h"

TagDetails::TagDetails(QObject *parent)
    : QObject{parent},
    m_id(0),
    m_name(""),
    m_description(""),
    m_add_date(""),
    m_update_date(""),
    m_type(-1),
    m_is_immutable(false),
    m_is_editable(false),
    m_is_required(false),
    m_songs(nullptr)
{

}

const int &TagDetails::get_id() const
{
    return m_id;
}

const QString &TagDetails::get_name() const
{
    return m_name;
}

const QString &TagDetails::get_description() const
{
    return m_description;
}

const QString &TagDetails::get_add_date() const
{
    return m_add_date;
}

const QString &TagDetails::get_update_date() const
{
    return m_update_date;
}

const int &TagDetails::get_type() const
{
    return m_type;
}

const bool &TagDetails::get_is_immutable() const
{
    return m_is_immutable;
}

const bool &TagDetails::get_is_editable() const
{
    return m_is_editable;
}

const bool &TagDetails::get_is_required() const
{
    return m_is_required;
}

const SongList *TagDetails::get_songs() const
{
    return m_songs;
}

void TagDetails::set_id(const int &id)
{
    m_id = id;
}

void TagDetails::set_name(const QString &name)
{
    m_name = name;
}

void TagDetails::set_description(const QString &description)
{
    m_description = description;
}

void TagDetails::set_add_date(const QString &add_date)
{
    m_add_date = add_date;
}

void TagDetails::set_update_date(const QString &update_date)
{
    m_update_date = update_date;
}

void TagDetails::set_type(const int &type)
{
    m_type = type;
}

void TagDetails::set_is_immutable(const bool &is_immutable)
{
    m_is_immutable = is_immutable;
}

void TagDetails::set_is_editable(const bool &is_editable)
{
    m_is_editable = is_editable;
}

void TagDetails::set_is_required(const bool &is_required)
{
    m_is_required = is_required;
}
void TagDetails::set_songs(const SongList *songs)
{
    m_songs = songs;
}

