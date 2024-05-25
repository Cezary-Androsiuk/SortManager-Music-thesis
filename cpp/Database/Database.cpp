#include "Database.h"

Database::Database(QObject *parent)
    : QObject{parent},
    m_databaseInitialized(false),
    m_saveExecQuery(false),
    m_showConstantTags(true),
    m_filters(nullptr),
    m_all_songs_model(nullptr),
    m_add_song_model(nullptr),
    m_edit_song_model(nullptr),
    m_all_tags_model(nullptr),
    m_add_tag_model(nullptr),
    m_edit_tag_model(nullptr),
    m_playlist_model(nullptr),
    m_filters_model(nullptr)
    // m_songs_main_path("")
{
    // DB << "DELETE OLD DATABASE" << " remove output: " << QFile(DATABASE_PATH).remove();
    // qDebug() << "\t\t+++ Database " << this;

    QObject::connect(this, &Database::signalInitializedOnStart, this, &Database::initializeFilters);
    QObject::connect(this, &Database::signalInitializedWithTags, this, &Database::initializeFilters);


    QObject::connect(this, &Database::signalFiltersInitailized, this, &Database::loadPlaylistList);
    QObject::connect(this, &Database::signalFiltersInitailized, this, &Database::loadPlaylistModel);

    QObject::connect(this, &Database::signalPlaylistRefreshed, this, &Database::loadPlaylistList);
    QObject::connect(this, &Database::signalPlaylistRefreshed, this, &Database::loadPlaylistModel);

    QObject::connect(this, &Database::signalFiltersUpdated, this, &Database::loadPlaylistList);
    QObject::connect(this, &Database::signalFiltersUpdated, this, &Database::loadPlaylistModel);
}

Database::~Database()
{
    // qDebug() << "\t\t--- Database " << this;

    // all dynamically allocated variables are deleted along with the Database instance is deleting
}
void Database::setSaveExecQuery(bool saveExecQuery)
{
    m_saveExecQuery = saveExecQuery;
}

void Database::setShowConstantTags(bool showConstantTags)
{
    m_showConstantTags = showConstantTags;
    // to remove all_tags_model (if was loaded)
    this->clearModelsMemory();
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
    m_databaseInitialized = true;
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
    m_databaseInitialized = true;
    emit this->signalInitializedWithTags();
}

void Database::createExampleData()
{
    QSqlQuery query(m_database);
    QList<QString> stl;

#define TMP_SHORT_1 stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES "
#define TMP_SHORT_2 stl.append(QString(

    // add song 1
    stl.append("INSERT INTO songs DEFAULT VALUES;");
    TMP_SHORT_1 "(1, 1, 1);");
    TMP_SHORT_1 "(1, 2, 'example song title');");
    TMP_SHORT_1 "(1, 3, 'song author');");
    TMP_SHORT_1 "(1, 4, 'long description');");
    TMP_SHORT_1 "(1, 5, 10);");
    TMP_SHORT_1 "(1, 6, 999);");
    TMP_SHORT_1 "(1, 7, 1010);");
    TMP_SHORT_1 "(1, 8, 'tekst piosenki');");
    TMP_SHORT_1 "(1, 9, 'path//');");
    TMP_SHORT_1 "(1, 10, 'path//');");
    TMP_SHORT_1 "(1, 11, 1711679000);");
    TMP_SHORT_1 "(1, 12, 1711679000);");

    // add song 2
    stl.append("INSERT INTO songs DEFAULT VALUES;");
    TMP_SHORT_1 "(2, 1, 2);");
    TMP_SHORT_1 "(2, 2, 'new song');");
    TMP_SHORT_1 "(2, 3, 'new author');");
    TMP_SHORT_1 "(2, 4, 'longer description');");
    TMP_SHORT_1 "(2, 5, 10);");
    TMP_SHORT_1 "(2, 6, 999);");
    TMP_SHORT_1 "(2, 7, 1010);");
    TMP_SHORT_1 "(2, 8, 'tekst piosenki');");
    TMP_SHORT_1 "(2, 9, 'path//');");
    TMP_SHORT_1 "(2, 10, 'path//');");
    TMP_SHORT_1 "(2, 11, 1711679000);");
    TMP_SHORT_1 "(2, 12, 1711679000);");

    // add tag 1
    TMP_SHORT_2 "INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) "
                "VALUES ('own tag', 'own tag desc', 1711679891, 1711679891, 2, 0, 1, 0);"));
    TMP_SHORT_2 "INSERT INTO songs_tags (song_id, tag_id) SELECT songs.id AS song_id, 13 AS tag_id FROM songs;"));
    TMP_SHORT_2 "UPDATE songs_tags SET value = '0' WHERE song_id = 1 AND tag_id = 13;"));
    TMP_SHORT_2 "UPDATE songs_tags SET value = '1' WHERE song_id = 2 AND tag_id = 13;"));

    // add tag 2
    TMP_SHORT_2 "INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) "
                "VALUES ('own more flexible tag', 'flexible desc', 1711679891, 1711679891, 1, 0, 1, 0);"));
    TMP_SHORT_2 "INSERT INTO songs_tags (song_id, tag_id) SELECT songs.id AS song_id, 14 AS tag_id FROM songs;"));
    TMP_SHORT_2 "UPDATE songs_tags SET value = '%1' WHERE song_id = 1 AND tag_id = 14;").arg(""));
    TMP_SHORT_2 "UPDATE songs_tags SET value = '%1' WHERE song_id = 2 AND tag_id = 14;").arg("some text that user input while adding tag"));

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

