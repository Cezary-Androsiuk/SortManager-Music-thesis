#include "Database.h"


void Database::loadAllSongs()
{
    if(m_all_songs_model != nullptr){
        DB << "all songs model was already loaded - skipped";
        emit this->signalAllSongsModelLoaded();
        return;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalAllSongsModelLoadError)

    // select all song_id and Title tags (btw Title id is 2)
    // this "AS title" is useless but as a comment to describe what is value
    QString query_text("SELECT song_id, value AS title FROM songs_tags WHERE tag_id = 2;");

    this->queryToFile(query_text);
    QSqlQuery query(m_database);
    if(!query.exec(query_text)){
        WR << "executing select query " << query.lastError();
        emit this->signalAllSongsModelLoadError("error while executing query " + query.lastError().text());
        return;
    }

    m_all_songs_model = new SongList(this);

    // read selected data
    while(query.next()){
        auto record = query.record();

        int song_id = record.value(0).toInt();
        QString song_title = record.value(1).toString();

        Song *song = new Song(m_all_songs_model);
        song->set_id(song_id);
        song->set_title(song_title);
        // value is not needed

        m_all_songs_model->songs().append(song);
    }

    this->debugPrintModel_all_songs();

    DB << "all songs model loaded correctly!";
    emit this->signalAllSongsModelLoaded();
}

void Database::loadAddSongModel()
{
    if(m_add_song_model != nullptr){
        DB << "add song model was already loaded - skipped";
        emit this->signalAddSongModelLoaded();
        return;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalAddSongModelLoadError)

    // get editable tags from rows of tags table
    QSqlQuery query(m_database);
    QString query_text("SELECT id, name, type, is_required FROM tags WHERE is_editable = 1;");
    this->queryToFile(query_text);
    if(!query.exec(query_text)){
        WR << "executing query " << query.lastError();
        emit this->signalAddSongModelLoadError("executing query " + query.lastError().text());
        return;
    }

    m_add_song_model = new SongDetails(this);
    // id is 0 by default

    TagList *tags_for_song_details = new TagList(m_add_song_model);

    while(query.next()){
        auto record = query.record();

        auto id = record.value(0).toInt();
        auto name = record.value(1).toString();
        auto type = record.value(2).toInt();
        auto is_required = record.value(3).toBool();

        Tag* tag = new Tag(m_add_song_model);
        tag->set_id(id);
        tag->set_name(name);
        tag->set_type(type);
        tag->set_is_editable(1);
        tag->set_is_required(is_required);

        tags_for_song_details->tags().append(tag);
    }

    // sort by tag id
    std::sort(
        tags_for_song_details->tags().begin(),
        tags_for_song_details->tags().end(),
        [](Tag *a, Tag *b){return a->get_id() < b->get_id();}
        );

    m_add_song_model->set_tags(tags_for_song_details);

    this->debugPrintModel_add_song();

    DB << "add song model loaded correctly";
    emit this->signalAddSongModelLoaded();
}

