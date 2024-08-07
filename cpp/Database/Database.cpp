#include "Database.h"

Database::Database(QObject *parent)
    : QObject{parent},
    m_cancelLoading(false),
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
    m_filters_model(nullptr)
{
    // DB << "DELETE OLD DATABASE" << " remove output: " << QFile(DATABASE_PATH).remove();

    QObject::connect(this, &Database::signalInitializedOnStart, this, &Database::initializeFilters);
    QObject::connect(this, &Database::signalInitializedWithTags, this, &Database::initializeFilters);
    ///
    ///
    QObject::connect(this, &Database::signalFiltersInitailized, this, &Database::loadPlaylist);
    ///
    QObject::connect(this, &Database::signalPlaylistRefreshed, this, &Database::loadPlaylist);
    ///
    QObject::connect(this, &Database::signalFiltersUpdated, this, &Database::loadPlaylist);

    /// connecting finish signals with loadingFinished signal
    QObject::connect(this, &Database::signalExportedSongsFromDatabase,      this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalExportSongsFromDatabaseError,   this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalExportedTagsFromDatabase,       this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalExportTagsFromDatabaseError,    this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalImportedSongsToDatabase,        this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalImportSongsToDatabaseError,     this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalImportedTagsToDatabase,         this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalImportTagsToDatabaseError,      this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalDeletedDatabase,                this, &Database::signalLoadingFinished);
    QObject::connect(this, &Database::signalDeleteDatabaseError,            this, &Database::signalLoadingFinished);

    /// after loading finished just for safety turn off m_cancelLoading
    /// especially in some methods that not uses signalLoadingProgress user might fast press the "cancel" button
    /// but nothing is there to cancel (methods are to fast)
    QObject::connect(this, &Database::signalLoadingFinished, this, &Database::finishedLoading);
}

Database::~Database()
{
    // all dynamically allocated variables are deleted along with the Database instance
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

void Database::cancelLoading()
{
    /// variable is checked each iteration and if is true
    /// then loading can be cancelled (but after that
    /// m_cancelLoading should be set to false)
    m_cancelLoading = true;
}

void Database::finishedLoading()
{
    /// after loading finished just for safety turn off m_cancelLoading
    /// especially in some methods that not uses signalLoadingProgress user might fast press the "cancel" button
    /// but nothing is there to cancel (methods are to fast)
    m_cancelLoading = false;
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
    DB << "creating example data...";
    QSqlQuery query(m_database);
    QList<QString> stl;

#define TMP_SHORT_1(X) stl.append("INSERT INTO songs_tags (song_id, tag_id, value) VALUES (" + songID + X)
#define TMP_SHORT_2(X) stl.append(QString(X))


    // add number tag
    TMP_SHORT_2("INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) "
                "VALUES ('Is a number song', 'tells if a song is a number around 1.5 seconds (google translate voice) and is "
                "used for test purposes', 1717550657, 1717550657, 2, 0, 1, 0);");
    // add letter tag
    TMP_SHORT_2("INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) "
                "VALUES ('Is a letter song', 'tells if a song is a letter around 16 seconds (google translate voice) and is "
                "used for test purposes', 1717550657, 1717550657, 2, 0, 1, 0);");

    // add numbers songs
    for(int i=0; i<9; i++)
    {
        // add song i
        QString songID = QString::number(i+1);
        QString songName = QString::number(i+101);
        stl.append("INSERT INTO songs DEFAULT VALUES;");
        TMP_SHORT_1(", 1, " + songID + ");");
        TMP_SHORT_1(", 2, 'example " + songName + " song');");
        TMP_SHORT_1(", 3, 'Google Translator');");
        TMP_SHORT_1(", 4, 'description of " + songName + " ');");
        TMP_SHORT_1(", 5, 0);");
        TMP_SHORT_1(", 6, 0);");
        TMP_SHORT_1(", 7, 1944);");
        TMP_SHORT_1(", 8, '');");
        TMP_SHORT_1(", 9, 'file:///" PROJ_PATH "/assets/example_songs/numbers-1.5s/" + songName + ".mp3');");
        TMP_SHORT_1(", 10, '');");
        TMP_SHORT_1(", 11, 1717550658);");
        TMP_SHORT_1(", 12, 1717550658);");
        TMP_SHORT_1(", 13, 1);"); // Is a number song
        TMP_SHORT_1(", 14, -1);"); // Is a letter song
    }

    // add letters songs
    for(int i=0; i<10; i++)
    {
        // add song x
        QString songID = QString::number(i+10);
        QString songName;
        if(i==0) songName = "a"; if(i==1) songName = "b"; if(i==2) songName = "c";
        if(i==3) songName = "e"; if(i==4) songName = "f"; if(i==5) songName = "g";
        if(i==6) songName = "h"; if(i==7) songName = "i"; if(i==8) songName = "w";
        if(i==9) songName = "z";
        stl.append("INSERT INTO songs DEFAULT VALUES;");
        TMP_SHORT_1(", 1, " + songID + ");");
        TMP_SHORT_1(", 2, 'example " + songName + " song');");
        TMP_SHORT_1(", 3, 'Google Translator');");
        TMP_SHORT_1(", 4, 'description of " + songName + " ');");
        TMP_SHORT_1(", 5, 0);");
        TMP_SHORT_1(", 6, 0);");
        TMP_SHORT_1(", 7, 16080);");
        TMP_SHORT_1(", 8, '');");
        TMP_SHORT_1(", 9, 'file:///" PROJ_PATH "/assets/example_songs/letters-16s/" + songName + ".mp3');");
        TMP_SHORT_1(", 10, '');");
        TMP_SHORT_1(", 11, 1717550659);");
        TMP_SHORT_1(", 12, 1717550659);");
        TMP_SHORT_1(", 13, -1);"); // Is a number song
        TMP_SHORT_1(", 14, 1);"); // Is a letter song
    }

    // DB << stl;

    int addedCounter = 0;
    for(const auto &st : stl)
    {
        this->queryToFile(st);
        if(!query.exec(st))
        {
            WR << "error while executing statement: " << st;
            WR << "error while executing statement: " << query.lastError();
            exit(1);
        }
        // DB << "executed" << ++addedCounter << "query";

        this->queryToFile(st);
    }
    DB << "finalizing adding an example data...";
    this->clearModelsMemory();
    this->makeCurrentFiltersValid();
    DB << "example data was added!";
}

void Database::initializeFilters()
{
    // somehow load filters from personalization

    /// initailize with empty tag comparators (without any limits) for now
    this->makeCurrentFiltersValid();

    this->debugPrint_filters();

    DB << "filters initialized correctly";
    emit this->signalFiltersInitailized(); /// this will trigger loadPlaylist
}

void Database::exportSongsFromDatabase(const QUrl &output_qurl)
{
    emit this->signalLoadingStarted("Exporting songs");
    /// in this method loading status will not show progess because
    /// is enough fast, but freeze while loading could be usefull

    IS_DATABASE_OPEN(signalExportSongsFromDatabaseError)
    QString output_file = output_qurl.toLocalFile();
    if(QFile::exists(output_file))
    {
        /// user will confirm overwrite file in select file form
        /// so in that step user don't might if I delete it
        QFile(output_file).remove();
    }

    /// fistly load list of songs
    QSqlQuery songsQuery(m_database);
    QString queryText = QString("SELECT id FROM songs;");
    this->queryToFile(queryText);
    if(!songsQuery.exec(queryText)){
        WR << "error while executing SELECT query " << songsQuery.lastError();
        emit this->signalExportSongsFromDatabaseError("error while executing SELECT query " + songsQuery.lastError().text());
        return;
    }
    
    /// secondly for each song load their tags and save it to json
    QJsonObject jsonMain;  /// json to export
    QJsonArray jsonSongs;   /// array of songs
    while(songsQuery.next())
    {
        int songID = songsQuery.value(0).toInt();
        QSqlQuery songQuery(m_database);
        const int TAG_NAME_INDEX = 0;
        const int TAG_TYPE_INDEX = 1;
        const int VALUE_INDEX = 2;
        queryText = QString("SELECT tags.name, tags.type, songs_tags.value "
                             "FROM songs_tags "
                             "JOIN tags ON songs_tags.tag_id = tags.id "
                             "WHERE songs_tags.song_id = %1;").arg(songID);
        this->queryToFile(queryText);
        if(!songQuery.exec(queryText)){
            WR << "error while executing query " << songQuery.lastError();
            emit this->signalExportSongsFromDatabaseError("error while executing query " + songQuery.lastError().text());
            return;
        }
        
        QJsonObject jsonSong;   /// song element (as temporary container)
        while(songQuery.next()){

            QString tagName = songQuery.value(TAG_NAME_INDEX).toString();
            bool isTextType = songQuery.value(TAG_TYPE_INDEX).toInt() == Database::TagType::TT_TEXT;
            auto tagValue = songQuery.value(VALUE_INDEX);
            
            /// if tag type is text type convert to string otherwise
            ///     (tag type is integer or TriState) convert to int
            if(isTextType)
                jsonSong[tagName] = tagValue.toString();
            else
                jsonSong[tagName] = tagValue.toInt();
            DB << tagValue;
        }
        
        jsonSongs.append(jsonSong);
    }
    jsonMain["songs"] = jsonSongs;

    QJsonDocument jsonData(jsonMain);

    QFile file(output_file);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        WR << "error while saving json file to" << output_file << "with error" << file.errorString();
        emit this->signalExportSongsFromDatabaseError(
            "error while saving json file to " + output_file + " with error " + file.errorString());
        return;
    }

    file.write(jsonData.toJson());
    file.close();

    DB << "songs exported successfully!";
    emit this->signalExportedSongsFromDatabase();
}

