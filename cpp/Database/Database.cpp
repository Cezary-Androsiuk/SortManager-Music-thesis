#include "Database.h"

Database::Database(QObject *parent)
    : QObject{parent},
    m_database_initialized(false),
    m_saveExecQuery(false),
    m_showConstantTags(true),
    m_all_songs_model(nullptr),
    m_add_song_model(nullptr),
    m_edit_song_model(nullptr),
    m_all_tags_model(nullptr),
    m_add_tag_model(nullptr),
    m_edit_tag_model(nullptr)
    // m_playlist_songs_model(nullptr)
    // m_songs_main_path("")
{
    // DB << "DELETE OLD DATABASE" << " remove output: " << QFile(DATABASE_PATH).remove();
    // qDebug() << "\t\t+++ Database " << this;
}

Database::~Database()
{
    // qDebug() << "\t\t--- Database " << this;

    // all dynamically allocated variables are deleted along with the Database instance is deleting
}

void Database::initializeOnStart()
{
    if(!QFile(DATABASE_PATH).exists())
    {
        DB << "database doesn't exist";
        DB << "emiting signal signalDatabaseNotFound to QML!";
        emit this->signalInitializeOnStartFailed("Database File Not Found");
        return;
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(DATABASE_PATH);

    if(!m_database.open()){
        WR << "database oppening failed!";
        DB << "emiting signal signalDatabaseError to QML!";
        emit this->signalInitializeOnStartFailed(m_database.lastError().text());
        return;
    }

    DB << "database initialized on start correctly!";
    m_database_initialized = true;
    emit this->signalInitializedOnStart();
}

void Database::initializeWithTags()
{
    // if database doesn't exist then also query log file shouldn't exist, but if exist rename it as archive
    if(QFile::exists(QUERY_LOG_PATH)){
        DB << "query log file " << QUERY_LOG_PATH << " already exist while creating new database ";
        QFile::remove(QUERY_ARCHIVE_LOG_PATH);
        QFile(QUERY_LOG_PATH).rename(QUERY_ARCHIVE_LOG_PATH);
        QFile::remove(QUERY_LOG_PATH);
    }

    // create database file
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(DATABASE_PATH);
    if(!m_database.open())
    {
        WR << "creating database in " << DATABASE_PATH << " failed while oppening! " << m_database.lastError();
        emit this->signalInitializeWithTagsFailed("oppening created database in " +
                                            DATABASE_PATH + " failed! " + m_database.lastError().text());
        return;
    }
    // get current date
    QDateTime dt;
    dt.setTimeZone(QTimeZone::systemTimeZone());
    auto add_date = dt.currentSecsSinceEpoch();

    // set query list to execute
    QList<QString> exec_queries = {
        QString(
             "CREATE TABLE songs(                                      "
             "    id            INTEGER     PRIMARY KEY AUTOINCREMENT  "
             ");                                                       "),
        QString(
             "CREATE TABLE tags(                                       "
             "    id            INTEGER     PRIMARY KEY AUTOINCREMENT, "
             "    name          TEXT        NOT NULL UNIQUE,           "
             "    description   TEXT        NOT NULL,                  "
             "    add_date      INTEGER     NOT NULL,                  "
             "    update_date   INTEGER     NOT NULL,                  "
             "    type          INTEGER     NOT NULL,                  " // TagType enum
             "    is_immutable  INTEGER     NOT NULL,                  " // \
                is_immutable - true means that the title (etc.) of the tag can not be changed
             "    is_editable   INTEGER     NOT NULL,                  " // \
                is_editable - false means that value of the tag related with song can not be changed by user
             "    is_required   INTEGER     NOT NULL                   " // \
                is_required - true means that field of the tag can not be left blank
             ");                                                       "),// \
            example 1: "Title" tag cannot change name to "Home", then is_immutable=true but \
                value related with this tag "Despacito" can be changed to "Despacito cover", then is_editable=true\
            example 2: "Add Date" tag cannot change name to "School", then is_immutable=true but \
                value related with this tag "2024-03-29" can not be changed to "2020-01-01", then is_editable=false
        QString(
             "CREATE TABLE songs_tags(                                             "
             "    id           INTEGER     PRIMARY KEY AUTOINCREMENT,              "
             "    song_id      INTEGER     NOT NULL,                               "
             "    tag_id       INTEGER     NOT NULL,                               "
             "    value        TEXT        NOT NULL DEFAULT '',                    "
             "                                                                     "
             "    FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE,    "
             "    FOREIGN KEY (tag_id)  REFERENCES tags(id)  ON DELETE CASCADE     "
             ");                                                                   "),

        // SHORTCUTS:
#define TAG_INS_1 "INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) "
#define TAG_INS_2 "VALUES('%1', '%2', %3, %3, %4, 1, "
#define TAG_END_INT arg(add_date).arg(Database::TagType::TT_INTEGER)
#define TAG_END_STR arg(add_date).arg(Database::TagType::TT_TEXT)
        QString(TAG_INS_1 TAG_INS_2 "0,0);") .arg("ID",               "")     .TAG_END_INT,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Title",            "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Author",           "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Description",      "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Begin (ms)",       "")     .TAG_END_INT,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("End (ms)",         "")     .TAG_END_INT,
        QString(TAG_INS_1 TAG_INS_2 "0,0);") .arg("Duration (ms)",    "")     .TAG_END_INT,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Lyrics",           "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "1,1);") .arg("Song Path",        "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "1,0);") .arg("Thumbnail Path",   "")     .TAG_END_STR,
        QString(TAG_INS_1 TAG_INS_2 "0,0);") .arg("Add Date",         "")     .TAG_END_INT,
        QString(TAG_INS_1 TAG_INS_2 "0,0);") .arg("Update Date",      "")     .TAG_END_INT,
    };

    // start transaction (all tables or nothing)
    BEGIN_TRANSACTION
    {
        QSqlQuery query(m_database);

        for(const auto &exec_query : exec_queries)
        {
            this->queryToFile(exec_query);
            if(!query.exec(exec_query))
            {
                WR << "error while executing query: " << exec_query << " " << query.lastError();
                DB << "cancelling...";
                m_database.rollback();
                emit this->signalInitializeWithTagsFailed(
                    "error while executing " + exec_query + " " + query.lastError().text());
                return;
            }
        }
    }
    END_TRANSACTION(signalInitializeWithTagsFailed)

    DB << "database initialized with tags correctly!";
    m_database_initialized = true;
    emit this->signalInitializedWithTags();
}