void Database::initializeFilters()
{
    IS_DATABASE_OPEN(signalFiltersInitailizeFailed)

    // initailize with empty tag comparators (without any limits)

    QString errorCode = this->fillFiltersWithValidTags();
    if(!errorCode.isNull()){
        WR << errorCode;
        emit this->signalFiltersUpdateError(errorCode);
        return;
    }

    this->debugPrint_filters();

    DB << "filters initialized correctly";
    emit this->signalFiltersInitailized();
    // this will trigger PlaylistList and loadPlaylistModel
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

    m_databaseInitialized = false;

    this->clearModelsMemory();

    if(!QFile(DATABASE_PATH).remove())
    {
        WR << "Removing database file" << DATABASE_PATH << "failed";
        if(!m_database.open())
        {
            WR << "Reopening database failed after failed with removing database file " << DATABASE_PATH;
            m_databaseInitialized = true;
            emit this->signalDeleteDatabaseError("Reopening database failed after failed with removing database file " + DATABASE_PATH);
        }
        emit this->signalDeleteDatabaseError("Removing database file " + DATABASE_PATH + " failed");
        return;
    }

    DB << "Database deleted correctly!";
    emit this->signalDeletedDatabase();
}

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
    // method will be trigger only by signalFiltersInitailized, signalPlaylistRefreshed and signalFiltersUpdated

    DB << " - staring playlist load";
    if(m_playlist_model != nullptr){
        // DB << "playlist model was already loaded - skipped";
        // emit this->signalPlaylistModelLoaded();
        // return;
        delete m_filters_model;
    }
    m_filters_model = nullptr;

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
    DB << " - query executed";

    m_playlist_model = new SongList(this);

    // read selected data
    while(query.next()){
        DB << " - query iterate start";
        auto record = query.record();

        int song_id = record.value(0).toInt();
        QString song_title = record.value(1).toString();

        Song *song = new Song(m_playlist_model);
        song->set_id(song_id);
        song->set_title(song_title);
        // value is not needed

        m_playlist_model->songs().append(song);
        DB << " - query iterate stop";
    }

    DB << " - iteration finished";

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
        // delete m_filters_model;
    }
    // m_filters_model = nullptr;

    // use m_filters model

    m_filters_model = new TagList(this);
    for(const auto &filter : m_filters->c_ref_tags())
    {
        TagWithComparator *twc = static_cast<TagWithComparator*>(filter);
        TagWithComparator *twcNew = new TagWithComparator(this);
        twcNew->set_id(twc->get_id());
        twcNew->set_name(twc->get_name());
        twcNew->set_type(twc->get_type());
        twcNew->set_value(twc->get_value());
        twcNew->set_is_editable(twc->get_is_editable());
        twcNew->set_is_required(twc->get_is_required());
        twcNew->set_is_immutable(twc->get_is_immutable());
        twcNew->set_comparison_way(twc->get_comparison_way());
        twcNew->set_comparison_value(twc->get_comparison_value());
        m_filters_model->tags().append(twcNew);
    }

    this->debugPrintModel_filters();

    DB << "filters model loaded correctly!";
    emit this->signalFiltersModelLoaded();
}

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

    this->clearModelsMemory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

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

    this->clearModelsMemory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

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

    this->clearModelsMemory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

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
    this->clearModelsMemory();

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
    this->clearModelsMemory();

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

    this->clearModelsMemory(); // after that, if user enter any model (allSongs, addSong, editTag, ...), data for him will be loaded again

    DB << "tag with id: " << tag_id << " deleted correctly";
    emit this->signalDeletedTag();
}

