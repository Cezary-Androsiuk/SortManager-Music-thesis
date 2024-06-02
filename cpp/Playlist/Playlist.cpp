#include "Playlist.h"

Playlist::Playlist(QObject *parent)
    : QObject{parent},
    m_playlistModel(nullptr),
    m_playlist(nullptr),
    m_songState({-1, -1, -1})
{
    /// after new playlist was loaded, shuffle it
    QObject::connect(this, &Playlist::playlistLoaded, this, &Playlist::shufflePlaylist);

    // /// load playlist model after playlist was shuffled (but not after playlistLoad, because
    // /// playlistLoad triggers shufflePlaylist)
    // // QObject::connect(this, &Playlist::playlistLoaded, this, &Playlist::loadPlaylistModel);
    // QObject::connect(this, &Playlist::playlistShuffled, this, &Playlist::loadPlaylistModel);

    /// after any playlist change update song state
    QObject::connect(this, &Playlist::playlistShuffled, this, &Playlist::updateSongState);
    QObject::connect(this, &Playlist::songStateChanged, this, &Playlist::loadPlaylistModel);
}

void Playlist::loadPlaylistModel()
{
    /// clear models memory because loadPlaylistModel is called only when
    /// something changed
    if(m_playlistModel != nullptr)
        delete m_playlistModel;
    m_playlistModel = nullptr;


    m_playlistModel = new SongList(this);

    for(const SongDetails *songDetails : m_playlist->c_ref_songs())
    {
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

void Playlist::loadPlaylist(SongDetailsList *list)
{

    if(m_playlist != nullptr)
        delete m_playlist;
    m_playlist = list;

    /// reset values (player will be reseted simultaneously)
    m_songState = {-1, -1, -1};

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

void Playlist::updateSongState()
{
    /// if playlist is empty set values to -1
    if(m_playlist->c_ref_songs().empty())
    {
        DB << "set values to -1 due to empty playlist";
        m_songState = {-1, -1, -1};
        return;
    }

    qsizetype &cpos = m_songState.m_currentPos;   // current position
    qsizetype &cid = m_songState.m_currentID;     // current id
    qsizetype &npos = m_songState.m_nextPos;      // next position

    if(cpos == -1 || cid == -1 || npos == -1)
    {
        /// print debug info to check if all values are really -1
        if(cpos != -1 || cid != -1 || npos != -1)
        {
            DB << "not all values are -1 ->"
               << (QString("{cpos: %1, cid: %2, npos: %3}")
                       .arg(cpos).arg(cid).arg(npos)).toStdString().c_str();
            exit(1);
        }

        /// at this point playlist is not empty and all song states are -1

        cpos = 0;
        cid = this->getIDKnowingPos(cpos);
        npos = this->getComputedNextSongPos(); /// cpos was already set, and can be used
    }
    else
    {
        /// if all of the song states are not -1, that means are values was
        ///   set ealier (I know, I am smart af XD). If it is not first call of method,
        ///   we can assume that player press shuffle (reload set values to -1)
        /// shuffle pressed action:

        /// cpos need to be set again (this time using ID of song)
        cpos = this->getPosKnowingID(cid);
        /// in case cpos was set as a first one set npos as second
        if(cpos == 0)
            npos = 1;
        else
            npos = 0;
    }

    DB << "song state changed";
    emit this->songStateChanged();
}

void Playlist::songPlaylingEnded()
{
    // qsizetype lastSongIndex = m_playlist->c_ref_songs().size() - 1;
    // if()
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

qsizetype Playlist::getPosKnowingID(const qsizetype &id) const
{
    /// iterate over all song until id matches
    qsizetype pos = 0;
    for(SongDetails *song : m_playlist->c_ref_songs())
    {
        if(song->get_id() == id)
        {
            return pos;
        }
        ++pos;
    }

    /// if id was not found in list
    WR << "song with id" << id << "NOT FOUND!";
    return -1;
}

qsizetype Playlist::getIDKnowingPos(const qsizetype &pos) const
{
    /// if pos larger than list
    if(pos >= m_playlist->c_ref_songs().size())
    {
        WR << "song with pos" << pos << "NOT FOUND!";
        return -1;
    }

    /// get id from list by position
    return m_playlist->c_ref_songs().at(pos)->get_id();
}

qsizetype Playlist::getComputedNextSongPos() const
{
    qsizetype listSize = m_playlist->c_ref_songs().size();
    const qsizetype &cpos = m_songState.m_currentPos;

    /// test if values are expected
    if(listSize == 0)
    {
        WR << "playlist cannot be empty when computing nextPos";
        exit(1);
    }
    if(cpos == -1)
    {
        WR << "currentPos value cannot be -1 when computing nextPos";
        exit(1);
    }

    /// for list length cases return nextPos
    if(cpos >= listSize-1)  /// case when currentPos is at the end of the list
        return 0;
    else                    /// case when currentPos is not at the end of the list
        return cpos+1;
}

qsizetype Playlist::getCurrentPos() const
{
    return m_songState.m_currentPos;
}

qsizetype Playlist::getCurrentID() const
{
    return m_songState.m_currentID;
}

qsizetype Playlist::getNextPos() const
{
    return m_songState.m_nextPos;
}

void Playlist::setCurrentPos(const qsizetype &pos)
{
    m_songState.m_currentPos = pos;
}

void Playlist::setCurrentID(const qsizetype &id)
{
    m_songState.m_currentID = id;
}

void Playlist::setNextPos(const qsizetype &pos)
{
    m_songState.m_nextPos = pos;
}