void Database::createExampleData()
{
    QSqlQuery query(m_database);
    QList<QString> stl;
    // add song 1
    stl.append("INSERT INTO songs DEFAULT VALUES;");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 1, 1);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 2, 'example song title');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 3, 'song author');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 4, 'long description');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 5, 10);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 6, 999);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 7, 1010);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 8, 'tekst piosenki');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 9, 'path//');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 10, 'path//');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 11, 1711679000);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 12, 1711679000);");

    // add song 2
    stl.append("INSERT INTO songs DEFAULT VALUES;");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 1, 2);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 2, 'new song');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 3, 'new author');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 4, 'longer description');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 5, 10);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 6, 999);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 7, 1010);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 8, 'tekst piosenki');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 9, 'path//');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 10, 'path//');");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 11, 1711679000);");
    stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 12, 1711679000);");

#define TMP_SHORT stl.append(QString(
    // add tag 1
    TMP_SHORT"INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('own tag', 'own tag desc', 1711679891, 1711679891, 2, 0, 1, 0);"));
    TMP_SHORT"INSERT INTO songs_tags (song_id, tag_id) SELECT songs.id AS song_id, 13 AS tag_id FROM songs;"));
    TMP_SHORT"UPDATE songs_tags SET value = '0' WHERE song_id = 1 AND tag_id = 13;"));
    TMP_SHORT"UPDATE songs_tags SET value = '1' WHERE song_id = 2 AND tag_id = 13;"));

    // add tag 2
    TMP_SHORT"INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('own more flexible tag', 'flexible desc', 1711679891, 1711679891, 1, 0, 1, 0);"));
    TMP_SHORT"INSERT INTO songs_tags (song_id, tag_id) SELECT songs.id AS song_id, 14 AS tag_id FROM songs;"));
    TMP_SHORT"UPDATE songs_tags SET value = '%1' WHERE song_id = 1 AND tag_id = 14;").arg(""));
    TMP_SHORT"UPDATE songs_tags SET value = '%1' WHERE song_id = 2 AND tag_id = 14;").arg("some text that user input while adding tag"));

    for(const auto &st : stl)
    {
        this->queryToFile(st);
        if(!query.exec(st))
        {
            WR << "error while executing statement: " << st;
            WR << "error while executing statement: " << query.lastError();
            exit(1);
        }

        this->queryToFile(st);
    }
}