void Database::refreshPlaylist()
{
    DB << "/////// start ///////";
    // emit this->signalPlaylistRefreshError("");
    // error will not be emited, cause there is no place for that

    // this->clearFiltersModelsMemory();
    DB << "/////// before signal ///////";
    DB << "refreshed playlist correctly";
    emit this->signalPlaylistRefreshed();
    // this will trigger PlaylistList and loadPlaylistModel
    DB << "/////// stop ///////";
}

void Database::updateFilters(QVariantList filters)
{
    DB << "/////// start ///////";
    IS_DATABASE_OPEN(signalFiltersInitailizeFailed)
    DB << filters;
    /*
        received structure:

        filters - QVariantList:
            id - int
            comparison_way - int
            comparison_value - QString

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

    QString errorCode = this->fillFiltersWithValidTags();
    if(!errorCode.isNull()){
        WR << errorCode;
        emit this->signalFiltersUpdateError(errorCode);
        return;
    }



    this->debugPrint_filters();
    // this->clearFiltersModelsMemory();
    DB << "/////// before signal ///////";
    DB << "filters updated correctly";
    emit this->signalFiltersUpdated();
    // this will trigger PlaylistList and loadPlaylistModel
    DB << "/////// stop ///////";
}

void Database::loadPlaylistList()
{
    DB << "/////// start ///////";
    // create list
    // list will be deleted in signal received

    DB << "/////// before signal ///////";
    DB << "playlist list loaded correctly";
    emit this->signalPlaylistListLoaded(new TagList()/* RIP memory */);
    // this will trigger Playlist::loadPlaylist(TagList)
    DB << "/////// stop ///////";
}

void Database::clearModelsMemory()
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
}

void Database::clearPlaylistModelsMemory()
{
    if(m_playlist_model != nullptr)
        delete m_playlist_model;
    m_playlist_model = nullptr;
}

void Database::clearFiltersModelsMemory()
{
    if(m_filters_model != nullptr)
        delete m_filters_model;
    m_filters_model = nullptr;
}

