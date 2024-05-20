#include "Database.h"

void Database::addSong(QVariantList new_song_data)
{
    IS_DATABASE_OPEN(signalAddSongError)

    /*
        received structure:

        new_song_data - QVariantList:
            id - int: from QML fields
            value - QString: from QML fields

        example:
        QList(
            QVariant(
                QVariantMap,
                QMap(
                    ("id", QVariant(int, 2))
                    ("value", QVariant(QString, "asd"))
                )
            ),
            QVariant(
                QVariantMap,
                QMap(
                    ("id", QVariant(int, 3))
                    ("value", QVariant(QString, "wre"))
                )
            ),
            ...
        )

        NOTE: received structure contains only editable fields
    */

    // in list there is no duration or add_date (because for add model was sended only editable fields)
    // DB << "QVariant(nullptr) =" << QVariant(nullptr);

    auto lambda_get_value_by_id = [&](int id) -> QVariant{
        for(const auto &list_variant : new_song_data)
        {
            auto map = list_variant.toMap();
            if(map["id"].toInt() == id)
                return map["value"];
        }

        WR << "id:" << id << "not found in the new_song_data - QVariantList!";
        return QVariant(QString(""));
    };

    auto lambda_get_map_index_by_id = [&](int id) -> int{
        int i=0;
        for(const auto &list_variant : new_song_data)
        {
            if(list_variant.toMap()["id"].toInt() == id)
                return i;
            ++i;
        }

        WR << "id:" << id << "not found in the new_song_data - QVariantList!";
        return -1;
    };

    // ONLY SONG PATH IS UNIQUE
    QString tmp_song_path = lambda_get_value_by_id(9 /*Song Path field id*/).toString();
    QString song_path(tmp_song_path);

    if(song_path.isEmpty())
    {
        WR << "error song path can't be empty!";
        emit this->signalAddSongError("error song path can't be empty!");
        return;
    }

    if(!song_path.contains("file:///"))
        song_path = "file:///" + song_path;

    QSqlQuery query(m_database);
    QString query_text("SELECT song_id FROM songs_tags WHERE tag_id = 9 AND value = :value;"); // tag_id = 9 is Song Path tag

    if(!query.prepare(query_text))
    {
        WR << "error while prepating SELECT query " << query.lastError();
        emit this->signalAddSongError("error while prepating SELECT query " + query.lastError().text());
        return;
    }

    query.bindValue(":value", song_path);

    this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
    if(!query.exec())
    {
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalAddSongError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    if(query.next())
    {
        // if select query return any records, that means in db exist song with the same song path
        // can't be any other song with the same song path as given
        DB << "Song Path should be unique, but this path belong to song with id =" << query.value(0).toInt();
        emit this->signalAddSongError(
            "Song Path should be unique, but this path belong to song with id = " + query.value(0).toString());
        return;
    }

    // get song duration
    QMediaPlayer mp;

    /// wait for the event (load media)
    QEventLoop loop;
    connect(&mp, &QMediaPlayer::mediaStatusChanged, &loop, &QEventLoop::quit);
    mp.setSource(song_path);
    loop.exec();
    if(mp.mediaStatus() != QMediaPlayer::LoadedMedia){
        WR << "error while loading song: " << mp.errorString();
        emit this->signalAddSongError("error while loading song: " + mp.errorString());
        return;
    }

    // ------------------------------ set not editable fields -----------------------------

    // set title if needed
    /// test if first field is title (as it should be)
    /// // actually I don't check this, I trust myself... I hope
    QString song_title(mp.metaData().value(QMediaMetaData::Title).toString()); // will be used in setting icon
    if(lambda_get_value_by_id(2 /*Title field id*/).toString() == ""){
        if(song_title == ""){
            WR << "song file doesn't contains Title metadata, setting file name as the Title";
            song_title = QFileInfo(song_path).baseName();
        }
        DB << "setting own title to" << song_title;
        int index = lambda_get_map_index_by_id(2/*Title field id*/);
        if(index == -1) // Title not exist
        {
            WR << "error, Title field not found!";
            emit this->signalAddSongError("error, Title field not found!");
            return;
        }
        else // Title field found
        {
            auto map = new_song_data[index].toMap();
            map["value"] = song_title;
            // this way because new_song_data[0].toMap() returns constant map
            new_song_data[index] = map;
        }
    }

    // // set Thumbnail Path // not works :c
    // if(lambda_get_value_by_id(10 /*Thumbnail Path field id*/).toString() == ""){
    //     /// only for my structure of files
    //     QString tmp_song_path = song_path;
    //     tmp_song_path.replace("/songs", "");
    //     QDir song_dir(QFileInfo(tmp_song_path).path());
    //     QString thumbnail_path = song_dir.path() + "/icons/" + song_title + " (BQ).jpg";
    //     if(QFile::exists(thumbnail_path))
    //     {
    //         int index = lambda_get_map_index_by_id(10/*Thumbnail Path field id*/);
    //         if(index == -1) // Thumbnail Path not exist
    //         {
    //             WR << "error, Thumbnail Path field not found!";
    //             emit this->signalAddSongError("error, Thumbnail Path field not found!");
    //             return;
    //         }
    //         else // Thumbnail Path field found
    //         {
    //             auto map = new_song_data[index].toMap();
    //             map["value"] = thumbnail_path;
    //             new_song_data[index] = map;
    //         }
    //     }
    //     else
    //     {
    //         DB << "cannot set not existing thumbnail path" << thumbnail_path;
    //         DB << "auto setting thumbail works only in home environment";
    //     }
    // }

    // set duration
    auto duration = QString::number(mp.duration());
    if(mp.duration() <= 0){
        WR << "error, duration <= 0 is a bad sign!";
        emit this->signalAddSongError("error, duration <= 0 is a bad sign!");
        return;
    }
    new_song_data.append(
        QVariant(
            QVariantMap({{"id", 7/*id of Duration field*/},{"value", duration}})));

    // add song date
    QDateTime dt;
    dt.setTimeZone(QTimeZone::systemTimeZone());
    auto current_time = QString::number(dt.currentSecsSinceEpoch());
    new_song_data.append(
        QVariant(
            QVariantMap({{"id", 11/*id of Add Date field*/},{"value", current_time}})));

    // update song date
    new_song_data.append(
        QVariant(
            QVariantMap({{"id", 12/*id of Update Date field*/},{"value", current_time}})));

    int song_id;

    BEGIN_TRANSACTION
    {
        // ------------------------------ add new song to database -----------------------------

        query_text = "INSERT INTO songs DEFAULT VALUES;";
        this->queryToFile(query_text);
        if(!query.exec(query_text)){
            WR << "error while executing INSERT INTO songs query " << query.lastError();
            DB << "cancelling transaction ...";
            m_database.rollback();
            emit this->signalAddSongError(
                "error while executing INSERT INTO songs query " + query.lastError().text());
            return;
        }

        song_id = query.lastInsertId().toInt();

        // add ID field
        new_song_data.append(
            QVariant(
                QVariantMap({{"id", 1/*id of ID field*/},{"value", song_id}})));

        // ------------------------------ add values to table songs_tags -----------------------------

        for(const auto &param : new_song_data)
        {
            auto map = param.toMap();
            if(!query.prepare("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (:song_id, :tag_id, :value);"))
            {
                WR << "error while preparing INSERT INTO songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalAddSongError(
                    "error while preparing INSERT INTO songs_tags query " + query.lastError().text());
                return;
            }

            query.bindValue(":song_id", song_id);
            query.bindValue(":tag_id", map["id"].toInt());
            query.bindValue(":value", this->notNull(map["value"].toString()));

            this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());

            if(!query.exec())
            {
                WR << "error while executing INSERT INTO songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "values: {song_id: '" << song_id << "', tag_id: '" << map["id"].toInt()
                   << "', value: '" << map["value"].toString() << "'}";
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalAddSongError(
                    "error while executing INSERT INTO songs_tags query " + query.lastError().text());
                return;
            }
        }
    }
    END_TRANSACTION(signalAddSongError)

    ///
    /// NOTE: after these operations values in songs_tags won't be in numerical order (i mean tag_id)
    ///

    this->clear_models_memory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

    DB << "song ( id:" << song_id << ") added correctly!";
    emit this->signalAddedSong();
}