void Database::setSaveExecQuery(bool saveExecQuery)
{
    m_saveExecQuery = saveExecQuery;
}

void Database::setShowConstantTags(bool showConstantTags)
{
    m_showConstantTags = showConstantTags;
    // to remove all_tags_model (if was loaded)
    this->clear_models_memory();
}

void Database::exportDatabase(const QUrl &output_qurl)
{
    QString output_file = output_qurl.toLocalFile();
    if(QFile::exists(output_file))
    {
        // user confirm overwrite file in select file form
        QFile(output_file).remove();
    }
    QObject ltp; // life time protector

    QSqlQuery query_songs(m_database);
    QSqlQuery query_song(m_database);
    QSqlQuery query_tags(m_database);
    QString query_text;

    QJsonObject main_json;
    QJsonArray songs_array_json;
    QJsonArray tags_array_json;

    // this can be done by loadAllSongsModel() and loadSongEditModel() (then get_all_songs_model and get_song_edit_model)
    // but i thought about that after writing the following code :/

    // ------------------------------ get all songs  -----------------------------
    // get songs list
    query_text = QString("SELECT id FROM songs;");
    this->queryToFile(query_text);
    if(!query_songs.exec(query_text)){
        WR << "error while executing SELECT query " << query_songs.lastError();
        emit this->signalExportDatabaseError("error while executing SELECT query " + query_songs.lastError().text());
        return;
    }

    while(query_songs.next())
    {
        QJsonObject song;
        int song_id = query_songs.value(0).toInt();
        // get tags for song
        query_text = QString("SELECT tags.name, tags.type, songs_tags.value "
                             "FROM songs_tags "
                             "JOIN tags ON songs_tags.tag_id = tags.id "
                             "WHERE songs_tags.song_id = %1;").arg(song_id);
        this->queryToFile(query_text);
        if(!query_song.exec(query_text)){
            WR << "error while executing query " << query_song.lastError();
            emit this->signalExportDatabaseError("error while executing query " + query_song.lastError().text());
            return;
        }

        // set all tags to QJsonObject
        while(query_song.next()){
            int tag_type = query_song.value(1).toInt();
            if(tag_type == 1) // String type
                song[query_song.value(0).toString()] = query_song.value(2).toString();
            else // TriSwich type or Integer type
                song[query_song.value(0).toString()] = query_song.value(2).toInt();
        }

        // add QJsonObject
        songs_array_json.append(song);
    }
    main_json["songs"] = songs_array_json;


    // ------------------------------ get all tags  -----------------------------
    // get songs list
    query_text = QString("SELECT * FROM tags"/*" WHERE is_immutable = 0"*/";");
    this->queryToFile(query_text);
    if(!query_tags.exec(query_text)){
        WR << "error while executing SELECT query " << query_tags.lastError();
        emit this->signalExportDatabaseError("error while executing SELECT query " + query_tags.lastError().text());
        return;
    }

    while(query_tags.next())
    {
        auto record = query_tags.record();

        QJsonObject tag;

        // set all tag parameters to QJsonObject
        for(int i=0; i<record.count(); i++){
            auto field_name = record.fieldName(i);
            if(field_name == "name" || field_name == "description")
                tag[field_name] = record.value(i).toString();
            else // rest of columns are integer
                tag[field_name] = record.value(i).toInt();
        }

        // add QJsonObject
        tags_array_json.append(tag);
    }
    main_json["tags"] = tags_array_json;

    QJsonDocument json_data(main_json);

    QFile file(output_file);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        WR << "error while saving json file to" << output_file << "with error" << file.errorString();
        emit this->signalExportDatabaseError(
            "error while saving json file to " + output_file + " with error " + file.errorString());
    }

    file.write(json_data.toJson());
    file.close();

    DB << "database exported successfully!";
    emit this->signalExportedDatabase();
}


/*
 * bellow is no hard comunication methods and user are not guided by the hand what was wrong
 * or interpret what user have on his mind. Just checking data, if user add "Song path" instead of "Song Path"
 * an error will shows up, if user spell wrong "Description" then this field will stay empty.
 *
 * Also everything needs to be fine with json file to commit changes
 *
 * NOTE: going through tags, algorithm don't react on additional fields if there is "name" and "type" fields
 *       then all is fine. Field "description" is also used, but only when exist, if not then will be empty
 *
 * NOTE: going through songs, algorithm will react on any field/key that doesn't exist in database and those
 *       that are specyfied, but tags are not editable like "Duration" or "Add Date", they won't be ignored!
*/
void Database::importTagsToDatabase(const QUrl &output_qurl)
{
    importDatabase(output_qurl);
}

