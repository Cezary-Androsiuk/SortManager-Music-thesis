#include "Playlist.h"

Playlist::Playlist(QObject *parent)
    : QObject{parent},
    m_playlistModel(nullptr),
    m_playlist(nullptr)
{
    /// after new playlist was loaded, shuffle it
    QObject::connect(this, &Playlist::playlistLoaded, this, &Playlist::shufflePlaylist);

    /// load playlist model after playlist was shuffled (but not after playlistLoad, because
    /// playlistLoad triggers shufflePlaylist)
    // QObject::connect(this, &Playlist::playlistLoaded, this, &Playlist::loadPlaylistModel);
    QObject::connect(this, &Playlist::playlistShuffled, this, &Playlist::loadPlaylistModel);
}

void Playlist::loadPlaylistModel()
{
    DB << " - staring playlist load";
    /// clear models memory because loadPlaylistModel is called only when
    /// something changed
    if(m_playlistModel != nullptr)
        delete m_playlistModel;
    m_playlistModel = nullptr;


    m_playlistModel = new SongList(this);

    for(const SongDetails *songDetails : m_playlist->c_ref_songs())
    {
        DB << " - song";
        Song *song = new Song(m_playlistModel);
        song->set_id(songDetails->get_id());

        /// get title form songDetails
        QString title = "no title set";
        for(const Tag *tag : songDetails->get_tags()->c_ref_tags())
        {
            if(tag->get_name() == "Title")
                title = tag->get_value();
        }
        song->set_title(title);
        // value is not needed

        m_playlistModel->songs().append(song);
    }

    DB << "playlist model loaded";
    emit this->playlistModelLoaded();
}

SongList* Playlist::getPlaylistModel() const
{
    return m_playlistModel;
}

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
void Playlist::loadPlaylist(SongDetailsList *list)
{
    DB << "loading playlist list";

    if(m_playlist != nullptr)
        delete m_playlist;
    m_playlist = list;

    DB << "playlist loaded";
    emit this->playlistLoaded();
}

void Playlist::shufflePlaylist()
{
    /// code allows to shuffle (with control about order (by getUniqueRandomNumbers
    /// method) of songs) playlist without realocating memory

    int songsCount = m_playlist->c_ref_songs().size();
    auto shuffleOrderList = Playlist::getUniqueRandomNumbers(songsCount);

    /// temporary store songs to add them to original list again
    QList<SongDetails *> tmpList;
    tmpList.reserve(songsCount);
    for(SongDetails *song : m_playlist->c_ref_songs())
    {
        tmpList.append(song);
    }

    /// clear original container and prepare it
    m_playlist->songs().clear();
    m_playlist->songs().reserve(songsCount);

    /// add songs to original list, but with specyfic order
    for(const int &index : shuffleOrderList)
    {
        m_playlist->songs().append(tmpList[index]);
    }

    DB << "playlist shuffled";
    emit this->playlistShuffled();
}

std::vector<int> Playlist::getUniqueRandomNumbers(int count)
{
    // create variables
    std::vector<int> source;
    source.reserve(count);

    std::vector<int> result;
    result.reserve(count);

    // prepare ordered source list [0, count)
    for(int i=0; i<count; ++i)
        source.push_back(i);

    // take random number from source list and add to result (removing number from set)
    for(int i=count; i>0; --i) // from count to 1 (inclusive)
    {
        int random = QRandomGenerator::global()->bounded(i); // range: [0, i)
        result.push_back(source[random]);
        source.erase(source.begin() + random); // remove taken item, range (i variable) will be decremented also
    }

    return result;
}

// SongList Playlist::shuffleList(const SongList &songs)
// {
//     int songsCount = songs.size();

//     auto shuffleOrderList = Playlist::getUniqueRandomNumbers(songsCount);

//     SongList songsNewOrder;
//     int songListIndex = 0;

//     // read random number from songs and append to the songNewOrder list
//     for(const int &randomNumber : shuffleOrderList)
//     {
//         Song *song = songs[randomNumber];
//         song->setListIndex(songListIndex++);
//         songsNewOrder.append(song);
//     }

//     return songsNewOrder;
// }