void Database::editSong(int song_id, QVariantList song_data)
{
    IS_DATABASE_OPEN(signalEditSongError)

    /*
        received structure:

        new_song_data - QVariantList:
            id - int: from QML fields
            value - QString: from QML fields

        example:
        QList(
            QVariant(
                QVariantMap,
                QMap(
                    ("id", QVariant(int, 2))
                    ("value", QVariant(QString, "asd"))
                )
            ),
            QVariant(
                QVariantMap,
                QMap(
                    ("id", QVariant(int, 3))
                    ("value", QVariant(QString, "wre"))
                )
            ),
            ...
        )

        NOTE: received structure contains only editable fields
    */

    // in list there is no duration or add_date (because for add model was sended only editable fields)

    auto lambda_get_value_by_id = [&](int id) -> QVariant{
        for(const auto &list_variant : song_data)
        {
            auto map = list_variant.toMap();
            if(map["id"].toInt() == id)
                return map["value"];
        }
        return QVariant(QString(""));
    };

    auto lambda_get_map_index_by_id = [&](int id) -> int{
        int i=0;
        for(const auto &list_variant : song_data)
        {
            if(list_variant.toMap()["id"].toInt() == id)
                return i;
            ++i;
        }
        return -1;
    };

    // ONLY SONG PATH IS UNIQUE
    QString tmp_song_path = lambda_get_value_by_id(9 /*Song Path field id*/).toString();
    QString song_path(tmp_song_path);

    if(song_path.isEmpty())
    {
        WR << "error song path can't be empty!";
        emit this->signalAddSongError("error song path can't be empty!");
        return;
    }

    if(!song_path.contains("file:///"))
        song_path = "file:///" + song_path;

    QSqlQuery query(m_database);
    QString query_text("SELECT song_id, value FROM songs_tags WHERE tag_id = 9 AND value = :value;"); // tag_id = 9 is Song Path tag

    if(!query.prepare(query_text))
    {
        WR << "error while prepating SELECT query " << query.lastError();
        emit this->signalEditSongError("error while prepating SELECT query " + query.lastError().text());
        return;
    }

    query.bindValue(":value", song_path);

    this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
    if(!query.exec())
    {
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalEditSongError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    bool song_path_changed = true;

    if(query.next())
    {
        if(query.value(0).toInt() != song_id)
        {
            // if select query return any record with different id that current
            //   that means in db exist song with the same song path as given
            // if return record with the same id, that means user not changed tag name while editing
            DB << "Song Path should be unique, but this path belong to song with id =" << query.value(0).toInt();
            emit this->signalEditSongError(
                "Song Path should be unique, but this path belong to song with id = " + query.value(0).toString());
            return;
        }
        song_path_changed = false; // found in db song path related with the same id
    }


    // auto tmp = song_data[6/* 6 should be Title*/].toMap();
    // tmp["value"] = QString("G:/JDownloader2/0_music/2021.08.19_Searching_New_1/songs/24kGoldn - Mood (Lyrics) ft. Iann Dior (1080p_30fps_H264-128kbit_AAC).mp4");
    // // this way because song_data[6].toMap() returns constant map
    // song_data[6/* 6 should be Title*/] = tmp;
    // WR << "TMP song path";

    if(song_path_changed)
    {
        // get song duration
        QMediaPlayer mp;

        /// wait for the event (load media)
        QEventLoop loop;
        connect(&mp, &QMediaPlayer::mediaStatusChanged, &loop, &QEventLoop::quit);
        mp.setSource(song_path);
        loop.exec();
        if(mp.mediaStatus() != QMediaPlayer::LoadedMedia){
            WR << "error while loading song: " << mp.errorString();
            emit this->signalEditSongError("error while loading song: " + mp.errorString());
            return;
        }

        // ------------------------------ set not editable fields -----------------------------

        // set title if needed
        /// test if first field is title (as it should be)
        /// // actually I don't check this, I trust myself... I hope
        QString song_title(mp.metaData().value(QMediaMetaData::Title).toString()); // will be used in setting icon
        if(lambda_get_value_by_id(2 /*Title field id*/).toString() == ""){
            if(song_title == ""){
                WR << "song file doesn't contains Title metadata, setting file name as the Title";
                song_title = QFileInfo(song_path).baseName();
            }
            DB << "setting own title to" << song_title;
            int index = lambda_get_map_index_by_id(2/*Title field id*/);
            if(index == -1) // Title not exist
            {
                WR << "error, Title field not found!";
                emit this->signalAddSongError("error, Title field not found!");
                return;
            }
            else // Title field found
            {
                auto map = song_data[index].toMap();
                map["value"] = song_title;
                // this way because song_data[0].toMap() returns constant map
                song_data[index] = map;
            }
        }

        // // set Thumbnail Path // not works :c
        // if(lambda_get_value_by_id(10 /*Thumbnail Path field id*/).toString() == ""){
        //     /// only for my structure of files
        //     QString tmp_song_path = song_path;
        //     tmp_song_path.replace("/songs", "");
        //     QDir song_dir(QFileInfo(tmp_song_path).path());
        //     QString thumbnail_path = song_dir.path() + "/icons/" + song_title + " (BQ).jpg";
        //     if(QFile::exists(thumbnail_path))
        //     {
        //         int index = lambda_get_map_index_by_id(10/*Thumbnail Path field id*/);
        //         if(index == -1) // Thumbnail Path not exist
        //         {
        //             WR << "error, Thumbnail Path field not found!";
        //             emit this->signalAddSongError("error, Thumbnail Path field not found!");
        //             return;
        //         }
        //         else // Thumbnail Path field found
        //         {
        //             auto map = song_data[index].toMap();
        //             map["value"] = thumbnail_path;
        //             song_data[index] = map;
        //         }
        //     }
        //     else
        //     {
        //         DB << "cannot set not existing thumbnail path" << thumbnail_path;
        //         DB << "auto setting thumbail works only in home environment";
        //     }
        // }
        {
            // set duration
            auto duration = QString::number(mp.duration());
            int index = lambda_get_map_index_by_id(7/*id of Duration field*/);
            if(index == -1)
            {
                song_data.append(
                    QVariant(
                        QVariantMap({{"id", 7/*id of Duration field*/},{"value", duration}})));
            }
            else
            {
                auto map = song_data[index].toMap();
                map["value"] = duration;
                song_data[index] = map;
            }
        }

    }

    {
        // update song date
        QDateTime dt;
        dt.setTimeZone(QTimeZone::systemTimeZone());
        auto current_time = QString::number(dt.currentSecsSinceEpoch());
        int index = lambda_get_map_index_by_id(12/*id of Update Date field*/);
        if(index == -1)
        {
            song_data.append(
                QVariant(
                    QVariantMap({{"id", 12/*id of Update Date field*/},{"value", current_time}})));
        }
        else
        {
            auto map = song_data[index].toMap();
            map["value"] = current_time;
            song_data[index] = map;
        }
    }


    BEGIN_TRANSACTION
    {
        // ------------------------------ add values to table songs_tags -----------------------------

        for(const auto &param : song_data)
        {

            auto map = param.toMap();
            if(!query.prepare("UPDATE songs_tags SET "
                               "value = :value "
                               "WHERE song_id = :song_id AND tag_id = :tag_id;"))
            {
                WR << "error while preparing INSERT INTO songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalEditSongError(
                    "error while preparing INSERT INTO songs_tags query " + query.lastError().text());
                return;
            }

            query.bindValue(":value", this->notNull(map["value"].toString()));
            query.bindValue(":song_id", song_id);
            query.bindValue(":tag_id", map["id"].toInt());

            this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());

            if(!query.exec())
            {
                WR << "error while executing UPDATE songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "values: {song_id: '" << song_id << "', tag_id: '" << map["id"].toInt()
                   << "', value: '" << map["value"].toString() << "'}";
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalEditSongError(
                    "error while executing UPDATE songs_tags query " + query.lastError().text());
                return;
            }
        }
    }
    END_TRANSACTION(signalEditSongError)

    ///
    /// NOTE: after these operations values in songs_tags won't be in numerical order (i mean tag_id)
    ///

    this->clear_models_memory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

    DB << "song ( id:" << song_id << ") edited correctly!";
    emit this->signalEditedSong();
}