void Database::importSongsToDatabase(const QUrl &output_qurl)
{
    importDatabase(output_qurl);
}

void Database::importDatabase(const QUrl &input_qurl)
{
    QString input_file = input_qurl.toLocalFile();
    if(!QFile(input_file).exists()){
        WR << "file " << input_file << " not found";
        emit this->signalImportDatabaseError("file " + input_file + " not found");
    }

    QFile json_file(input_file);
    if(!json_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        WR << "error while reading json file from" << input_file << "with error" << json_file.errorString();
        emit this->signalImportDatabaseError(
            "error while reading json file from " + input_file + " with error " + json_file.errorString());
    }

    QJsonParseError json_error;
    QJsonDocument main_json_doc = QJsonDocument::fromJson(json_file.readAll(), &json_error);
    json_file.close();

    if(json_error.error != QJsonParseError::NoError) {
        WR << "json parse error: " << json_error.errorString();
        emit this->signalImportDatabaseError("json parse error: " + json_error.errorString());
        return;
    }

    if(!main_json_doc.isObject()){
        WR << "json file does not contains json object";
        emit this->signalImportDatabaseError("json file does not contains json object");
        return;
    }

    QJsonObject main_json = main_json_doc.object();

    // ------------------------------ get data from database  -----------------------------
    QStringList used_song_paths;

    {
        // Get song paths
        QSqlQuery query(m_database);
        QString query_text("SELECT value FROM songs_tags WHERE tag_id = 9;"); // tag_id = 9 is Song Path tag
        this->queryToFile(query_text);
        if(!query.exec(query_text))
        {
            WR << "error while executing SELECT query:" << query.lastError();
            emit this->signalImportDatabaseError("error while executing SELECT query: " + query.lastError().text());
            return;
        }

        /// store song paths of used songs
        /// m_all_songs_model will be cleared after any operation like add song
        if(query.next())
            used_song_paths.append(query.value(0).toString());
    }


    BEGIN_TRANSACTION
    {
        // skip if json file not contains "tags" object
        /// user might want to add just songs, so error not needed
        if(main_json.contains("tags"))
        {
            // ------------------------------ get data about tags -----------------------------
            QStringList used_tag_names;
            QString load_error;

            auto lambda_model = [&](QString desc){
                /// [&] means get reference from parents variables
                /// desc is value received from signalAllTagsModelLoadError(QString desc)
                load_error = desc; // some warning occur, but i don't see a better way
            };

            // Get tags names
            /// load all tags and react for the error result
            auto connection_model = connect(this, &Database::signalAllTagsModelLoadError, lambda_model);

            // get ALL tags
            bool prev_show_constant_tags = m_showConstantTags;
            m_showConstantTags = true;
            this->loadAllTags();
            m_showConstantTags = prev_show_constant_tags;

            if(load_error != ""){
                WR << "Error while loading all tags model for tags part:" << load_error;
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalImportDatabaseError("Error while loading all tags model for tags part: " + load_error);
                return;
            }
            disconnect(connection_model);

            /// store names of used tags
            /// m_all_tags_model will be cleared after any operation like add tag
            for(const auto &tag : m_all_tags_model->c_ref_tags())
                used_tag_names.append(tag->get_name());

            // ------------------------------ add tags -----------------------------

            QString list_of_add_tags_failed;
            QStringList added_from_json_tag_names;
            QString last_tag_add_error;

            auto lambda_operation = [&](QString desc){
                // [&] means get reference from parents variables
                // desc is value received from signalAllTagsModelLoadError(QString desc)
                last_tag_add_error += desc;
                // some warning occur, but i don't see a better way
            };

            auto connection_operation = connect(this, &Database::signalAddTagError, lambda_operation);

            QJsonArray tags_array_json = main_json["tags"].toArray();
            // go through all tags in json
            for(const auto &_tag : tags_array_json)
            {
                QJsonObject tag = _tag.toObject();

                // get tag name
                if(!tag.contains("name"))
                {
                    list_of_add_tags_failed += "{one of the tags not contains field 'name'} ";
                    // not exit here because later app will display user what tags need an repair
                    continue;
                }
                QString tag_name = tag["name"].toString();

                // test if not used already
                if(used_tag_names.contains( tag_name ))
                {
                    list_of_add_tags_failed += "{tag name '"+tag_name+"' already in use} ";
                    // not exit here because later app will display user what tags need an repair
                    continue;
                }
                if(added_from_json_tag_names.contains( tag_name ))
                {
                    list_of_add_tags_failed += "{tag name '"+tag_name+"' is a duplicate in json} ";
                    // not exit here because later app will display user what tags need an repair
                    continue;
                }

                // prepare structure for addTag
                QString description = "";
                if(tag.contains("description"))
                    description = tag["description"].toString();


                if(!tag.contains("type")){
                    list_of_add_tags_failed += "{one of the tags not contains field 'type'} ";
                    continue;
                }
                int type = tag["type"].toInt();
                if(type<0 || 2<type){
                    list_of_add_tags_failed += "{tag type need to be 0, 1 or 2. But tag with name '"
                                               +tag_name+"' has value '"+
                                               QString::number(tag["type"].toInt())+"'} ";
                    continue;
                }

                QVariantList structure = {
                    QVariantMap{
                        {"delegate_type", QString("param")},
                        {"name", QString("Name")},
                        {"value", tag_name}
                    },
                    QVariantMap{
                        {"delegate_type", QString("param")},
                        {"name", QString("Description")},
                        {"value", description}
                    },
                    QVariantMap{
                        {"delegate_type", QString("param")},
                        {"name", QString("Type")},
                        {"value", type}
                    }
                };

                this->addTag(structure);

                if(!last_tag_add_error.isEmpty())
                {
                    list_of_add_tags_failed += "{while adding tag with name '"+tag_name+"', an error occur: "+last_tag_add_error+"} ";
                    last_tag_add_error.clear();
                    // not exit here because later app will display user what tags need an repair
                    continue;
                }

                added_from_json_tag_names.append( tag_name );
            }

            disconnect(connection_operation);

            if(!list_of_add_tags_failed.isEmpty())
            {
                // exit here, before adding songs, its because not adding one of new tags,
                //   could quickly escalate as multiple errors while adding songs
                DB << "found errors while adding tags:" << list_of_add_tags_failed;
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalImportDatabaseError(
                    "found errors while adding tags: " + list_of_add_tags_failed);
                return;
            }

            // if no error occur then marge lists of tag names and tag types
            // this help us later :)
            for(const auto &name : added_from_json_tag_names)
                used_tag_names.append(name);
        }


        // skip if json file not contains "songs" object
        /// user might want to add just tags, so error not needed
        if(main_json.contains("songs"))
        {
            // ------------------------------ get data about songs -----------------------------
            QStringList used_tag_names;
            TagList all_tags;
            QString load_error;

            auto lambda_model = [&](QString desc){
                /// [&] means get reference from parents variables
                /// desc is value received from signalAllTagsModelLoadError(QString desc)
                load_error = desc; // some warning occur, but i don't see a better way
            };

            // Get tags data
            /// load all tags and react for the error result
            auto connection_model = connect(this, &Database::signalAllTagsModelLoadError, lambda_model);

            bool prev_show_constant_tags = m_showConstantTags;
            m_showConstantTags = true;
            this->loadAllTags();
            m_showConstantTags = prev_show_constant_tags;

            if(load_error != ""){
                WR << "Error while loading all tags model for songs part:" << load_error;
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalImportDatabaseError("Error while loading all tags model for songs part: " + load_error);
                return;
            }

            disconnect(connection_model);

            /// store names of used tags
            /// m_all_tags_model will be cleared after any operation like add song
            for(const auto &mdl_tag : m_all_tags_model->c_ref_tags()){
                Tag *tag = new Tag(&all_tags);

                tag->set_id(mdl_tag->get_id());
                tag->set_name(mdl_tag->get_name());
                tag->set_type(mdl_tag->get_type());
                tag->set_is_editable(mdl_tag->get_is_editable());

                all_tags.tags().append(tag);
            }

            // ------------------------------ add songs -----------------------------

            QString list_of_add_songs_failed;
            QStringList added_from_json_song_paths;
            QString last_song_add_error;

            auto lambda_operation = [&](QString desc){
                // [&] means get reference from parents variables
                // desc is value received from signalAllTagsModelLoadError(QString desc)
                last_song_add_error += desc;
                // some warning occur, but i don't see a better way
            };

            auto connection_operation = connect(this, &Database::signalAddSongError, lambda_operation);

            QJsonArray songs_array_json = main_json["songs"].toArray();
            for(const auto &_song : songs_array_json)
            {
                QJsonObject song = _song.toObject();

                // get song path
                if(!song.contains("Song Path"))
                {
                    list_of_add_songs_failed += "{one of the songs not contains field 'Song Path'} ";
                    // not exit here because later app will display user what songs need an repair
                    continue;
                }
                QString song_path = song["Song Path"].toString();

                // check if is not used already
                if(used_tag_names.contains( song_path ))
                {
                    list_of_add_songs_failed += "{song path '"+song_path+"' already in use} ";
                    // not exit here because later app will display user what songs need an repair
                    continue;
                }
                if(added_from_json_song_paths.contains( song_path ))
                {
                    list_of_add_songs_failed += "{song path '"+song_path+"' is a duplicate in json} ";
                    // not exit here because later app will display user what songs need an repair
                    continue;
                }

                // prepare structure for addSong
                QVariantList structure;

                /// well... thats efficient XD 3th nested for loop
                auto lambda_set_given_in_json_values = [&]() -> bool{
                    // iterate over all "keys": values in current song
                    for (auto it = song.begin(); it != song.end(); ++it) {
                        QString key = it.key();
                        QJsonValue json_value = it.value();

                        bool found_related_tag = false;
                        for(const auto &tag : all_tags.c_ref_tags())
                        {
                            if(tag->get_name() == key)
                            {
                                found_related_tag = true;

                                if(!tag->get_is_editable())
                                {
                                    // error if json contains value for not editable tag \
                                    (example: tag "ID", "Duration" or "Add Date")
                                    list_of_add_songs_failed += "{found field that can't be set cause is not editable: '"+
                                                                key+"'} ";

                                    // lambda return
                                    return false;
                                }

                                QVariant qv_value;

                                if(tag->get_type() == 0) // state
                                {
                                    int value = json_value.toInt();
                                    if(value > 0)       qv_value = 1;
                                    else if(value < 0)  qv_value = -1;
                                    else                qv_value = 0;
                                }
                                else if(tag->get_type() == 1) // string
                                {
                                    qv_value = json_value.toString();
                                }
                                else if(tag->get_type() == 2) // integer
                                {
                                    qv_value = json_value.toInt();
                                }
                                else
                                {
                                    WR << "Error unknown tag in tags list! tag name:" << tag->get_name() << ",type:" << tag->get_type();
                                    DB << "Using 'string' type ";
                                    qv_value = json_value.toString();
                                }

                                structure.append(
                                    QVariantMap{
                                        {"id", tag->get_id()},
                                        {"value", qv_value}
                                    });

                                break; // i am ECO :)
                            }
                        }

                        if(!found_related_tag)
                        {
                            // error given tag in song does not exist
                            list_of_add_songs_failed +=
                                "{given tag '"+key+"' not exist  '" +
                                song["Song Path"].toString()+"' has tag named'"+
                                json_value.toString()+"'} ";

                            // lambda return
                            return false;
                        }
                    }
                    return true;
                };
                if(!lambda_set_given_in_json_values())
                {
                    // if error in song occur then continue from here
                    // list_of_add_tags_failed was set while returning lambda
                    continue;
                }

                auto lambda_field_is_in_structure = [&](int id) -> bool{
                    for(const auto &tag : structure)
                    {
                        if(tag.toMap()["id"].toInt() == id)
                            return true;
                    }
                    return false;
                };

                for(const auto &tag : all_tags.c_ref_tags())
                {
                    // skip not editable (those will be added in addSong())
                    if(!tag->get_is_editable()){
                        continue;
                    }
                    if(!lambda_field_is_in_structure(tag->get_id()))
                    {
                        QVariant qv_value; // default value
                        if(tag->get_type() == 0) // state
                        {
                            qv_value = 0;
                        }
                        else if(tag->get_type() == 1) // string
                        {
                            qv_value = QString("");
                        }
                        else if(tag->get_type() == 2) // integer
                        {
                            qv_value = 0;
                        }

                        structure.append(
                            QVariantMap{
                                {"id", tag->get_id()},
                                {"value", qv_value}
                            });
                    }
                }

                this->addSong(structure);

                if(!last_song_add_error.isEmpty())
                {
                    list_of_add_songs_failed += "{while adding song with song path '"+song_path+"', an error occur: "+last_song_add_error+"} ";
                    last_song_add_error.clear();
                    // not exit here because later app will display user what songs need an repair
                    continue;
                }

                added_from_json_song_paths.append(song_path);
            }

            disconnect(connection_operation);

            if(!list_of_add_songs_failed.isEmpty())
            {
                DB << "found errors while adding songs:" << list_of_add_songs_failed;
                DB << "cancelling transaction ...";
                m_database.rollback();
                emit this->signalImportDatabaseError(
                    "found errors while adding songs: " + list_of_add_songs_failed);
                return;
            }
        }
    }
    END_TRANSACTION(signalImportDatabaseError)


    DB << "database imported successfully!";
    emit this->signalImportedDatabase();
}

