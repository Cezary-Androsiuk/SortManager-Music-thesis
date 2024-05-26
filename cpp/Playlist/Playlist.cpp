#include "Playlist.h"

Playlist::Playlist(QObject *parent)
    : QObject{parent}
{}
/*
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
//*/
void Playlist::loadNewPlaylistList(SongDetailsList *list)
{
    DB << "in PLAYLIST";
}