void Database::deleteSong(int song_id)
{
    IS_DATABASE_OPEN(signalDeleteSongError)

    BEGIN_TRANSACTION
    {
        QSqlQuery query(m_database);
        if(!query.prepare("DELETE FROM songs WHERE id = :id")){
            WR << "error while preparing DELETE song query with id: " << song_id << ", error: " << query.lastError();
            emit this->signalDeleteSongError("error while preparing DELETE song query with id: "
                                             + QString::number(song_id) + ", error: " + query.lastError().text());
            return;
        }
        query.bindValue(":id", song_id);
        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing DELETE song query with id: " << song_id << ", error: " << query.lastError();
            emit this->signalDeleteSongError("error while executing DELETE song query with id: "
                                             + QString::number(song_id) + ", error: " + query.lastError().text());
            return;
        }

        // records in songs_tags, that contains deleted song_id will be also deleted because of constraint "ON DELETE CASCADE"
        // actually i find out that "ON DELETE CASCADE" is not working (but earlier worked :<)... then i will make it the hard way...

        if(!query.prepare("DELETE FROM songs_tags WHERE song_id = :id")){
            WR << "error while preparing DELETE songs_tags query with id: " << song_id << ", error: " << query.lastError();
            emit this->signalDeleteSongError("error while preparing DELETE songs_tags query with id: "
                                             + QString::number(song_id) + ", error: " + query.lastError().text());
            return;
        }
        query.bindValue(":id", song_id);
        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing DELETE songs_tags query with id: " << song_id << ", error: " << query.lastError();
            emit this->signalDeleteSongError("error while executing DELETE songs_tags query with id: "
                                             + QString::number(song_id) + ", error: " + query.lastError().text());
            return;
        }
    }
    END_TRANSACTION(signalDeleteSongError)

    this->clear_models_memory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

    DB << "song with id: " << song_id << " deleted correctly";
    emit this->signalDeletedSong();
}