void Database::deleteDatabase()
{
    IS_DATABASE_OPEN(signalDeleteDatabaseError)
    m_database.close();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    m_database_initialized = false;

    this->clear_models_memory();

    if(!QFile(DATABASE_PATH).remove())
    {
        WR << "Removing database file" << DATABASE_PATH << "failed";
        if(!m_database.open())
        {
            WR << "Reopening database failed after failed with removing database file " << DATABASE_PATH;
            m_database_initialized = true;
            emit this->signalDeleteDatabaseError("Reopening database failed after failed with removing database file " + DATABASE_PATH);
        }
        emit this->signalDeleteDatabaseError("Removing database file " + DATABASE_PATH + " failed");
        return;
    }

    DB << "Database deleted correctly!";
    emit this->signalDeletedDatabase();
}

SongList *Database::get_all_songs_model() const
{
    return m_all_songs_model;
}

SongDetails *Database::get_edit_song_model() const
{
    return m_edit_song_model;
}

SongDetails *Database::get_add_song_model() const
{
    return m_add_song_model;
}

TagList *Database::get_all_tags_model() const
{
    return m_all_tags_model;
}

TagDetails *Database::get_edit_tag_model() const
{
    return m_edit_tag_model;
}

TagDetails *Database::get_add_tag_model() const
{
    return m_add_tag_model;
}