void Database::loadEditSongModel(int song_id)
{
    if(m_edit_song_model != nullptr){
        if(m_edit_song_model->get_id() == song_id){
            DB << "song with id=" << song_id << " already loaded";
            emit this->signalEditSongModelLoaded();
            return;
        }
        delete m_edit_song_model;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalEditSongModelLoadError)

    QSqlQuery query(m_database);
    QString query_text(QString("SELECT tags.id, tags.name, tags.type, tags.is_editable, tags.is_required, songs_tags.value "
                               "FROM songs_tags "
                               "JOIN tags ON songs_tags.tag_id = tags.id "
                               "WHERE songs_tags.song_id = %1;").arg(song_id));
    this->queryToFile(query_text);
    if(!query.exec(query_text)){
        WR << "error while executing query " << query.lastError();
        emit this->signalEditSongModelLoadError("error while executing query " + query.lastError().text());
        return;
    }

    m_edit_song_model = new SongDetails(this);
    m_edit_song_model->set_id(song_id);

    TagList *tags_for_song_details = new TagList(m_edit_song_model);

    while(query.next()){
        auto record = query.record();

        auto id = record.value(0).toInt();
        auto name = record.value(1).toString();
        auto type = record.value(2).toInt();
        auto is_editable = record.value(3).toBool();
        auto is_required = record.value(4).toBool();
        auto value = record.value(5).toString();
        if(id == 11 || id == 12) // Add Date or Update Date tag
        {
            auto int_value = value.toInt();
            auto add_date_dt = QDateTime::fromSecsSinceEpoch(int_value);
            add_date_dt.setTimeZone(QTimeZone::systemTimeZone());
            value = add_date_dt.toString("hh:mm dd-MM-yyyy"); // yyyy-MM-dd hh:mm
        }

        Tag* tag = new Tag(m_add_song_model);
        tag->set_id(id);
        tag->set_name(name);
        tag->set_type(type);
        tag->set_is_editable(is_editable);
        tag->set_is_required(is_required);
        tag->set_value(value);

        tags_for_song_details->tags().append(tag);
    }

    // sort by tag id
    std::sort(
        tags_for_song_details->tags().begin(),
        tags_for_song_details->tags().end(),
        [](Tag *a, Tag *b){return a->get_id() < b->get_id();}
        );

    m_edit_song_model->set_tags(tags_for_song_details);

    this->debugPrintModel_edit_song();

    DB << "edit song ( id:" << song_id << ") model loaded correctly";
    emit this->signalEditSongModelLoaded();
}


void Database::loadAllTags()
{
    if(m_all_tags_model != nullptr){
        DB << "all tags model was already loaded - skipped";
        emit this->signalAllTagsModelLoaded();
        return;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalAllTagsModelLoadError)

    QString query_text("SELECT id, name, type, is_editable, is_immutable FROM tags");
    // if user don't want to show constant tags, then change query
    if(!m_showConstantTags)
        query_text += " WHERE is_immutable = 0;";
    else
        query_text += ";";

    this->queryToFile(query_text);
    QSqlQuery query(m_database);
    if(!query.exec(query_text)){
        WR << "executing query " << query.lastError();
        emit this->signalAllTagsModelLoadError("executing query " + query.lastError().text());
        return;
    }

    m_all_tags_model = new TagList(this);

    // set values to variable from database
    while(query.next()){
        auto record = query.record();

        int tag_id = record.value(0).toInt();
        QString tag_name = record.value(1).toString();
        int tag_type = record.value(2).toInt();
        bool tag_is_editable = record.value(3).toInt();
        bool tag_is_immutable = record.value(4).toBool();

        Tag* tag = new Tag(m_all_tags_model);
        // used by Tags.qml and import Database
        tag->set_id(tag_id);
        tag->set_name(tag_name);

        // used by Tags.qml
        tag->set_is_immutable(tag_is_immutable);

        // used by import Database
        tag->set_is_editable(tag_is_editable);
        tag->set_type(tag_type);

        m_all_tags_model->tags().append(tag);
    }

    this->debugPrintModel_all_tags();

    DB << (m_showConstantTags ? "all" : "all editable") << "tags model loaded correctly";
    emit this->signalAllTagsModelLoaded();
}