void Database::addTag(QVariantList new_tag_data)
{
    IS_DATABASE_OPEN(signalAddTagError)

    /*
        received structure:

        new_tag_data - QVariantList:
            type - QString: "param" or "song"
            id - int: from QML fields
            value - QString: from QML fields

        example:
        QList(
            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "param"))
                    ("name", QVariant(QString, "Name"))
                    ("value", QVariant(QString, "some tag name"))
                )
            ),
            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "param"))
                    ("name", QVariant(QString, "Description"))
                    ("value", QVariant(QString, ""))
                )
            ),

            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "song"))
                    ("id", QVariant(int, 1))
                    ("value", QVariant(QString, "0"))
                )
            ),
            ...
        )

        NOTE: received structure contains only editable fields
    */

    // ------------------------------ test tag name ifis unique -----------------------------
    QString tag_name = new_tag_data[0/* 0 should be Name*/].toMap()["value"].toString();

    QSqlQuery query(m_database);
    QString query_text("SELECT id FROM tags WHERE name = :name;");
    if(!query.prepare(query_text))
    {
        WR << "error while prepating SELECT query " << query.lastError();
        emit this->signalAddTagError("error while prepating SELECT query " + query.lastError().text());
        return;
    }

    query.bindValue(":name", tag_name);

    this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
    if(!query.exec())
    {
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalAddTagError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    if(query.next())
    {
        // if select query return any records, that means in db exist tag with the same name
        // can't be any other tag with the same name as given
        DB << "Tag name should be unique, but this name belong to Tag with id =" << query.value(0).toInt();
        emit this->signalAddTagError(
            "Tag name should be unique, but this name belong to Tag with id = " + query.value(0).toString());
        return;
    }

    int tag_id;

    // ------------------------------ split data params and songs -----------------------------

    QMap<QString, QString> got_params;
    QMap<int, QString> got_songs;
    for(const auto &i : new_tag_data)
    {
        auto map = i.toMap(); // Name, Description, Type
        if(map["delegate_type"].toString() == "param")
            got_params.insert(map["name"].toString(), map["value"].toString());
        else if(map["delegate_type"].toString() == "song")
            got_songs.insert(map["id"].toInt(), map["value"].toString());
    }

    BEGIN_TRANSACTION
    {
        // ------------------------------ add new tag to database -----------------------------

        if(!query.prepare("INSERT INTO tags "
                           "(name, description, add_date, update_date, "
                           "type, is_immutable, is_editable, is_required) "
                           "VALUES "
                           "(:name, :desc, :addd, :updd, :type, 0, 1, 0);"))
        {
            WR << "error while preparing INSERT INTO tags query " << query.lastError();
            DB << "last query: " << query.lastQuery();
            DB << "cancelling transaction ...";
            m_database.rollback();
            emit this->signalAddTagError(
                "error while preparing INSERT INTO tags query " + query.lastError().text());
            return;
        }

        QDateTime dt;
        dt.setTimeZone(QTimeZone::systemTimeZone());
        auto current_time = dt.currentSecsSinceEpoch();

        query.bindValue(":name", this->notNull(got_params["Name"]));
        query.bindValue(":desc", this->notNull(got_params["Description"]));
        query.bindValue(":addd", current_time);
        query.bindValue(":updd", current_time);
        query.bindValue(":type", got_params["Type"].toInt());

        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing INSERT INTO tags query " << query.lastError();
            DB << "cancelling transaction ...";
            m_database.rollback();
            emit this->signalAddTagError(
                "error while executing INSERT INTO tags query " + query.lastError().text());
            return;
        }

        tag_id = query.lastInsertId().toInt();

        // ------------------------------ add values to table songs_tags -----------------------------

        for(auto it = got_songs.constBegin(); it != got_songs.constEnd(); ++it)
        {
            if(!query.prepare("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (:song_id, :tag_id, :value);"))
            {
                WR << "error while preparing INSERT INTO songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalAddTagError(
                    "error while preparing INSERT INTO songs_tags query " + query.lastError().text());
                return;
            }

            query.bindValue(":song_id", it.key());
            query.bindValue(":tag_id", tag_id);
            query.bindValue(":value", this->notNull(it.value()));

            this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());

            if(!query.exec())
            {
                WR << "error while executing INSERT INTO songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "values: {song_id: '" << it.key() << "', tag_id: '" << tag_id
                   << "', value: '" << it.value() << "'}";
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalAddTagError(
                    "error while executing INSERT INTO songs_tags query " + query.lastError().text());
                return;
            }
        }
    }
    END_TRANSACTION(signalAddTagError)

    // after that, if user enter any model (allSongs, addSong, editTag, ...),
    // data for him need to be loaded again
    this->clear_models_memory();

    DB << "tag ( id:" << tag_id << ") added correctly!";
    emit this->signalAddedTag();
}

