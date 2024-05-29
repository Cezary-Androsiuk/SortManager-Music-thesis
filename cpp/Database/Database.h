#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql> // to make it work add "Sql"to find_package() and "Qt6::Sql" to target_link_libraries()
#include <QSqlQuery>
#include <QSqlField>
#include <QFile>
#include <QFileInfo>
#include <QVariant>
#include <QTextStream>
#include <QDateTime>
#include <QColor>
#include <QtMultimedia/QMediaPlayer> // load song duration
#include <QtMultimedia/QMediaMetaData>
#include <QEventLoop> // wait for the event
#include <QMap>
#include <QSet>
#include <QHash>
// #include <algorithm>

#include <QRegularExpression>

#include "cpp/DebugPrint/DebugPrint.h"

#include "cpp/Song/Song.h"
#include "cpp/Song/SongDetails.h"
#include "cpp/Song/SongDetailsList.h"
#include "cpp/Song/SongList.h"
#include "cpp/Tag/Tag.h"
#include "cpp/Tag/TagWithComparator.h"
#include "cpp/Tag/TagDetails.h"
#include "cpp/Tag/TagList.h"
#include "cpp/Filter/Filter.h"

#define DATABASE_PATH QString("./SortManager-Music.db")     /// database directory
#define DB_TAG_NAME_REG_EX "^[a-z][_a-z0-9]*$"              /// regular expression describes what is naming convension of tag names in database
#define QUERY_LOG_PATH "./query.log"                        /// query log directory - stores all executed queries
#define QUERY_ARCHIVE_LOG_PATH "./query_prev.log"           /// previous query log directory (as archive) - stores all executed queries
#define PRINT_MODELS_LISTS true                             /// if true -> model will be printed to console, before will be sended to QML after load

// ------------------------- shortcuts that make code more readable and consistent ------------------------- //
/// is database open
#define IS_DATABASE_OPEN(errorSignal) if(!Database::isDatabaseOpen(&Database::errorSignal, __PRETTY_FUNCTION__)) return;
/// transactions start
#define BEGIN_TRANSACTION bool is_transaction_started_here = false; if(m_database.transaction()){ is_transaction_started_here = true; }
/// transactions end
#define END_TRANSACTION(errorSignal) if(is_transaction_started_here){ if(!Database::endTransaction(&Database::errorSignal, __PRETTY_FUNCTION__)) return; }
/// throw an exception
#define THROW_EXCEPTION(desc) throw std::runtime_error((QString() + desc).toStdString().c_str())
/// handle error and return // requires emit keyword before signal
#define HANDLE_ERROR(desc, errorSignal) WR << desc; errorSignal(desc); return;
/// catch common error thrown by called function
#define CATH(errorSignal) catch(std::runtime_error &e) {WR << e.what(); errorSignal(e.what()); return;}


class Database : public QObject
{
    Q_OBJECT

    // frontend songs accessors
    Q_PROPERTY(SongList* all_songs_model        READ get_all_songs_model    CONSTANT)
    Q_PROPERTY(SongDetails* edit_song_model     READ get_edit_song_model    CONSTANT)
    Q_PROPERTY(SongDetails* add_song_model      READ get_add_song_model     CONSTANT)

    // frontend tags accessors
    Q_PROPERTY(TagList* all_tags_model          READ get_all_tags_model     CONSTANT)
    Q_PROPERTY(TagDetails* edit_tag_model       READ get_edit_tag_model     CONSTANT)
    Q_PROPERTY(TagDetails* add_tag_model        READ get_add_tag_model      CONSTANT)

    // frontend playlist accessors
    Q_PROPERTY(TagList* filters_model           READ get_filters_model      CONSTANT)

public:
    struct TagType{
        enum{
            TT_INTEGER,
            TT_TEXT,
            TT_BOOL
        };
    };

    explicit Database(QObject *parent = nullptr);
    ~Database();

    // setters for personalizations
    void setSaveExecQuery(bool saveExecQuery);
    void setShowConstantTags(bool showConstantTags);
    // void set_songs_main_path(const QString &path);