SongList *Database::get_playlist_model() const
{
    return m_playlist_model;
}

TagList *Database::get_filters_model() const
{
    return m_filters_model;
}


void Database::clear_models_memory()
{
    // remove loaded models from memory

    if(m_all_songs_model != nullptr)
        delete m_all_songs_model;
    m_all_songs_model = nullptr;

    if(m_add_song_model != nullptr)
        delete m_add_song_model;
    m_add_song_model = nullptr;

    if(m_edit_song_model != nullptr)
        delete m_edit_song_model;
    m_edit_song_model = nullptr;


    if(m_all_tags_model != nullptr)
        delete m_all_tags_model;
    m_all_tags_model = nullptr;

    if(m_add_tag_model != nullptr)
        delete m_add_tag_model;
    m_add_tag_model = nullptr;

    if(m_edit_tag_model != nullptr)
        delete m_edit_tag_model;
    m_edit_tag_model = nullptr;


    // if(m_playlist_songs_model != nullptr)
    //     delete m_playlist_songs_model;
    // m_playlist_songs_model = nullptr;
}

QString Database::notNull(const QString &value)
{
    // wierd and confusing... but i understand why
    // DB << "IS EMPTY: '" << QString().isNull() << "' ?"; // output: IS EMPTY: 'true' ?
    // DB << "IS EMPTY: '" << QString("").isNull() << "' ?"; // output: IS EMPTY: 'false' ?
    // this method is useful for binding strings in SQL Query
    return value.isNull() ? QString("") : value;
}