void Database::editTag(int tag_id, QVariantList tag_data)
{
    IS_DATABASE_OPEN(signalEditTagError)

    /*
        received structure:

        new_tag_data - QVariantList:
            type - QString: "param" or "song"
            id - int: from QML fields
            value - QString: from QML fields

        example:
        QList(
            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "param"))
                    ("name", QVariant(QString, "Name"))
                    ("value", QVariant(QString, "some tag name"))
                )
            ),
            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "param"))
                    ("name", QVariant(QString, "Description"))
                    ("value", QVariant(QString, ""))
                )
            ),

            QVariant(
                QVariantMap,
                QMap(
                    ("delegate_type", QVariant(QString, "song"))
                    ("id", QVariant(int, 1))
                    ("value", QVariant(QString, "0"))
                )
            ),
            ...
        )

        NOTE: received structure contains only editable fields
    */

    // ------------------------------ test tag name if is unique -----------------------------
    QString tag_name = tag_data[0/* 0 should be Name*/].toMap()["value"].toString();

    QSqlQuery query(m_database);
    QString query_text("SELECT id FROM tags WHERE name = :name;");
    if(!query.prepare(query_text))
    {
        WR << "error while prepating SELECT query " << query.lastError();
        emit this->signalEditTagError("error while prepating SELECT query " + query.lastError().text());
        return;
    }

    query.bindValue(":name", tag_name);

    this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
    if(!query.exec())
    {
        WR << "error while executing SELECT query " << query.lastError();
        emit this->signalEditTagError("error while executing SELECT query " + query.lastError().text());
        return;
    }

    if(query.next())
    {
        if(query.value(0).toInt() != tag_id)
        {
            // if select query return any record with different id that current
            //   that means in db exist tag with the same name as given
            // if return record with the same id, that means user not changed tag name while editing
            DB << "Tag name should be unique, but this name belong to Tag with id =" << query.value(0).toInt();
            emit this->signalEditTagError(
                "Tag name should be unique, but this name belong to Tag with id = " + query.value(0).toString());
            return;
        }
    }

    // ------------------------------ split data params and songs -----------------------------

    QMap<QString, QString> got_params;
    QMap<int, QString> got_songs;
    for(const auto &i : tag_data)
    {
        auto map = i.toMap(); // Name, Description
        if(map["delegate_type"].toString() == "param")
            got_params.insert(map["name"].toString(), map["value"].toString());
        else if(map["delegate_type"].toString() == "song")
            got_songs.insert(map["id"].toInt(), map["value"].toString());
    }

    BEGIN_TRANSACTION
    {
        // ------------------------------ update tag in database -----------------------------

        if(!query.prepare("UPDATE tags SET "
                           "name = :name, description = :desc, update_date = :updd "
                           "WHERE id = :id;"))
        {
            WR << "error while preparing UPDATE tags query " << query.lastError();
            DB << "last query: " << query.lastQuery();
            DB << "cancelling transaction ...";
            m_database.rollback();
            emit this->signalEditTagError(
                "error while preparing UPDATE tags query " + query.lastError().text());
            return;
        }

        QDateTime dt;
        dt.setTimeZone(QTimeZone::systemTimeZone());
        auto current_time = dt.currentSecsSinceEpoch();

        query.bindValue(":name", this->notNull(got_params["Name"]));
        query.bindValue(":desc", this->notNull(got_params["Description"]));
        query.bindValue(":updd", current_time);
        query.bindValue(":id", tag_id);

        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing UPDATE tags query " << query.lastError();
            DB << "cancelling transaction ...";
            m_database.rollback();
            emit this->signalEditTagError(
                "error while executing UPDATE tags query " + query.lastError().text());
            return;
        }

        // ------------------------------ update values in table songs_tags -----------------------------

        for(auto it = got_songs.constBegin(); it != got_songs.constEnd(); ++it)
        {
            if(!query.prepare("UPDATE songs_tags SET value = :value WHERE song_id = :song_id AND tag_id = :tag_id;"))
            {
                WR << "error while preparing UPDATE songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalEditTagError(
                    "error while preparing UPDATE songs_tags query " + query.lastError().text());
                return;
            }

            query.bindValue(":value", this->notNull(it.value()));
            query.bindValue(":song_id", it.key());
            query.bindValue(":tag_id", tag_id);

            this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());

            if(!query.exec())
            {
                WR << "error while executing UPDATE songs_tags query " << query.lastError();
                DB << "last query: " << query.lastQuery();
                DB << "values: {song_id: '" << it.key() << "', tag_id: '" << tag_id
                   << "', value: '" << it.value() << "'}";
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalEditTagError(
                    "error while executing UPDATE songs_tags query " + query.lastError().text());
                return;
            }
        }
    }
    END_TRANSACTION(signalEditTagError)

    // after that, if user enter any model (allSongs, addSong, editTag, ...),
    // data for him need to be loaded again
    this->clear_models_memory();

    DB << "tag ( id:" << tag_id << ") updated correctly!";
    emit this->signalEditedTag();
}