    /// errors should have "QString desc" because they will also send the message about error
signals: // -------------------------------------------------- database init -------------------------------------------------- //
    /// finished
    void signalInitializedOnStart();    /// emited after database was initialized successfully
    void signalInitializedWithTags();   /// emited after database was builded successfully
    void signalFiltersInitailized();
    /// error
    void signalInitializeOnStartFailed(QString desc);   /// emited when any error occur while initializing database on start (mostly database open error)
    void signalInitializeWithTagsFailed(QString desc);  /// emited when any error occur while initializing database with tags
    void signalFiltersInitailizeFailed(QString desc);

signals: // -------------------------------------------------- db management -------------------------------------------------- //
    /// finished
    void signalExportedSongsFromDatabase();     /// emited when data was correctly exported from the database to json
    void signalExportedTagsFromDatabase();      /// emited when data was correctly exported from the database to json
    void signalImportedSongsToDatabase();       /// emited when data was correctly imported Songs to the database
    // void signalImportedDatabase();    //TO DELETE emited when data was correctly imported to the database
    void signalImportedTagsToDatabase();        /// emited when data was correctly imported Tags to the database
    void signalDeletedDatabase();               /// emited when data was correctly deleted from the database
    /// error
    void signalExportSongsFromDatabaseError(QString desc);           /// emited when an error occur while exporting data from the database to json
    void signalExportTagsFromDatabaseError(QString desc);           /// emited when an error occur while exporting data from the database to json
    void signalImportSongsToDatabaseError(QString desc);    /// emited when an error occur while importing Songs data to the database
    void signalImportTagsToDatabaseError(QString desc);     /// emited when an error occur while importing Tags data to the database
    // void signalImportDatabaseError(QString desc);     //TO DELETE emited when an error occur while importing data to the database
    void signalDeleteDatabaseError(QString desc);           /// emited when an error occur while deleting data from the database

signals: // -------------------------------------------------- load models -------------------------------------------------- //
    // songs
    /// finished
    void signalAllSongsModelLoaded();       /// emited when all songs model was loaded correctly
    void signalAddSongModelLoaded();        /// emited when add song model was loaded correctly
    void signalEditSongModelLoaded();       /// emited when edit song model was loaded correctly
    /// error
    void signalAllSongsModelLoadError(QString desc);    /// emited when any error occur while loading all songs model
    void signalAddSongModelLoadError(QString desc);     /// emited when any error occur while loading add song model
    void signalEditSongModelLoadError(QString desc);    /// emited when any error occur while loading edit song model

    // tags
    /// finished
    void signalAllTagsModelLoaded();        /// emited when all tags model was loaded correctly
    void signalAddTagModelLoaded();         /// emited when add tag model was loaded correctly
    void signalEditTagModelLoaded();        /// emited when edit tag model was loaded correctly
    /// error
    void signalAllTagsModelLoadError(QString desc);     /// emited when any error occur while loading all tags model
    void signalAddTagModelLoadError(QString desc);      /// emited when any error occur while loading add tag model
    void signalEditTagModelLoadError(QString desc);     /// emited when any error occur while loading edit tag model