bool Database::isDatabaseOpen(void (Database::*signal)(QString), const char *caller_name)
{
    // using the following definition (which uses this function) shortens the code by ~60 lines
    // IS_DATABASE_OPEN(signalMethod)
    // can't be const (to emit signal can't function can't be constant)
    // can't be static (to emit signal method should be related with object (also i don't want to pass all required arguments))
    if(!m_database_initialized){
        WR << "database wasn't initialized! While executing"
           << caller_name << "method, error:" << m_database.lastError();
        emit (this->*signal)("database wasn't initialized, error: " + m_database.lastError().text());
        // emit not require caller_name because of signal name
        return false;
    }

    return true;
}

bool Database::beginTransaction(void (Database::*signal)(QString), const char *caller_name)
{
    // using the following definition (which uses this function) shortens the code by ~60 lines
    // IS_DATABASE_OPEN(signalMethod)
    // can't be const (to emit signal can't function can't be constant)
    // can't be static (to emit signal method should be related with object (also i don't want to pass all required arguments))

    // start transaction (all tables or nothing)
    if(!m_database.transaction()){
        WR << "creating transaction error! While executing"
           << caller_name << "method, error:" << m_database.lastError();
        emit (this->*signal)("creating transaction error, error: " + m_database.lastError().text());
        // emit not require caller_name because of signal name
        return false;
    }

    return true;
}