void Database::deleteTag(int tag_id)
{
    IS_DATABASE_OPEN(signalDeleteTagError)

    BEGIN_TRANSACTION
    {
        QSqlQuery query(m_database);
        if(!query.prepare("DELETE FROM tags WHERE id = :id")){
            WR << "error while preparing DELETE tag query with id: " << tag_id << ", error: " << query.lastError();
            emit this->signalDeleteTagError("error while preparing DELETE tag query with id: "
                                            + QString::number(tag_id) + ", error: " + query.lastError().text());
            return;
        }
        query.bindValue(":id", tag_id);
        DB << query.boundValues();
        // this->queryToFile();
        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing DELETE tag query with id: " << tag_id << ", error: " << query.lastError();
            emit this->signalDeleteTagError("error while executing DELETE tag query with id: "
                                            + QString::number(tag_id) + ", error: " + query.lastError().text());
            return;
        }

        // records in songs_tags, that contains deleted tag_id will be also deleted because of constraint "ON DELETE CASCADE"
        // actually i find out that "ON DELETE CASCADE" is not working (but earlier worked :<)... then i will make it the hard way...

        if(!query.prepare("DELETE FROM songs_tags WHERE tag_id = :id")){
            WR << "error while preparing DELETE songs_tags query with id: " << tag_id << ", error: " << query.lastError();
            emit this->signalDeleteTagError("error while preparing DELETE songs_tags query with id: "
                                            + QString::number(tag_id) + ", error: " + query.lastError().text());
            return;
        }
        query.bindValue(":id", tag_id);
        DB << query.boundValues();
        // this->queryToFile();
        this->queryToFile(query.lastQuery(), query.boundValueNames(), query.boundValues());
        if(!query.exec()){
            WR << "error while executing DELETE songs_tags query with id: " << tag_id << ", error: " << query.lastError();
            emit this->signalDeleteTagError("error while executing DELETE songs_tags query with id: "
                                            + QString::number(tag_id) + ", error: " + query.lastError().text());
            return;
        }
    }
    END_TRANSACTION(signalDeleteTagError)

    this->clear_models_memory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

    DB << "tag with id: " << tag_id << " deleted correctly";
    emit this->signalDeletedTag();
}