void Database::exportTagsFromDatabase(const QUrl &output_qurl)
{
    emit this->signalLoadingStarted("Exporting Tags");
    /// in this method loading status will not show progess because
    /// is enough fast, but freeze while loading could be usefull

    IS_DATABASE_OPEN(signalExportTagsFromDatabaseError)
    QString output_file = output_qurl.toLocalFile();
    if(QFile::exists(output_file))
    {
        /// user will confirm overwrite file in select file form
        /// so in that step user don't might if I delete it
        QFile(output_file).remove();
    }

    /// fistly load load all tags data
    QSqlQuery songsQuery(m_database);
    /// immutable tags are allways here UwU, so there is no need to export them
    QString queryText = QString("SELECT * FROM tags WHERE is_immutable = 0;");
    this->queryToFile(queryText);
    if(!songsQuery.exec(queryText)){
        WR << "error while executing SELECT query " << songsQuery.lastError();
        emit this->signalExportTagsFromDatabaseError("error while executing SELECT query " + songsQuery.lastError().text());
        return;
    }

    /// secondly save tags data to json variables
    QJsonObject jsonMain;  /// json to export
    QJsonArray jsonTags;   /// array of tags
    while(songsQuery.next())
    {
        auto record = songsQuery.record();
        QJsonObject tag;

        /// set all tag parameters to QJsonObject
        for(int i=0; i<record.count(); i++){
            auto fieldName = record.fieldName(i);
            bool isTextField = fieldName == "name" || fieldName == "description";

            /// if tag type is text type convert to string otherwise
            ///     (tag type is integer or TriState) convert to int
            if(isTextField)
                tag[fieldName] = record.value(i).toString();
            else
                tag[fieldName] = record.value(i).toInt();
        }

        jsonTags.append(tag);
    }
    jsonMain["tags"] = jsonTags;

    QJsonDocument jsonData(jsonMain);

    QFile file(output_file);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        WR << "error while saving json file to" << output_file << "with error" << file.errorString();
        emit this->signalExportTagsFromDatabaseError(
            "error while saving json file to " + output_file + " with error " + file.errorString());
        return;
    }

    file.write(jsonData.toJson());
    file.close();

    DB << "tags exported successfully!";
    emit this->signalExportedTagsFromDatabase();
}