    // playlist
    /// finished
    void signalPlaylistModelLoaded();           /// emited when playlist model was loaded correctly                         // SongList* listofsongs
    void signalEditPlaylistSongModelLoaded();   /// emited when edit playlist song model was loaded correctly
    void signalFiltersModelLoaded();            /// emited when filters model was loaded correctly to playlist
    /// error occur
    void signalPlaylistModelLoadError(QString desc);            /// emited when any error occur while loading playlist model
    void signalEditPlaylistSongModelLoadError(QString desc);    /// emited when any error occur while loading edit playlist song model
    void signalFiltersModelLoadError(QString desc);             /// emited when any error occur while loading filters model

signals: // -------------------------------------------------- database actions -------------------------------------------------- //
    /// finished
    void signalAddedSong();     /// emited when song was added correctly
    void signalEditedSong();    /// emited when song was edited correctly
    void signalDeletedSong();   /// emited when song was deleted correctly
    void signalAddedTag();      /// emited when tag was added correctly
    void signalEditedTag();     /// emited when tag was edited correctly
    void signalDeletedTag();    /// emited when tag was deleted correctly
    /// error occur
    void signalAddSongError(QString desc);      /// emited when any error occur while adding song
    void signalEditSongError(QString desc);     /// emited when any error occur while editing song
    void signalDeleteSongError(QString desc);   /// emited when any error occur while deleting song
    void signalAddTagError(QString desc);       /// emited when any error occur while adding tag
    void signalEditTagError(QString desc);      /// emited when any error occur while editing tag
    void signalDeleteTagError(QString desc);    /// emited when any error occur while deleting tag

signals: // -------------------------------------------------- playlist actions -------------------------------------------------- //
    /// finished
    void signalPlaylistRefreshed(); /// emited by refreshPlaylist and triggers loadPlaylistList
    void signalFiltersUpdated();    /// emited by updateFilters and triggers loadPlaylistList
    /// error occur
    void signalPlaylistRefreshError(QString desc);  /// emited by refreshPlaylist
    void signalFiltersUpdateError(QString desc);    /// emited by updateFilters
    void signalPlaylistListLoadError(QString desc); /// is oposite to signalPlaylistListLoaded, and emtited when loadPlaylistList failed

signals: // signals for Playlist
    void signalPlaylistListLoaded(SongDetailsList *list);    /// emited by loadPlaylistList with list of Tags for Playlist class (list is limited by constraints that are readed from m_filters)

public slots: // database init
    void initializeOnStart();       /// starting aapplication
    void initializeWithTags();      /// building new database
    void createExampleData();
    void initializeFilters();       /// initialize filters (to avoid m_filters being empty)

public slots: // db management
    void exportSongsFromDatabase(const QUrl &output_qurl);
    void exportTagsFromDatabase(const QUrl &output_qurl);
    void importSongsToDatabase(const QUrl &input_qurl);
    void importTagsToDatabase(const QUrl &input_qurl);
    void importDatabase(const QUrl &input_qurl); // TO DELETE
    void deleteDatabase();

public slots: // load models
    // songs
    void loadAllSongs();
    void loadAddSongModel();
    void loadEditSongModel(int song_id);            /// argument is used to tell what song user want to edit
    // tags
    void loadAllTags();
    void loadAddTagModel();
    void loadEditTagModel(int tag_id);              /// argument is used to tell what tag user want to edit
    // playlist
    void loadFiltersModel();

public slots: // database actions
    // songs
    void addSong(QVariantList new_song_data);
    void editSong(int song_id, QVariantList song_data);
    void deleteSong(int song_id);
    // tags
    void addTag(QVariantList new_tag_data);
    void editTag(int tag_id, QVariantList tag_data);
    void deleteTag(int tag_id);

public slots: // playlist actions
    void refreshPlaylist();                     /// is triggered by QML when user press refresh button
    void updateFilters(QVariantList filters);   /// is triggered by QML when user save changes in filters page
    void loadPlaylistList();                    /// is triggered by signalFiltersInitialized, signalPlaylistRefreshed and signalFiltersUpdated

private: // other methods to support
    void clearModelsMemory();           /// clears models from memory, cause something was changed and their need to be loaded again
    void clearFiltersModelsMemory();    /// is a sepatate method cause when user save playlist (trigger updateFilters) there is no need to clear all models (just a filters)
    void makeCurrentFiltersValid();     /// add or remove filters in m_filters to contain only tags that are in db /// is called after clearModelsMemory (cause that means something changed) or by initializeFilters
    QString fillFiltersWithValidTags(); /// used makeCurrentFiltersValid to can compare valid tags with the current ones, returns QString with error (if no error QString will be null)

    static QString notNull(const QString &value);
    bool isDatabaseOpen(void (Database::*signal)(QString), const char *caller_name = "");
    bool beginTransaction(void (Database::*signal)(QString), const char *caller_name = "");
    bool endTransaction(void (Database::*signal)(QString), const char *caller_name = "");

