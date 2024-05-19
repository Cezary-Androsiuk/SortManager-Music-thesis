#include "Tag.h"

Tag::Tag(QObject *parent)
    : QObject{parent},
    m_id(0),
    m_name(""),
    m_value(""),
    m_type(-1),
    m_is_immutable(false),
    m_is_editable(false),
    m_is_required(false)
{

}

const int &Tag::get_id() const
{
    return m_id;
}

const QString &Tag::get_name() const
{
    return m_name;
}

const QString &Tag::get_value() const
{
    return m_value;
}

const int &Tag::get_type() const
{
    return m_type;
}

const bool &Tag::get_is_immutable() const
{
    return m_is_immutable;
}

const bool &Tag::get_is_editable() const
{
    return m_is_editable;
}

const bool &Tag::get_is_required() const
{
    return m_is_required;
}

void Tag::set_id(const int &id)
{
    m_id = id;
}

void Tag::set_name(const QString &name)
{
    m_name = name;
}

void Tag::set_value(const QString &value)
{
    m_value = value;
}

void Tag::set_type(const int &type)
{
    m_type = type;
}

void Tag::set_is_immutable(const bool &is_immutable)
{
    m_is_immutable = is_immutable;
}

void Tag::set_is_editable(const bool &is_editable)
{
    m_is_editable = is_editable;
}

void Tag::set_is_required(const bool &is_required)
{
    m_is_required = is_required;
}