void Database::importSongsToDatabase(const QUrl &input_qurl)
{
#define ERROR_SIGNAL this->signalImportSongsToDatabaseError
    /*
     * program while importing songs will look at each tag (in each song) that is writen in json file
     * and will show an error if any of this tags are unknown, in example: when user
     * spell something wrong
     */

    emit this->signalLoadingStarted("Importing Songs");

    QJsonObject jsonMain;
    try
    {
        jsonMain = this->importDatabaseLoadJsonFromFile(input_qurl).object();
    }
    CATCH;

    /// test if json file contains songs aray
    if(!jsonMain.contains("songs"))
    {
        HANDLE_ERROR(
            "file '" + input_qurl.toLocalFile() +
            "' does not contains 'songs' array")
    }

    /// load all songs once, instead of checking in db if each song path is unique
    QStringList usedSongPaths;
    try
    {
        usedSongPaths = this->importDatabaseGetUsedSongPaths();
    }
    CATCH;

    /// load all tags once, instead of checking in db if each tag exist
    /// variable will contain only tag names that exist in db and are editable
    QStringList editableTagNames;
    try
    {
        editableTagNames = this->importDatabaseGetEditableTagNames();
    }
    CATCH;

    /// i didn't like to import exported songs user need to remove from json
    /// not editable fields, so to keep all smooth not editable tags will be
    /// also readed to could be skipped in future parts
    QStringList notEditableTagNames;
    try
    {
        notEditableTagNames = this->importDatabaseGetNotEditableTagNames();
    }
    CATCH;

    DB << "initial data was loaded";

    QJsonArray jsonSongs = jsonMain["songs"].toArray();

    /// initial json songs validation
    for(const auto &jsonSongIt : jsonSongs)
    {
        QJsonObject jsonSong = jsonSongIt.toObject();

        /// NOTE: handling required fields can also be handled by comparing
        /// with list of required fields (in database exist column is_required)
        /// however there will be only one required field and this will be more
        /// readable in code below

        /// handle when json song not contains 'Song Path' tag
        if(!jsonSong.contains("Song Path"))
        {
            HANDLE_ERROR("one of the songs not contains required 'Song Path' tag!")
        }

        QString songPathValue = jsonSong["Song Path"].toString();

        /// handle when json song 'Song Path' tag is already in use
        if(usedSongPaths.contains(songPathValue))
        {
            HANDLE_ERROR(
                "one of the songs contains 'Song Path'='"
                    + songPathValue +
                    "' that already is (or will be) in use!")
        }

        /// add song path to list of used 'Song Path' tags (because it will be in use)
        usedSongPaths.append(songPathValue);

        /// check if json song contains only existing and editable tags
        for (auto tagIt = jsonSong.begin(); tagIt != jsonSong.end(); ++tagIt)
        {
            QString tagName = tagIt.key();
            if(editableTagNames.contains(tagName))
                continue;

            /// skip not editable, but existing in db tags
            if(notEditableTagNames.contains(tagName))
            {
                DB << ("skipped '" + tagName + "' tag").toStdString().c_str();
                continue;
            }

            HANDLE_ERROR(
                "one of the songs contains tag '"
                    + tagName +
                    "' that do not exist in database or is not editable!")
        }
    }

    DB << "initial validation was completed";

    /// at this point data are mostly valid

    /// addSong() was not implemented good enough (idk by who XD) and need to receive
    /// all existing editable tags...
    /// thats why following code iterates through all editable tags (editableTagNames)

    /// prepare list of structures that can be pass to addSong() method
    /// but firstly build map to translate tag names, given in json to tag id's

    QMap<QString, int> IDEquivalentForName;
    try
    {
        IDEquivalentForName = this->importDatabaseGetIDEquivalentForName();
    }
    CATCH;

    QList<QVariantList> listOfStructures;
    for(const auto &jsonSongIt : jsonSongs)
    {
        QJsonObject jsonSong = jsonSongIt.toObject();
        QVariantList structure;
        for(const auto &tagName : editableTagNames)
        {
            QString tagValue = "";

            /// replace tagValue if value exist in json
            if(jsonSong.contains(tagName))
            {
                if(jsonSong.value(tagName).isString())
                {
                    tagValue = jsonSong.value(tagName).toString();
                }
                else
                {
                    int tmpTagValue = jsonSong.value(tagName).toInt();
                    tagValue = QString::number(tmpTagValue);
                }

                DB << "equal: " << tagValue;
            }

            /// find tagID of tagName
            int tagID = IDEquivalentForName[tagName];

            structure.append(
                QVariantMap{ {"id", tagID}, {"value", tagValue} }
                );
        }
        listOfStructures.append(structure);
    }

    DB << "structure of songs was builded";

    /// connect addSong() method error with lambda
    QString addSongErrorInfo;
    auto addSongLambda = [&addSongErrorInfo](QString desc){
        // desc is a value received from signal
        WR << "error in add song: "<< desc;
        addSongErrorInfo = desc;
    };
    auto addSongErrorConnection =
        QObject::connect(this, &Database::signalAddSongError, addSongLambda);

    /// loading status variables
    qsizetype songsTotal = jsonSongs.size();
    qsizetype songsLoaded = 0;
    qsizetype loadSongsToRefresh = 0;
    // double refreshProgressPercentage = 0.01;
    int refreshAfterSongsLoaded = 1;

    /// add songs
    BEGIN_TRANSACTION
    {
        for(const auto &structure : listOfStructures)
        {
            /// btw, i can't imagine how event flow could look like with
            /// connection and calling following method and how this will allways
            /// handle the error...
            // DB << "stucture passed to addSong:" << structure;
            this->addSong(structure);

            if(!addSongErrorInfo.isNull())
            {
                m_database.rollback();
                WR << "structure that failed: " << structure;
                HANDLE_ERROR(
                    "adding song failed: " + addSongErrorInfo)
            }

            /// check if cancel loading
            if(m_cancelLoading)
            {
                DB << "loading canceled";
                m_cancelLoading = false;
                m_database.rollback();
                return; /// without signal (is not required, trust me)
            }

            /// send load status
            if(songsLoaded >= loadSongsToRefresh)
            {
                // loadSongsToRefresh += songsTotal * refreshProgressPercentage;
                loadSongsToRefresh += refreshAfterSongsLoaded;
                emit this->signalLoadingProgress({__func__, "Importing Songs", songsLoaded, songsTotal});
                QCoreApplication::processEvents();
            }
            ++songsLoaded;
        }
    }
    END_TRANSACTION(signalImportSongsToDatabaseError)

    QObject::disconnect(addSongErrorConnection);

    DB << "importing songs finished!";
    emit this->signalImportedSongsToDatabase();
#undef ERROR_SIGNAL
}