    /// support import database
    QJsonDocument importDatabaseLoadJsonFromFile(QUrl jsonFilePath);
    QStringList importDatabaseGetUsedSongPaths();
    QStringList importDatabaseGetAvaliableTagNames();
    int importDatabaseChangeTagNameToTagID(QString tagName) const;

    /// print methods
    void debugPrint_filters() const;
    void debugPrintModel_all_songs() const;
    void debugPrintModel_add_song() const;
    void debugPrintModel_edit_song() const;
    static void debugPrintModel_playlistList(const SongDetailsList* const model); /// Just because You are unique doesn't mean You are usefull
    void debugPrintModel_all_tags() const;
    void debugPrintModel_add_tag() const;
    void debugPrintModel_edit_tag() const;
    void debugPrintModel_filters() const;

    /// print support methods
    static QString _debugPrintModel_SongList(const SongList* const model);
    static QString _debugPrintModel_SongDetails(const SongDetails* const model);
    static QString _debugPrintModel_SongDetailsList(const SongDetailsList* const model);
    static QString _debugPrintModel_TagList(const TagList* const model);
    static QString _debugPrintModel_TagDetails(const TagDetails* const model);


    void queryToFile(QString query, QStringList param_names = {}, QVariantList param_values = {}) const;


    QList<int> prepListOfSongsForPlaylist() const;                                  /// creates query based on the m_filters
    static QList<int> margeCommonItemsToOneList(QList<QList<int>> list);            /// extension of prepPlaylistSongsQuery - self-descriptive
    static bool checkIfAllListsContainsValue(QList<QList<int>> list, int value);    /// extension of margeCommonItemsToOneList - self-descriptive
    static QString prepIntegerConstraint(const TagWithComparator *twc);             /// extension of prepPlaylistSongsQuery - prepare constraint based on the comparator values (comparison_way and comparison_value)
    static QString prepTextConstraint(const TagWithComparator *twc);                /// extension of prepPlaylistSongsQuery - prepare constraint based on the comparator values (comparison_way and comparison_value)
    static QString prepBoolConstraint(const TagWithComparator *twc);                /// extension of prepPlaylistSongsQuery - prepare constraint based on the comparator values (comparison_way and comparison_value)

public:
    // model accessors
    /// songs
    SongList *get_all_songs_model() const;
    SongDetails *get_edit_song_model() const;
    SongDetails *get_add_song_model() const;
    /// tags
    TagList *get_all_tags_model() const;
    TagDetails *get_edit_tag_model() const;
    TagDetails *get_add_tag_model() const;
    /// playlist
    TagList *get_filters_model() const;

private:
    bool m_databaseInitialized;
    bool m_saveExecQuery;
    bool m_showConstantTags;

    // database handler
    QSqlDatabase m_database;

    TagList *m_filters;

    // Models
    /// songs
    SongList* m_all_songs_model;
    SongDetails* m_add_song_model;
    SongDetails* m_edit_song_model;
    /// tags
    TagList *m_all_tags_model;
    TagDetails* m_add_tag_model;
    TagDetails* m_edit_tag_model;
    /// playlist
    TagList *m_filters_model;
};