QString Database::fillFiltersWithValidTags()
{
    QString query_text("SELECT id, name, type, is_editable, is_immutable FROM tags;");
    this->queryToFile(query_text);
    QSqlQuery query(m_database);
    if(!query.exec(query_text)){
        return QString("executing query " + query.lastError().text());
    }

    if(m_filters != nullptr)
        delete m_filters;
    m_filters = nullptr;

    m_filters = new TagList(this);

    // set values to variable from database (to contains values, only existing in db)
    while(query.next()){
        auto record = query.record();

        int tag_id = record.value(0).toInt();
        QString tag_name = record.value(1).toString();
        int tag_type = record.value(2).toInt();

        TagWithComparator* tag = new TagWithComparator(m_filters);
        tag->set_id(tag_id);
        tag->set_name(tag_name);
        tag->set_type(tag_type);
        tag->set_comparison_way(0);
        tag->set_comparison_value("");

        m_filters->tags().append(tag);
    }
    return QString(); // QString().isNull() == true
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
    if(!m_databaseInitialized){
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

void Database::debugPrint_filters() const
{
#if PRINT_MODELS_LISTS
    DB << "FILTERS:" << this->_debugPrintModel_TagList(m_filters).toStdString().c_str();
#endif
}

void Database::debugPrintModel_all_songs() const
{
#if PRINT_MODELS_LISTS
    DB << "ALL SONGS MODEL:" << this->_debugPrintModel_SongList(m_all_songs_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_add_song() const
{
#if PRINT_MODELS_LISTS
    DB << "ADD SONG MODEL:" << this->_debugPrintModel_SongDetails(m_add_song_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_edit_song() const
{
#if PRINT_MODELS_LISTS
    DB << "EDIT SONG MODEL:" << this->_debugPrintModel_SongDetails(m_edit_song_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_all_tags() const
{
#if PRINT_MODELS_LISTS
    DB << "ALL TAGS MODEL:" << this->_debugPrintModel_TagList(m_all_tags_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_add_tag() const
{
#if PRINT_MODELS_LISTS
    DB << "ADD TAG MODEL:" << this->_debugPrintModel_TagDetails(m_add_tag_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_edit_tag() const
{
#if PRINT_MODELS_LISTS
    DB << "EDIT TAG MODEL:" << this->_debugPrintModel_TagDetails(m_edit_tag_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_playlist() const
{
#if PRINT_MODELS_LISTS
    DB << "PLAYLIST MODEL:" << this->_debugPrintModel_SongList(m_playlist_model).toStdString().c_str();
#endif
}

void Database::debugPrintModel_filters() const
{
#if PRINT_MODELS_LISTS
    DB << "FILTERS MODEL:" << this->_debugPrintModel_TagList(m_filters_model).toStdString().c_str();
#endif
}

QString Database::_debugPrintModel_SongList(const SongList * const model)
{
    /* const pointer and const variable that he points to, thats why:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y; */

    QString obj_data("\n[");
    for(const auto &s : model->c_ref_songs()){
        obj_data += QString("\n   {id: '%1', title: '%2', value: '%3'}")
                        .arg(s->get_id())
                        .arg(s->get_title(),
                             s->get_value());
        obj_data += (s == model->c_ref_songs().last() ? "" : ", ");
    }
    return obj_data + "\n]";
}

QString Database::_debugPrintModel_SongDetails(const SongDetails* const model)
{
    /* const pointer and const variable that he points to, thats why:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y; */

    QString obj_data( QString("\n{"
                             "\n   song_id: '%1', "
                             "\n   tags: [")
                         .arg(model->get_id()) );
    for(const auto &t : model->get_tags()->c_ref_tags()){
        obj_data += QString("\n      {id: '%1', name: '%2', value: '%3', type: '%4', "
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
    return obj_data + "\n   ]"
                      "\n}";
}

QString Database::_debugPrintModel_TagList(const TagList * const model)
{
    /* const pointer and const variable that he points to, thats why:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y; */

    QString obj_data("\n[");
    for(const auto &t : model->c_ref_tags()){
        obj_data += QString("\n   {id: '%1', name: '%2', value: '%3', type: '%4', "
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
    return obj_data + "\n]";
}

QString Database::_debugPrintModel_TagDetails(const TagDetails* const model)
{
    /* const pointer and const variable that he points to, thats why:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y; */

    QString obj_data(QString("\n{"
                             "\n   id: '%1', "
                             "\n   name: '%2', "
                             "\n   description: '%3', "
                             "\n   add_date: '%4', "
                             "\n   update_date: '%5', "
                             "\n   type: '%6', "
                             "\n   is_immutable: '%7', "
                             "\n   is_editable: '%8', "
                             "\n   is_required: '%9', "
                             "\n   songs: [")
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
        obj_data += QString("\n      {id: '%1', title: '%2', value: '%3'}")
                        .arg(s->get_id())
                        .arg(s->get_title(),
                             s->get_value());
        obj_data += (s == model->get_songs()->c_ref_songs().last() ? "" : ", ");
    }
    return obj_data + "\n   ]"
                      "\n}";
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
    // if(!m_databaseInitialized){
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
    // emit this->signalPlaylistModelLoaded();
    return QSqlQuery();
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