void Database::importTagsToDatabase(const QUrl &input_qurl)
{
#define ERROR_SIGNAL emit this->signalImportTagsToDatabaseError

    /*
     * imporing will fail if exist parametr(field) that not
     * exist in db (as a column), tags that cannot be set are
     * ignored (like 'id' or 'add_date')
     * in summary algorithm will read only fields ['name', 'type',
     * 'description'], where 'description is optional
     */
    emit this->signalLoadingStarted("Importing Tags");
    /// in this method loading status will not show progess because
    /// is enough fast, but freeze while loading could be usefull

    QJsonObject jsonMain;
    try
    {
        jsonMain = this->importDatabaseLoadJsonFromFile(input_qurl).object();
    }
    CATCH;

    /// test if json file contains tags aray
    if(!jsonMain.contains("tags"))
    {
        HANDLE_ERROR(
            "file '" + input_qurl.toLocalFile() +
            "' does not contains 'tags' array")
    }

    /// load all tag names that already in use
    QStringList usedTagNames;
    try
    {
        usedTagNames = this->importDatabaseGetUsedTagNames();
    }
    CATCH;

    DB << "initial data was loaded";

    QJsonArray jsonTags = jsonMain["tags"].toArray();
    const QList<QString> editableTagFields({
        "name",
        "type",
        "description"
    });
    const QList<QString> ignoredTagsFields({
        "id",
        "add_date",
        "update_date",
        "is_immutable",
        "is_editable",
        "is_required"
    });
    int tagType;

    /// initial json tags validation
    for(const auto &jsonTagIt : jsonTags)
    {
        QJsonObject jsonTag = jsonTagIt.toObject();

        /// handle when name not exist
        if(!jsonTag.contains("name"))
        {
            HANDLE_ERROR("one of the tags not contains required 'name' field!")
        }

        QString tagName = jsonTag["name"].toString();

        /// handle when json tag name is already in use
        if(usedTagNames.contains(tagName))
        {
            HANDLE_ERROR(
                "one of the tags contains 'name'='"
                + tagName +
                "' that already is (or will be) in use!")
        }

        /// add tag name to list of used tag names (because it will be in use)
        /// if something fail, content of this list will not matter
        usedTagNames.append(tagName);

        /// handle when type not exist
        if(!jsonTag.contains("type"))
        {
            HANDLE_ERROR("one of the tags not contains required 'type' field!")
        }

        /// handle value that is set in a tag type (use global tagType variable)
        tagType = -1;
        tagType = jsonTag["type"].toInt(-1);
        if(
            /// I want to make it clear what can be used:
            tagType != Database::TagType::TT_INTEGER &&
            tagType != Database::TagType::TT_TEXT &&
            tagType != Database::TagType::TT_TRISWITCH
            )
        {
            HANDLE_ERROR(
                "one of the tags contains 'type'='"
                + jsonTag["type"].toString() +
                "' that is not a valid type value (use 0, 1 or 2)!")
        }

        /// check if json tag contains only existing fields
        for (auto fieldIt = jsonTag.begin(); fieldIt != jsonTag.end(); ++fieldIt)
        {
            QString fieldName = fieldIt.key();

            if(editableTagFields.contains(fieldName))
                continue;

            if(ignoredTagsFields.contains(fieldName))
            {
                DB << ("skipped '" + fieldName + "' field").toStdString().c_str();
                continue;
            }

            HANDLE_ERROR(
                "one of the tags contains field '"
                + fieldName +
                "' that is not a parameter for a tag!")
        }
    }

    DB << "initial validation was completed";

    /// at this point data are mostly valid

    /// addTag was also not implemented well enough and requires all fields and
    /// all songs as argument

    /// load all songs IDs
    QStringList listOfSongsIDs;
    try
    {
        listOfSongsIDs = this->importDatabaseGetUsedSongIDs();
    }
    CATCH;

    /// build structure that can be pass to addTag method
    QList<QVariantList> listOfStructures;
    for(const auto &jsonTagIt : jsonTags) /// for each tag
    {
        QJsonObject jsonTag = jsonTagIt.toObject();
        QVariantList structure;

        /// set all editable fields of tag
        for(auto fieldName : editableTagFields) /// for each editable field
        {
            QVariant fieldValue;

            if(jsonTag.contains(fieldName))
            {
                if(fieldName == "type")
                    fieldValue = jsonTag.value(fieldName).toInt();
                else
                    fieldValue = jsonTag.value(fieldName).toString();
            }


            /// deam... addTag really sucks... that method requires 'Name', but not actual name 'name'...
            if (!fieldName.isEmpty()) {
                fieldName[0] = fieldName[0].toUpper();
            }

            structure.append(
                QVariantMap{ {"delegate_type", "param"}, {"name", fieldName}, {"value", fieldValue} }
                );
        }

        /// set all songs of tag
        for(const auto &songID : listOfSongsIDs)
        {
            /// set empty value, because adding tags allows for now, only to add tags
            /// but not to set value of songs for them
            /// empty value can be represented by nullptr

            structure.append(
                QVariantMap{ {"delegate_type", "song"}, {"id", songID}, {"value", QVariant::fromValue(nullptr)} }
                );
        }
        listOfStructures.append(structure);
    }

    DB << "structure of tags was builded";

    /// connect addTag() method error with lambda
    QString addTagErrorInfo;
    auto addTagLambda = [&addTagErrorInfo](QString desc){
        // desc is a value received from signal
        WR << "error in add tag: "<< desc;
        addTagErrorInfo = desc;
    };
    auto addTagErrorConnection =
        QObject::connect(this, &Database::signalAddTagError, addTagLambda);

    /// add songs
    BEGIN_TRANSACTION
    {
        for(const auto &structure : listOfStructures)
        {
            DB << structure;
            /// btw, i can't imagine how event flow could look like with
            /// connection and calling following method and how this will allways
            /// handle the error...
            this->addTag(structure);

            if(!addTagErrorInfo.isNull())
            {
                m_database.rollback();
                WR << "structure that failed: " << structure;
                HANDLE_ERROR(
                    "adding tag failed: " + addTagErrorInfo)
            }
        }
    }
    END_TRANSACTION(signalImportTagsToDatabaseError)

    QObject::disconnect(addTagErrorConnection);

    DB << "tags are imported correctly!";
    emit this->signalImportedTagsToDatabase();
#undef ERROR_SIGNAL
}