/*
class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool databaseOpen() const;
    ///
    /// \brief isDatabaseConsistent     - test if constant tags from the database are equal to constant tags readed
    ///                                     from json file (TagsConstant::loadTagsInfo)
    ///                                     Method reacts only on order of tags, adding new constant tag,
    ///                                     removing one or changing db_name/db_type
    /// \return                         - true if database is consistent, false if not
    ///
    bool isDatabaseConsistent() const;
    typedef QList<QString> Qls;

signals:
    void allSongsChanged();

private:
    QSqlDatabase m_database;
    QList<Song*> m_all_songs;
    QList<Song*> m_playlist_songs;

public:
    void initDatabase(int* error_code = nullptr);

    static bool testTagNameValidation(const QString &tag_name);

    ///
    /// \brief preparePlaylistSongsQuery    - Method used by playlist selection after specyfied filters
    ///                                         prepare statement for playlist. Do not need to pass all tags because it will be
    ///                                         hard to handle something like this [title LIKE ''] or like this [start < ]
    ///                                         and easier is not to mention parameters that i don't know than mentions alot more
    /// \param tc_names                     - (generated by QML) names of (setted to filter) constant tags
    /// \param tc_values                    - (given by user) values of (setted to filter) constant tags
    /// \param tc_comparators               - (generated by QML) choosed by user from combobox type of comparsion given
    ///                                         value (example [NOT LIKE / LIKE] or [=/</>/!=])
    /// \param te_names                     - (generated by QML) names of (setted to filter) editable tags
    /// \param te_values                    - (generated by QML) choosed by user by three state switch
    ///                                         that tag or not (tags to not carry about are not included)
    /// \param error_code                   - if is 0, then all good
    /// \return                             - statement to execute
    ///
    QSqlQuery preparePlaylistSongsQuery(const Qls &tc_names, const Qls &tc_values, const Qls &tc_comparators,
                                        const Qls &te_names, const QList<bool> &te_values,
                                        int* error_code = nullptr) const;
    ///
    /// \brief loadPlaylistSongs            - Method used by playlist selection after specyfied filters
    ///                                         reads songs from database and add them to m_playlist_songs
    /// \param tc_names                     - (generated by QML) names of (setted to filter) constant tags
    /// \param tc_values                    - (given by user) values of (setted to filter) constant tags
    /// \param tc_comparators               - (generated by QML) choosed by user from combobox type of comparsion given
    ///                                         value (example [NOT LIKE / LIKE] or [=/</>/!=])
    /// \param te_names                     - (generated by QML) names of (setted to filter) editable tags
    /// \param te_values                    - (generated by QML) choosed by user by three state switch
    /// \param error_code                   - if is 0, then all good
    ///
    void loadPlaylistSongs(const Qls &tc_names, const Qls &tc_comparators, const Qls &tc_values,
                           const Qls &te_names, const QList<bool> &te_values,
                           int* error_code = nullptr);
    void loadAllSongs(int* error_code = nullptr);

    bool save_query_to_file() const;
    void setSave_query_to_file(bool newSave_query_to_file);

    QList<Song *> all_songs() const;
    QList<Song *> playlist_songs() const;

    void loadSongDetails(int id);

public slots:
    void addTagEditableDatabase(QString tag_name, QList<int> songs_id, QList<int> songs_values, int *error_code = nullptr);
    ///
    /// \brief delTagEditableDatabase       - executed by pressing X button, next to tag, that user want to delete
    /// \param tag_name                     - (generated by QML) table name
    /// \param error_code
    ///
    void delTagEditableDatabase(QString tag_name, int* error_code = nullptr);

    ///
    /// \brief addSongDatabase          - add song to database, required m_all_songs reload after, passing all constant tags is
    ///                                     important, because adding we can specify all values (is hard to check which
    ///                                     one has a default value setted) so i can add (by QML) values that i want to
    ///                                     initialize song, but 4 example editable tags all have null
    /// \param tc_values                - (given by user)
    /// \param te_names                 - (generated by QML)
    /// \param te_values                - (generated by QML)
    /// \param error_code               - if is 0, then all good
    ///
    void addSongDatabase(const Qls &tc_names, const Qls &tc_values,
                         const Qls &te_names, const Qls &te_values,
                         int* error_code = nullptr);
    void updateSongDatabase(const Qls &tc_names, const Qls &tc_values,
                            const Qls &te_names, const Qls &te_values,
                            int* error_code = nullptr);
private:
    bool m_save_query_to_file;
};
*/