bool Database::endTransaction(void (Database::*signal)(QString), const char *caller_name)
{
    // using the following definition (which uses this function) shortens the code by ~60 lines
    // IS_DATABASE_OPEN(signalMethod)
    // can't be const (to emit signal can't function can't be constant)
    // can't be static (to emit signal method should be related with object (also i don't want to pass all required arguments))

    // finalize transaction
    if(!m_database.commit()){
        WR << "commiting transaction error! While executing"
           << caller_name << "method, error:" << m_database.lastError();
        emit (this->*signal)("commiting transaction error, error: " + m_database.lastError().text());
        // emit not require caller_name because of signal name
        return false;
    }

    return true;
}

void Database::queryToFile(QString query, QStringList param_names, QVariantList param_values) const
{
    if(!m_saveExecQuery)
        return;

    QFile file(QUERY_LOG_PATH);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        WR << "Error while oppening file " << QUERY_LOG_PATH;
        return;
    }

    // bind values
    if(param_names.size() != param_values.size())
    {
        WR << "param_names.size("<<param_names.size()<<") != param_values.size("<<param_values.size()<<")";

        QTextStream out(&file);
        out << " ?  " << query << "\n" << "\t - ";
        for(const auto &param : param_names)
            out << "'" << param << "', ";
        out << "\n" << "\t - ";
        for(const auto &param : param_values)
            out << "'" << param.toString() << "', ";
        out << "\n";
        file.close();
    }
    for(int i=0; i<param_names.size(); i++)
    {
        query.replace(param_names[i], "'" + param_values[i].toString() + "'");
    }
    QTextStream out(&file);
    out << " ?  " << query << "\n";
    file.close();

}


QSqlQuery Database::prepPlaylistSongsQuery()//cQls tc_names, cQls tc_values, cQls tc_comparators, cQls te_names, cQlb te_values, int *error_code) const
{
    // if(!m_database_initialized){
    //     WR << "database wasnot initalized";
    //     EC(1);
    //     return QSqlQuery();
    // }

    // if(tc_names.count() != tc_values.count() || tc_names.count() != tc_comparators.count()){
    //     WR << "error, size of given tc_names("
    //        << tc_names.count()
    //        << ") do not match size of tc_values(" <<
    //         tc_values.count() << ")";
    //     EC(10);
    //     return QSqlQuery();
    // }

    // if(te_names.count() != te_values.count()){
    //     WR << "error, size of given te_names("
    //        << te_names.count()
    //        << ") do not match size of te_values(" <<
    //         te_values.count() << ")";
    //     EC(20);
    //     return QSqlQuery();
    // }

    // if(tc_names.empty() && te_names.empty()){
    //     EC(0);
    //     return QSqlQuery("SELECT id, title FROM songs", this->m_database);
    // }

    // // prepare statement
    // QString statement("SELECT songs.id, songs.title FROM songs JOIN tags ON songs.id = tags.songs_id WHERE ");
    // for(int i=0; i<tc_names.count(); i++){
    //     statement += "songs." + tc_names[i] + " " + tc_comparators[i] + " :" + tc_names[i] + (i+1 < tc_names.size() ? " AND " : "");
    // }

    // if(te_names.count() > 0){
    //     statement += " AND ";
    //     for(int i=0; i<te_names.count(); i++){
    //         statement += "tags." + te_names[i] + " = :" +
    //                      te_names[i] + (i+1 < te_names.size() ? " AND " : "");
    //     }
    // }
    // QSqlQuery query(this->m_database);
    // if(!query.prepare(statement)){
    //     WR << "Error while preparing statement " << statement << "";
    //     EC(40);
    //     return QSqlQuery();
    // }

    // // bind values
    // for(int i=0; i<tc_names.count(); i++){
    //     query.bindValue(":" + tc_names[i], tc_values[i]);
    // }
    // for(int i=0; i<te_names.count(); i++){
    //     query.bindValue(":" + te_names[i], tc_values[i]);
    // }

    // EC(0);
    // return query;
    emit this->signalPlaylistModelLoaded();
    return QSqlQuery();
}