void Database::deleteDatabase()
{
    emit this->signalLoadingStarted("Deleting Database");
    /// in this method loading status will not show progess because
    /// is enough fast, but freeze while loading could be usefull

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

    /// query will return list of tags (each record == one tag)
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

void Database::loadFiltersModel()
{
    if(m_filters_model != nullptr){
        DB << "filters model was already loaded - skipped";
        emit this->signalFiltersModelLoaded();
        return;
    }
    /// model memory is cleared when something in database changed and after user save filters (updateFilters)

    /// use m_filters to create model
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
        WR << "error while loading song file"<<song_path<<": " << mp.errorString();
        emit this->signalAddSongError("error while loading song file'"+ song_path + "': " + mp.errorString());
        return;
    }

    // ------------------------------ set not editable fields -----------------------------

    // set title if needed
    /// test if first field is title (as it should be)
    /// // actually I don't check this, I trust myself... I hope
    QString song_title(mp.metaData().value(QMediaMetaData::Title).toString()); // will be used in setting icon
    if(lambda_get_value_by_id(2 /*Title field id*/).toString() == ""){
        if(song_title == ""){
            DB << "song file doesn't contains Title metadata, setting file name as the Title";
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

    // set author if needed
    /// test if first field is author (as it should be)
    if(lambda_get_value_by_id(3 /*Author field id*/).toString() == ""){
        DB << "available metadata: " << mp.metaData().keys();
        QString songAuthor(mp.metaData().value(QMediaMetaData::Author).toString());
        if(songAuthor == ""){
            DB << "song file doesn't contains Author metadata, leaving empty filed";
        }

        QString songAlbumArtist(mp.metaData().value(QMediaMetaData::AlbumArtist).toString());
        if(songAlbumArtist == ""){
            DB << "song file doesn't contains AlbumArtist metadata, leaving empty filed";
        }

        QString songContributingArtist(mp.metaData().value(QMediaMetaData::ContributingArtist).toString());
        if(songContributingArtist == ""){
            DB << "song file doesn't contains ContributingArtist metadata, leaving empty filed";
        }

        QString songComposer(mp.metaData().value(QMediaMetaData::Composer).toString());
        if(songComposer == ""){
            DB << "song file doesn't contains Composer metadata, leaving empty filed";
        }

        QString songPublisher(mp.metaData().value(QMediaMetaData::Publisher).toString());
        if(songPublisher == ""){
            DB << "song file doesn't contains Publisher metadata, leaving empty filed";
        }

        // if field is empty add "add field value" else add ";add field value"
        // or if "add field" is empty always add ""
        songAuthor += songAuthor == "" || songAlbumArtist == "" ? songAlbumArtist : ";" + songAlbumArtist;
        songAuthor += songAuthor == "" || songContributingArtist == "" ? songContributingArtist : ";" + songContributingArtist;
        songAuthor += songAuthor == "" || songComposer == "" ? songComposer : ";" + songComposer;
        songAuthor += songAuthor == "" || songPublisher == "" ? songPublisher : ";" + songPublisher;

        if(songAuthor != "")
        {
            DB << "setting own author to" << songAuthor;
            int index = lambda_get_map_index_by_id(3/*Author field id*/);
            if(index == -1) // Author not exist
            {
                WR << "error, Author field not found!";
                emit this->signalAddSongError("error, Author field not found!");
                return;
            }
            else // Author field found
            {
                auto map = new_song_data[index].toMap();
                map["value"] = songAuthor;
                // this way because new_song_data[0].toMap() returns constant map
                new_song_data[index] = map;
            }
        }
        else
        {
            DB << "none of the author fields contains data";
        }
    }

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
            WR << "error while loading song"<<song_path<<": " << mp.errorString();
            emit this->signalEditSongError("error while loading song '"+ song_path + "': " + mp.errorString());
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

    const char *author = "Cezary Androsiuk"; printf("%s", song_path.isEmpty() ? author : "");

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
    // DB << new_tag_data;

    // ------------------------------ test tag name if is unique -----------------------------
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
    this->makeCurrentFiltersValid();

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
    this->makeCurrentFiltersValid();

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
    this->makeCurrentFiltersValid();

    DB << "tag with id: " << tag_id << " deleted correctly";
    emit this->signalDeletedTag();
}


void Database::refreshPlaylist()
{
    // emit this->signalPlaylistRefreshError("");
    /// error will not be emited, cause there is no place for that ...

    emit this->signalPlaylistRefreshed(); /// this will trigger loadPlaylist
}

void Database::updateFilters(QVariantList filters)
{
    /*
        received structure:

        filters - QVariantList:
            id - int
            comparison_way - int
            comparison_value - QString

        NOTE: received structure contain all fields
    */

    if(m_filters != nullptr)
        delete m_filters;

    m_filters = new TagList(this);

    for(const auto &variant : filters)
    {
        TagWithComparator *twc = new TagWithComparator(m_filters);
        const QMap map = variant.toMap();

        twc->set_id(map["id"].toInt());
        twc->set_name(map["name"].toString());
        twc->set_type(map["type"].toInt());
        twc->set_comparison_way(map["comparison_way"].toInt());
        twc->set_comparison_value(map["comparison_value"].toString());

        // DB << twc->get_id() << "|" << twc->get_name() <<"|"<< twc->get_comparison_value();

        m_filters->tags().append(twc);
    }

    this->clearFiltersModelsMemory();
    this->debugPrint_filters();
    emit this->signalFiltersUpdated(); /// this will trigger loadPlaylist
}

void Database::loadPlaylist()
{
    IS_DATABASE_OPEN(signalPlaylistLoadError)

    QList<int> songsIDs = this->prepListOfSongsForPlaylist();

    SongDetailsList *songDetailsList = new SongDetailsList(this);
    for(const int &songID : songsIDs)
    {
        /// query will return list of tags (each record == one tag)
        QSqlQuery query(m_database);
        QString query_text(QString("SELECT tags.id, tags.name, tags.type, songs_tags.value "
                                   "FROM songs_tags "
                                   "JOIN tags ON songs_tags.tag_id = tags.id "
                                   "WHERE songs_tags.song_id = %1;").arg(songID));
        this->queryToFile(query_text);
        if(!query.exec(query_text)){
            WR << "error while executing query " << query.lastError();
            DB << "song with id:" << songID << "skipped";
        }

        SongDetails *songDetails = new SongDetails(songDetailsList);
        songDetails->set_id(songID);

        TagList *tagListForSongDetails = new TagList(songDetails);

        /// read only required minimum for playlist casue I will not add something like playlistSongDetails
        QSet<int> tagsToRead = {
            1,  /* ID */
            2,  /* Title */
            5,  /* Begin */
            6,  /* End */
            // 7,  /* Duration - check if realy used */
            9,  /* Song Path */
            10, /* Thumbnail Path */
        };

        while(query.next()){
            auto record = query.record();
            int id = record.value(0).toInt();
            if(!tagsToRead.contains(id))
                continue;

            auto name = record.value(1).toString();
            auto type = record.value(2).toInt();
            auto value = record.value(3).toString();

            Tag* tag = new Tag(tagListForSongDetails);
            tag->set_id(id);
            tag->set_name(name);
            tag->set_type(type);
            tag->set_value(value);

            tagListForSongDetails->tags().append(tag);
        }

        /// sort by tag id
        /// to keep everything nice and in order
        std::sort(
            tagListForSongDetails->tags().begin(),
            tagListForSongDetails->tags().end(),
            [](Tag *a, Tag *b){return a->get_id() < b->get_id();}
            );

        // DB << tagListForSongDetails->c_ref_tags();
        songDetails->set_tags(tagListForSongDetails);
        songDetailsList->songs().append(songDetails);
    }

    Database::debugPrintModel_Playlist(songDetailsList);
    /// we are trust each other and playlist class will delete old Playlist
    emit this->signalPlaylistLoaded(songDetailsList); /// this will trigger Playlist::loadPlaylist(TagList)
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

    clearFiltersModelsMemory();
}

void Database::clearFiltersModelsMemory()
{
    if(m_filters_model != nullptr)
        delete m_filters_model;
    m_filters_model = nullptr;
}

void Database::makeCurrentFiltersValid()
{
    TagList *oldFilters = m_filters; /// save for later cause fillFiltersWithValidTags create own instance
    m_filters = nullptr;
    QString errorCode = this->fillFiltersWithValidTags();
    if(!errorCode.isNull()){
        m_filters = oldFilters; /// bring back previous filters
        WR << errorCode;
        emit this->signalFiltersUpdateError(errorCode);
        return;
    }

    if(m_filters == nullptr) /// just for my own peace
    {
        m_filters = oldFilters; /// bring back previous filters
        WR << "fillFiltersWithValidTags screwed up with the one thing, it was created for. Good Job!";
        emit this->signalFiltersUpdateError(
            "fillFiltersWithValidTags screwed up with the one thing, it was created for. Good Job!");
        return;
    }

    /// check if method was started with m_filters being nullptr
    /// if so, there is no need to panic, that only means in our filled (with valid tags) m_filters
    ///    contains data that don't need to be changed
    if(oldFilters != nullptr)
    {
        /// compare valid filters with the old ones (old ones might contain comparison data)
        for(auto &filter : m_filters->c_ref_tags()) /// there is no warning cause fillFiltersWithValidTags create TagList
        {
            for(const auto &oldFilter : oldFilters->c_ref_tags())
            {
                if(filter->get_id() == oldFilter->get_id())
                {
                    TagWithComparator *twc_filter = static_cast<TagWithComparator *>(filter);
                    const TagWithComparator *twc_oldFilter = static_cast<const TagWithComparator *>(oldFilter);

                    twc_filter->set_comparison_way(twc_oldFilter->get_comparison_way());
                    twc_filter->set_comparison_value(twc_oldFilter->get_comparison_value());
                    break;
                }
            }
        }

        if(oldFilters != nullptr)
            delete oldFilters;   /// I totally forgot about it earlier :c
    }

    DB << "filters are now valid!";
    // this->debugPrint_filters();
}

QString Database::fillFiltersWithValidTags()
{
    QString query_text("SELECT id, name, type FROM tags;");
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

    DB << "filters filled with empty and valid tags";
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

QJsonDocument Database::importDatabaseLoadJsonFromFile(QUrl jsonFilePath)
{
    QString inputFile = jsonFilePath.toLocalFile();
    if(!QFile(inputFile).exists())
        THROW_EXCEPTION("file '" + inputFile + "' not found!");

    QFile jsonFile(inputFile);
    if(!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
        THROW_EXCEPTION("error while reading json from file '" + inputFile + "' with an error: " + jsonFile.errorString());

    QJsonParseError jsonError;
    QJsonDocument jsonData = QJsonDocument::fromJson(jsonFile.readAll(), &jsonError);
    jsonFile.close();

    if(jsonError.error != QJsonParseError::NoError)
        THROW_EXCEPTION("json parse error: " + jsonError.errorString());

    if(!jsonData.isObject())
        THROW_EXCEPTION("json file does not contains json object!");

    return jsonData;
}

QStringList Database::importDatabaseGetUsedSongPaths()
{
    QStringList usedSongPaths;

    /// Get used song paths
    QSqlQuery query(m_database);
    QString queryText("SELECT value FROM songs_tags WHERE tag_id = 9;"); /// tag_id = 9 is Song Path tag
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        usedSongPaths.append(query.value(0).toString());
    }

    return usedSongPaths;
}

QStringList Database::importDatabaseGetUsedTagNames()
{
    QStringList usedTagNames;

    /// Get used tag names
    QSqlQuery query(m_database);
    QString queryText("SELECT name FROM tags;");
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        usedTagNames.append(query.value(0).toString());
    }

    return usedTagNames;
}

QStringList Database::importDatabaseGetEditableTagNames()
{
    QStringList editableTagNames;

    /// Get editable tag names
    QSqlQuery query(m_database);
    QString queryText("SELECT name FROM tags WHERE is_editable = 1;");
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        editableTagNames.append(query.value(0).toString());
    }

    return editableTagNames;
}

QStringList Database::importDatabaseGetNotEditableTagNames()
{
    QStringList notEditableTagNames;

    /// Get not editable tag names
    QSqlQuery query(m_database);
    QString queryText("SELECT name FROM tags WHERE is_editable = 0;");
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        notEditableTagNames.append(query.value(0).toString());
    }

    return notEditableTagNames;
}