/*
drop table Customers;
drop table Orders;
drop table Shippings;


--drop table songs;
CREATE TABLE songs(
   id            INTEGER     PRIMARY KEY AUTOINCREMENT
);
--drop table tags;
CREATE TABLE tags(
   id            INTEGER     PRIMARY KEY AUTOINCREMENT,
   name          TEXT        NOT NULL UNIQUE,
   description   TEXT        NOT NULL,
   add_date      INTEGER     NOT NULL,
   update_date   INTEGER     NOT NULL,
   type          INTEGER     NOT NULL,
   is_immutable  INTEGER     NOT NULL,
   is_editable   INTEGER     NOT NULL,
   is_required   INTEGER     NOT NULL
);
--drop table songs_tags;
CREATE TABLE songs_tags(
   id           INTEGER     PRIMARY KEY AUTOINCREMENT,
   song_id      INTEGER     NOT NULL,
   tag_id       INTEGER     NOT NULL,
   value        TEXT        NOT NULL,

   FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE,
   FOREIGN KEY (tag_id)  REFERENCES tags(id)  ON DELETE CASCADE
);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('ID', '', 1711679891, 1711679891, 0, 1, 0, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Title', '', 1711679891, 1711679891, 1, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Author', '', 1711679891, 1711679891, 1, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Description', '', 1711679891, 1711679891, 1, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Begin (ms)', '', 1711679891, 1711679891, 0, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('End (ms)', '', 1711679891, 1711679891, 0, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Duration (ms)', '', 1711679891, 1711679891, 0, 1, 0, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Lyrics', '', 1711679891, 1711679891, 1, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Song Path', '', 1711679891, 1711679891, 1, 1, 1, 1);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Thumbnail Path', '', 1711679891, 1711679891, 1, 1, 1, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Add Date', '', 1711679891, 1711679891, 0, 1, 0, 0);
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required) VALUES ('Update Date', '', 1711679891, 1711679891, 0, 1, 0, 0);


-- adding song 1
INSERT INTO songs DEFAULT VALUES;
-- lista wraz z ilością powinna być dostarczona przez qml
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 2, "example song title");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 3, "song author");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 4, "long description");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 5, 10);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 6, 999);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 7, 1010);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 8, "tekst piosenki");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 9, "path//");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 10, "path//");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 11, 1711679000);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (1, 12, 1711679000);
--

-- adding song 2
INSERT INTO songs DEFAULT VALUES;
-- lista wraz z ilością powinna być dostarczona przez qml
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 2, "new song");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 3, "new author");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 4, "long description");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 5, 10);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 6, 999);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 7, 1010);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 8, "tekst piosenki");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 9, "path//");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 10, "path//");
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 11, 1711679000);
INSERT INTO songs_tags (song_id, tag_id, value) VALUES (2, 12, 1711679000);
--

-- adding tag 1
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required)
VALUES ('own tag', '', 1711679891, 1711679891, 2, 0, 1, 0);
INSERT INTO songs_tags (song_id, tag_id)
SELECT songs.id AS song_id, 13 AS tag_id -- tag id is 13
FROM songs;
-- for in songs, i dla każdej ustawiamy wartość domyślną zależnie od type lub tego co wybrał użytkownik
UPDATE songs_tags
SET value = '0' -- default for state
WHERE song_id = 1 AND tag_id = 13;

UPDATE songs_tags
SET value = '1'
WHERE song_id = 2 AND tag_id = 13;
--

-- adding tag 2
INSERT INTO tags (name, description, add_date, update_date, type, is_immutable, is_editable, is_required)
VALUES ('own more flexible tag', '', 1711679891, 1711679891, 1, 0, 1, 0);
INSERT INTO songs_tags (song_id, tag_id)
SELECT songs.id AS song_id, 14 AS tag_id -- tag id is 14
FROM songs;
-- for in songs, i dla każdej ustawiamy wartość domyślną zależnie od type lub tego co wybrał użytkownik
UPDATE songs_tags
SET value = '' -- default for text
WHERE song_id = 1 AND tag_id = 14;

UPDATE songs_tags
SET value = 'some text that user input while adding tag'
WHERE song_id = 2 AND tag_id = 14;
--


*/

#endif // DATABASE_H