void Database::editPlaylistSong(int song_id, QVariantList song_data)
{
    // NOTE: loaded data, can be reached by m_edit_song_model

    QString load_error;
    auto lambda_model = [&](QString desc){
        /// [&] means get reference from parents variables
        /// desc is value received from signalEditSongModelLoadError(QString desc)
        load_error = desc; // some warning occur, but i don't see a better way
    };

    auto connection_model = connect(this, &Database::signalEditSongError, lambda_model);

    this->editSong(song_id, song_data);

    if(load_error != ""){
        WR << "Error while editing song for editing playlist song:" << load_error;
        emit this->signalEditPlaylistSongError("Error while editing song for editing playlist song:" + load_error);
        return;
    }
    disconnect(connection_model);

    emit this->signalEditedPlaylistSong();
}

// void Database::deletePlaylistSong(int song_id)
// {
//     // NOTE: loaded data, can be reached by m_edit_song_model

//     QString load_error;
//     auto lambda_model = [&](QString desc){
//         /// [&] means get reference from parents variables
//         /// desc is value received from signalEditSongModelLoadError(QString desc)
//         load_error = desc; // some warning occur, but i don't see a better way
//     };

//     auto connection_model = connect(this, &Database::signalDeleteSongError, lambda_model);

//     this->deleteSong(song_id);

//     if(load_error != ""){
//         WR << "Error while loading edit song model for edit playlist model:" << load_error;
//         emit this->signalDeletePlaylistSongError("Error while loading edit song model for edit playlist model: " + load_error);
//         return;
//     }
//     disconnect(connection_model);

//     emit this->signalDeletedPlaylistSong();
// }