QStringList Database::importDatabaseGetUsedSongIDs()
{
    QStringList usedSongIDs;

    /// Get used song IDs
    QSqlQuery query(m_database);
    QString queryText("SELECT id FROM songs;");
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        usedSongIDs.append(query.value(0).toString());
    }

    return usedSongIDs;
}

QMap<QString, int> Database::importDatabaseGetIDEquivalentForName()
{
    QMap<QString, int> IDEquivalentForName;

    /// Get not editable tag names
    QSqlQuery query(m_database);
    QString queryText("SELECT id, name FROM tags;");
    this->queryToFile(queryText);
    if(!query.exec(queryText))
        THROW_EXCEPTION("error while executing '"+queryText+"' query:" + query.lastError().text());

    while(query.next()){
        IDEquivalentForName.insert(query.value(1).toString(), query.value(0).toInt());
    }

    return IDEquivalentForName;
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

void Database::debugPrintModel_Playlist(const SongDetailsList * const model)
{
#if PRINT_MODELS_LISTS
    DB << "PLAYLIST LIST:" << Database::_debugPrintModel_SongDetailsList(model).toStdString().c_str();
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

QString Database::_debugPrintModel_SongDetailsList(const SongDetailsList * const model)
{
    /* const pointer and const variable that he points to, thats why:
     *     const int *x - can't:  *x = 7; | can  x = &y;
     *     int const *x - can't:  *x = 7; | can  x = &y;
     *     int *const x - can:  *x = 7; | can't  x = &y; */
    QString obj_data("\n[");
    for(const auto &song : model->c_ref_songs()){
        obj_data += QString("\n   {"
                            "\n      song_id: '%1', "
                            "\n      tags: [")
                        .arg(song->get_id());
        for(const auto &t : song->get_tags()->c_ref_tags()){
            obj_data += QString("\n         {id: '%1', name: '%2', value: '%3', type: '%4', "
                                "is_immutable: '%5', is_editable: '%6', is_required: '%7'}")
                            .arg(t->get_id())
                            .arg(t->get_name(),
                                 t->get_value())
                            .arg(t->get_type())
                            .arg(t->get_is_immutable())
                            .arg(t->get_is_editable()) // bool is a nightmare
                            .arg(t->get_is_required());
            obj_data += (t == song->get_tags()->c_ref_tags().last() ? "" : ", ");
        }
        obj_data += "\n      ]"
                    "\n   }";
        obj_data += (song == model->c_ref_songs().last() ? "" : ", ");
    }
    return obj_data + "\n]";
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
                            "is_immutable: '%5', is_editable: '%6', is_required: '%7'")
                        .arg(t->get_id())
                        .arg(t->get_name(),
                             t->get_value())
                        .arg(t->get_type())
                        .arg(t->get_is_immutable())
                        .arg(t->get_is_editable()) // bool is a nightmare
                        .arg(t->get_is_required());
        if (TagWithComparator* twc = dynamic_cast<TagWithComparator*>(t)) {
            obj_data += QString(", comparsion_way: '%1', comparsion_value: '%2'")
                            .arg(twc->get_comparison_way())
                            .arg(twc->get_comparison_value());
        }
        obj_data += "}";
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


QList<int> Database::prepListOfSongsForPlaylist() const
{
    /// build list of constraints
    QStringList constraints;
    for(const auto &t : m_filters->c_ref_tags())
    {
        const TagWithComparator *twc = static_cast<const TagWithComparator *>(t);
        QString constraint;

        int tagType = twc->get_type();
        switch(tagType)
        {
        case Database::TagType::TT_INTEGER:
            constraint = Database::prepIntegerConstraint(twc);
            break;
        case Database::TagType::TT_TEXT:
            constraint = Database::prepTextConstraint(twc);
            break;
        case Database::TagType::TT_TRISWITCH:
            constraint = Database::prepBoolConstraint(twc);
            break;
        default:
            WR << "unknown type of the tag:" << tagType << "skipping adding this constraint";
        }
        // DB <<"constraint" << constraint;
        if(!constraint.isNull())
            constraints.append(constraint);
    }
    // DB << "constraints" << constraints;

    /// now in constrainst variable are all restrictions that matters, but playlist requires
    /// list of songs (theirs ID) that are belong to that constraints


    /// change constraints to songs list (list of ID's)
    /// let's assume that no error will occur :)
    /// if occur constraint will be skipped
    QList<QList<int>> listOfListsOfIDs;
    for(const auto &constraint : constraints)
    {
        QString query_text(QString("SELECT song_id FROM songs_tags WHERE %1;").arg(constraint));

        this->queryToFile(query_text);
        QSqlQuery query(m_database);
        if(!query.exec(query_text)){
            WR << "executing select query " << query.lastError();
            continue;
        }

        QList<int> listOfIDs;
        while(query.next()){
            listOfIDs.append(query.record().value(0).toInt());
        }
        // DB << listOfIDs;
        listOfListsOfIDs.append(listOfIDs);
    }

    /// add list that contains all the songs if there is no constraints in our output list
    /// because in that case all songs will be in playlist
    if(listOfListsOfIDs.empty())
    {
        QString query_text("SELECT id FROM songs;");
        this->queryToFile(query_text);
        QSqlQuery query(m_database);
        if(!query.exec(query_text)){
            WR << "executing select query error" << query.lastError() << "can't load all songs";
        }
        else
        {
            QList<int> listOfIDs;
            while(query.next()){
                listOfIDs.append(query.record().value(0).toInt());
            }
            listOfListsOfIDs.append(listOfIDs);
        }
    }

    /// get songs (theirs ID) that belong to all lists
    QList<int> uniqueIDs = Database::margeCommonItemsToOneList(listOfListsOfIDs);
    // DB << "uniqueIDs: " << uniqueIDs;

    DB << "list of songs for playlist prepared";
    return uniqueIDs;
}

QList<int> Database::margeCommonItemsToOneList(QList<QList<int> > list)
{
    QList<int> uniqueIDs;
    // jeżeli i-ty element z listy A zawiera się w listach B,...,Z to dodaj do listy uniqueIDs
    for (const auto &innerList : list) {
        for (const auto &value : innerList) {
            if(uniqueIDs.contains(value)) /// skip value that was already checked
                continue;

            if(Database::checkIfAllListsContainsValue(list, value))
                uniqueIDs.append(value);
        }
    }
    return uniqueIDs;
}

bool Database::checkIfAllListsContainsValue(QList<QList<int> > list, int value)
{
    for (const auto &innerList : list) {
        if(!innerList.contains(value))
            return false; /// found list that not contains value
    }
    return true;
}

QString Database::prepIntegerConstraint(const TagWithComparator *twc)
{
    QString constraint( QString("tag_id = %1 AND ").arg(twc->get_id()) );
    int comparsionWay = twc->get_comparison_way();
    int comparsionValue = twc->get_comparison_value().toInt();
    switch(comparsionWay)
    {
    case TagWithComparator::IntegerCompare::DO_NOT_COMPARE:
        return QString();
    case TagWithComparator::IntegerCompare::IS_EQUAL_TO:
        return constraint + QString("CAST(value AS INTEGER) = %1").arg(comparsionValue);
        break;
    case TagWithComparator::IntegerCompare::IS_DIFFERENT_THAN:
        return constraint + QString("CAST(value AS INTEGER) != %1").arg(comparsionValue);
        break;
    case TagWithComparator::IntegerCompare::IS_LESS_OR_EQUAL_TO:
        return constraint + QString("CAST(value AS INTEGER) <= %1").arg(comparsionValue);
        break;
    case TagWithComparator::IntegerCompare::IS_LESS_THAN:
        return constraint + QString("CAST(value AS INTEGER) < %1").arg(comparsionValue);
        break;
    case TagWithComparator::IntegerCompare::IS_GREATER_OR_EQUAL_TO:
        return constraint + QString("CAST(value AS INTEGER) >= %1").arg(comparsionValue);
        break;
    case TagWithComparator::IntegerCompare::IS_GREATER_THAN:
        return constraint + QString("CAST(value AS INTEGER) > %1").arg(comparsionValue);
        break;
    default:
        WR << "unknown comparsion way of the tag:" << comparsionWay
           << "skipping adding this constraint with " << comparsionValue
           << "value";
        return QString();
    }
}

QString Database::prepTextConstraint(const TagWithComparator *twc)
{
    QString constraint( QString("tag_id = %1 AND ").arg(twc->get_id()) );
    int comparsionWay = twc->get_comparison_way();
    QString comparsionValue = twc->get_comparison_value();
    switch(comparsionWay)
    {
    case TagWithComparator::TextCompare::DO_NOT_COMPARE:
        return QString();
    case TagWithComparator::TextCompare::IS_EQUAL_TO:
        return constraint + QString("value = '%1'").arg(comparsionValue);
        break;
    case TagWithComparator::TextCompare::IS_DIFFERENT_THAN:
        return constraint + QString("value != '%1'").arg(comparsionValue);
        break;
    case TagWithComparator::TextCompare::IS_APPROXIMATELY_EQUAL_TO:
        return constraint + QString("LOWER(value) = LOWER('%1')").arg(comparsionValue);
        break;
    case TagWithComparator::TextCompare::IS_APPROXIMATELY_DIFFERENT_THAN:
        return constraint + QString("LOWER(value) != LOWER('%1')").arg(comparsionValue);
        break;
    case TagWithComparator::TextCompare::IS_LIKE:
        return constraint + QString("value LIKE '%1'").arg(comparsionValue);
        break;
    case TagWithComparator::TextCompare::IS_EQUAL_REGEX:
        return constraint + QString("value REGEXP '%1'").arg(comparsionValue);
        break;
    default:
        WR << "unknown comparsion way of the tag:" << comparsionWay
           << "skipping adding this constraint with " << comparsionValue.toStdString().c_str()
           << "value";
        return QString();
    }
}

QString Database::prepBoolConstraint(const TagWithComparator *twc)
{
    int comparsionValue = twc->get_comparison_value().toInt();
    switch(comparsionValue + 1) /// +1 is cause in triswich do not compare is related wit -1 and compare is 1
    {
    case TagWithComparator::BoolCompare::NOT_BELONG_TO_TAG:
        return QString("tag_id = %1 AND ").arg(twc->get_id()) +
               QString("CAST(value AS INTEGER) = %1").arg(/*twc->get_comparison_value()*/-1);
    case TagWithComparator::BoolCompare::BELONG_TO_TAG:
        return QString("tag_id = %1 AND ").arg(twc->get_id()) +
               QString("CAST(value AS INTEGER) = %1").arg(/*twc->get_comparison_value()*/1);

    case TagWithComparator::BoolCompare::DO_NOT_COMPARE:
        return QString();
    default:
        WR << "unknown comparsion value of the tag:" << comparsionValue
           << "skipping adding this constraint with " << twc->get_comparison_value()
           << "value";
        return QString();
    }
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

TagList *Database::get_filters_model() const
{
    return m_filters_model;
}