void Database::loadAddTagModel()
{
    if(m_add_tag_model != nullptr){
        DB << "add tag model was already loaded - skipped";
        emit this->signalAddTagModelLoaded();
        return;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalAddTagModelLoadError)

    // select all song_id and Title tags (btw Title id is 2)
    // this "AS title" is useless but as a comment to describe what is value
    // values are not required because user will be choosing what type of tag it is
    QSqlQuery query(m_database);
    QString query_text = "SELECT song_id, value AS title FROM songs_tags WHERE tag_id = 2;";
    this->queryToFile(query_text);
    if(!query.exec(query_text)){
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalAddTagModelLoadError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    m_add_tag_model = new TagDetails(this);
    // m_add_tag_model parameters like name/type will be default and qml will take care of interpreting fields by name

    SongList *songs_for_tag_details = new SongList(this);

    while(query.next()){
        auto record = query.record();

        int song_id = record.value(0).toInt();
        auto title = record.value(1).toString();

        Song* song = new Song(m_add_tag_model);
        song->set_id(song_id);
        song->set_title(title);
        // song value will be default

        songs_for_tag_details->songs().append(song);
    }

    m_add_tag_model->set_songs(songs_for_tag_details);

    this->debugPrintModel_add_tag();

    DB << "add tag model loaded correctly!";
    emit this->signalAddTagModelLoaded();
}

void Database::loadEditTagModel(int tag_id)
{
    if(m_edit_tag_model != nullptr){
        if(m_edit_tag_model->get_id() == tag_id){
            DB << "tag with id=" << tag_id << " already loaded";
            emit this->signalEditTagModelLoaded();
            return;
        }
        delete m_edit_tag_model;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalEditTagModelLoadError)

    bool is_date_tag = false;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv set tag parameters vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    QSqlQuery query(m_database);
    QString query_text = QString("SELECT * FROM tags WHERE id = %1;").arg(tag_id);
    this->queryToFile(query_text);
    if(!query.exec(query_text)){
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalAddTagModelLoadError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    m_edit_tag_model = new TagDetails(this);
    // qml will take care of interpreting fields by name

    // set tag parameters to variables
    if(query.next())
    {
        auto record = query.record();
        for(int i=0; i<record.count(); i++){
            QString column_name = record.fieldName(i);
            auto column_value = record.value(i);

            // the following code is based on trust (for myself) that data in database are correct
            // actually all load models are rebuilded :)
            if(column_name == "id"){
                int cv_int = column_value.toInt();
                m_edit_tag_model->set_id(cv_int);
            }
            else if(column_name == "name"){
                QString string(column_value.toString());
                if(string.contains("Date", Qt::CaseSensitive))
                    is_date_tag = true;
                m_edit_tag_model->set_name(string);
            }
            else if(column_name == "description"){
                QString string(column_value.toString());
                m_edit_tag_model->set_description(string);
            }
            else if(column_name == "add_date"){
                int cv_int = column_value.toInt();

                auto add_date_dt = QDateTime::fromSecsSinceEpoch(cv_int);
                add_date_dt.setTimeZone(QTimeZone::systemTimeZone());
                auto add_date_str = add_date_dt.toString("hh:mm dd-MM-yyyy"); // yyyy-MM-dd hh:mm

                m_edit_tag_model->set_add_date(add_date_str);
            }
            else if(column_name == "update_date"){
                int cv_int = column_value.toInt();

                auto update_date_dt = QDateTime::fromSecsSinceEpoch(cv_int);
                update_date_dt.setTimeZone(QTimeZone::systemTimeZone());
                auto update_date_str = update_date_dt.toString("hh:mm dd-MM-yyyy"); // yyyy-MM-dd hh:mm

                m_edit_tag_model->set_update_date(update_date_str);
            }
            else if(column_name == "type"){
                int cv_int = column_value.toInt();
                m_edit_tag_model->set_type(cv_int);
            }
            else if(column_name == "is_immutable"){
                bool value = column_value.toBool();
                m_edit_tag_model->set_is_immutable(value);
            }
            else if(column_name == "is_editable"){
                bool value = column_value.toBool();
                m_edit_tag_model->set_is_editable(value);
            }
            else if(column_name == "is_required"){
                bool value = column_value.toBool();
                m_edit_tag_model->set_is_required(value);
            }
        }
    }
    else
    {
        WR << "error while going through SELECT query (no result) " << query.lastError();
        delete m_edit_tag_model;
        m_edit_tag_model = nullptr;
        emit this->signalAddTagModelLoadError("error while going through SELECT query (no result) " + query.lastError().text());
        return;
    }

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv set songs parameters vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // this query is a mystery, GPT made it for me, but after i "press" him...
    // however this can be solved with two queries (one selecting titles of the song and another to select values between song and current tag)
    // select title:                                            SELECT song_id, value AS title      FROM songs_tags WHERE tag_id = 2
    // select values related between songs and tag(with id=4):  SELECT song_id, value AS song_value FROM songs_tags WHERE tag_id = 4;
    query_text = QString("SELECT song_id,                                               "
                         "    MAX(CASE WHEN tag_id = 2 THEN value END) AS title,        "
                         "    MAX(CASE WHEN tag_id = %1 THEN value END) AS song_value   "
                         "FROM songs_tags                                               "
                         "WHERE tag_id IN (2, %1)                                       "
                         "GROUP BY song_id;                                             ").arg(tag_id);

    this->queryToFile(query_text);
    if(!query.exec(query_text)){
        WR << "error while executing SELECT query " << query.lastError();
        delete m_edit_tag_model;
        m_edit_tag_model = nullptr;
        emit this->signalAddTagModelLoadError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    SongList *songs_for_tag_details = new SongList(m_edit_tag_model);

    while(query.next()){
        auto record = query.record();

        int song_id = record.value(0).toInt();
        auto title = record.value(1).toString();
        auto value = record.value(2).toString();
        if(is_date_tag)
        {
            auto int_value = value.toInt();
            auto add_date_dt = QDateTime::fromSecsSinceEpoch(int_value);
            add_date_dt.setTimeZone(QTimeZone::systemTimeZone());
            value = add_date_dt.toString("hh:mm dd-MM-yyyy"); // yyyy-MM-dd hh:mm
        }


        Song* song = new Song(m_edit_tag_model);
        song->set_id(song_id);
        song->set_title(title);
        song->set_value(value);

        songs_for_tag_details->songs().append(song);
    }

    m_edit_tag_model->set_songs(songs_for_tag_details);

    this->debugPrintModel_edit_tag();

    DB << "edit tag ( id:" << tag_id << ") model loaded correctly!";
    emit this->signalEditTagModelLoaded();
}

/*void Database::loadPlaylistSongs(cQls tc_names, cQls tc_comparators, cQls tc_values, cQls te_names, cQlb te_values)
{
    WR << "load playlist not finished!";
    return;

    // /// after checking if model is loaded because, if it is loaded then we don't care about database
    // IS_DATABASE_OPEN(signalPlaylistSongsModelLoadError)

    // int local_error_code;
    // QSqlQuery query(this->prepPlaylistSongsQuery(
    //     tc_names, tc_values, tc_comparators, te_names, te_values, &local_error_code));
    // this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
    // if(local_error_code != 0){
    //     WR << "error while preparing playlist song query:" << local_error_code;
    //     emit this->signalPlaylistSongsModelLoadError("");
    //     return;
    // }


    // for(auto& song : this->m_playlist_songs){
    //     delete song;
    // }
    // this->m_playlist_songs.clear();


    // if(!query.exec()){
    //     WR << "executing query " << query.lastError();
    //     emit this->signalPlaylistSongsModelLoadError("");
    //     return;
    // }

    // // // debug print
    // // auto blank_record = query.record();
    // // for(int i=0; i<blank_record.count(); i++)
    // //     qDebug() << blank_record.fieldName(i);

    // while(query.next()){
    //     auto record = query.record();

    //     // // debug pring
    //     // for(int i=0; i<record.count(); i++){
    //     //     qDebug() << record.value(i) ;
    //     // }

    //     bool local_error_code;
    //     int song_id = record.value(0).toInt(&local_error_code);
    //     if(!local_error_code){
    //         WR << "error while converting id to int: " << record.value(0);
    //         emit this->signalPlaylistSongsModelLoadError("");
    //         return;
    //     }

    //     QString title = record.value(1).toString();

    //     // song will be removed after this (database instance) is removed
    //     // also when a new songs are attempted to load by this method
    //     Song* song = new Song(this);
    //     song->set_song_id(song_id);
    //     song->set_song_title(title);
    //     this->m_playlist_songs.append(song);
    // }

    // // emit songs playlist changed

    // DB << "playlist songs model loaded correctly";
    // emit this->signalPlaylistSongsModelLoaded();
}*/

void Database::loadPlaylistModel()
{
    if(m_playlist_model != nullptr){
        DB << "playlist model was already loaded - skipped";
        emit this->signalPlaylistModelLoaded();
        return;
    }

    /// after checking if model is already loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalPlaylistModelLoadError)

    // select all song_id and Title tags (btw Title id is 2)
    // this "AS title" is useless but as a comment to describe what is value
    QString query_text("SELECT song_id, value AS title FROM songs_tags WHERE tag_id = 2;");

    this->queryToFile(query_text);
    QSqlQuery query(m_database);
    if(!query.exec(query_text)){
        WR << "executing select query " << query.lastError();
        emit this->signalPlaylistModelLoadError("error while executing query " + query.lastError().text());
        return;
    }

    m_playlist_model = new SongList(this);

    // read selected data
    while(query.next()){
        auto record = query.record();

        int song_id = record.value(0).toInt();
        QString song_title = record.value(1).toString();

        Song *song = new Song(m_playlist_model);
        song->set_id(song_id);
        song->set_title(song_title);
        // value is not needed

        m_playlist_model->songs().append(song);
    }

    this->debugPrintModel_playlist();

    DB << "playlist model loaded correctly!";
    emit this->signalPlaylistModelLoaded();
}

void Database::loadEditPlaylistSongModel(int song_id)
{
    // NOTE: loaded data, can be reached by m_edit_song_model

    QString load_error;
    auto lambda_model = [&](QString desc){
        /// [&] means get reference from parents variables
        /// desc is value received from signalEditSongModelLoadError(QString desc)
        load_error = desc; // some warning occur, but i don't see a better way
    };

    auto connection_model = connect(this, &Database::signalEditSongModelLoadError, lambda_model);

    this->loadEditSongModel(song_id);

    if(load_error != ""){
        WR << "Error while loading edit song model for edit playlist:" << load_error;
        emit this->signalEditPlaylistSongModelLoadError("Error while loading edit song model for edit playlist: " + load_error);
        return;
    }
    disconnect(connection_model);

    emit this->signalEditPlaylistSongModelLoaded();
}

void Database::loadFiltersModel()
{
    if(m_filters_model != nullptr){
        DB << "filters model was already loaded - skipped";
        emit this->signalFiltersModelLoaded();
        return;
    }

    /// after checking if model is loaded because, if it is loaded then we don't care about database
    IS_DATABASE_OPEN(signalFiltersModelLoadError)

    QString query_text("SELECT id, name, type FROM tags;");

    this->queryToFile(query_text);
    QSqlQuery query(m_database);
    if(!query.exec(query_text)){
        WR << "executing query " << query.lastError();
        emit this->signalFiltersModelLoadError("executing query " + query.lastError().text());
        return;
    }

    m_filters_model = new TagList(this);

    // set values to variable from database
    while(query.next()){
        auto record = query.record();

        int tag_id = record.value(0).toInt();
        QString tag_name = record.value(1).toString();
        int tag_type = record.value(2).toInt();

        TagWithComparator* tag = new TagWithComparator(m_filters_model);
        tag->set_id(tag_id);
        tag->set_name(tag_name);
        tag->set_type(tag_type);
        tag->set_comparison_way(2);
        tag->set_comparison_value("129");

        m_filters_model->tags().append(tag);
    }

    this->debugPrintModel_filters();

    DB << "filters model loaded correctly!";
    emit this->signalFiltersModelLoaded();
}

