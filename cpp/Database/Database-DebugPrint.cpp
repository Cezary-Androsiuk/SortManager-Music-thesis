#include "Database.h"


void Database::debugPrintModel_all_songs() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "ALL SONGS MODEL:";
    DB << this->_debugPrintModel_SongList(m_all_songs_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_add_song() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "ADD SONG MODEL:";
    DB << this->_debugPrintModel_SongDetails(m_add_song_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_edit_song() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "EDIT SONG MODEL:";
    DB << this->_debugPrintModel_SongDetails(m_edit_song_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_all_tags() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "ALL TAGS MODEL:";
    DB << this->_debugPrintModel_TagList(m_all_tags_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_add_tag() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "ADD TAG MODEL:";
    DB << this->_debugPrintModel_TagDetails(m_add_tag_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_edit_tag() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "EDIT TAG MODEL:";
    DB << this->_debugPrintModel_TagDetails(m_edit_tag_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_playlist() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "PLAYLIST MODEL:";
    DB << this->_debugPrintModel_SongList(m_playlist_model);
    DB << "|";
    DB << "|";
#endif
}

void Database::debugPrintModel_filters() const
{
#if PRINT_MODELS_LISTS
    DB << "|";
    DB << "|";
    DB << "FILTERS MODEL:";
    DB << this->_debugPrintModel_TagList(m_filters_model);
    DB << "|";
    DB << "|";
#endif
}

QString Database::_debugPrintModel_SongList(SongList * const &model) const
{
    /* fistly:  XD reference to pointer variable, that is sick
     * secondly:
     * const pointer not variable:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y;
     * that way because both methods is const and if we passing an reference to a pointer,
     *     we need to provide that place on what this pointer points won't change
    */
    QString obj_data("[");
    for(const auto &s : model->c_ref_songs()){
        obj_data += QString("{id: '%1', title: '%2', value: '%3'}")
                        .arg(s->get_id())
                        .arg(s->get_title(),
                             s->get_value());
        obj_data += (s == model->c_ref_songs().last() ? "" : ", ");
    }
    return obj_data + "]";
}

QString Database::_debugPrintModel_SongDetails(SongDetails* const &model) const
{
    /* fistly:  XD reference to pointer variable, that is sick
     * secondly:
     * const pointer not variable:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y;
     * that way because both methods is const and if we passing an reference to a pointer,
     *     we need to provide that place on what this pointer points won't change
    */
    QString obj_data( QString("{song_id: '%1', tags: [").arg(model->get_id()) );
    for(const auto &t : model->get_tags()->c_ref_tags()){
        obj_data += QString("{id: '%1', name: '%2', value: '%3', type: '%4', "
                            "is_immutable: '%5', is_editable: '%6', is_required: '%7'}")
                        .arg(t->get_id())
                        .arg(t->get_name(),
                             t->get_value())
                        .arg(t->get_type())
                        .arg(t->get_is_immutable())
                        .arg(t->get_is_editable()) // bool is a nightmare
                        .arg(t->get_is_required());
        obj_data += (t == model->get_tags()->c_ref_tags().last() ? "" : ", ");
    }
    return obj_data + "]}";
}

QString Database::_debugPrintModel_TagList(TagList * const &model) const
{
    /* fistly:  XD reference to pointer variable, that is sick
     * secondly:
     * const pointer not variable:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y;
     * that way because both methods is const and if we passing an reference to a pointer,
     *     we need to provide that place on what this pointer points won't change
    */
    QString obj_data("[");
    for(const auto &t : model->c_ref_tags()){
        obj_data += QString("{id: '%1', name: '%2', value: '%3', type: '%4', "
                            "is_immutable: '%5', is_editable: '%6', is_required: '%7'}")
                        .arg(t->get_id())
                        .arg(t->get_name(),
                             t->get_value())
                        .arg(t->get_type())
                        .arg(t->get_is_immutable())
                        .arg(t->get_is_editable()) // bool is a nightmare
                        .arg(t->get_is_required());
        obj_data += (t == model->c_ref_tags().last() ? "" : ", ");
    }
    return obj_data + "]";
}

QString Database::_debugPrintModel_TagDetails(TagDetails* const &model) const
{
    /* fistly:  XD reference to pointer variable, that is sick
     * secondly:
     * const pointer not variable:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y;
     * that way because both methods is const and if we passing an reference to a pointer,
     *     we need to provide that place on what this pointer points won't change
    */
    QString obj_data(QString("{id: '%1', name: '%2', description: '%3', add_date: '%4', update_date: '%5', "
                             "type: '%6', is_immutable: '%7', is_editable: '%8', is_required: '%9', songs: [")
                         .arg(model->get_id())
                         .arg(model->get_name(),
                              model->get_description(),
                              model->get_add_date(),
                              model->get_update_date())
                         .arg(model->get_type())
                         .arg(model->get_is_immutable())
                         .arg(model->get_is_editable()) // bool is a nightmare
                         .arg(model->get_is_required()) );
    for(const auto &s : model->get_songs()->c_ref_songs()){
        obj_data += QString("{id: '%1', title: '%2', value: '%3'}")
                        .arg(s->get_id())
                        .arg(s->get_title(),
                             s->get_value());
        obj_data += (s == model->get_songs()->c_ref_songs().last() ? "" : ", ");
    }
    return obj_data + "]}";
}
