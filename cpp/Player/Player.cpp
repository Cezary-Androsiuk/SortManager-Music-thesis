#include "Player.h"

Player::Player(QObject *parent)
    : QObject{parent},
    m_playerStarted(false)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_song = new SongDetails(this);


    this->buildParametersConnections();


    /// connection don't need to be reconeced after restart player, cause it won't use parameters (m_player, m_song, ...)
    QObject::connect(this, &Player::songChanged, this, &Player::updatePlayer);
}

void Player::buildParametersConnections()
{
    /// is called after parameters was reinitialized (like after restart player)
    /// to reconnect broken connections

    m_player->setAudioOutput(m_audioOutput);

    QObject::connect(m_player, &QMediaPlayer::positionChanged, this, &Player::updatePlayerProgress);
    QObject::connect(m_player, &QMediaPlayer::positionChanged, this, &Player::songPositionChanged);

    QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &Player::onMediaStatusChanged);

    QObject::connect(m_player, &QMediaPlayer::playingChanged, this, &Player::playingChanged);
}

void Player::play()
{
    if(m_player->isPlaying())
    {
        m_player->pause();
        m_playerStarted = false;
    }
    else
    {
        m_playerStarted = true;
        m_player->play();
    }
}

void Player::nextSong()
{
    emit this->songEnded();
}

void Player::restartSong()
{
    this->updatePlayer(); /// gives effect equal to move player to the start
}

void Player::setVolume(int volume)
{
    m_volume = static_cast<float>(volume) / 100.f;
    this->m_audioOutput->setVolume(m_volume);
    /// after restart m_audioOutput variable, volume need to be set again
}

void Player::changeSong(const SongDetails *receivedSong)
{
    /// update player (to the same song) only when player is not playing
    if(receivedSong->get_id() == m_song->get_id())
    {
        DB << "updating song to the same one";
        if(m_player->isPlaying())
        {
            return;
        }

        /// if not playing, then song can be changed
    }

    /// use received song as a source to create this song
    m_song->set_id(receivedSong->get_id());
    TagList *tagList = new TagList(m_song);

    const TagList *receivedTagList = receivedSong->get_tags();
    for(const Tag *receivedTag : receivedTagList->c_ref_tags())
    {
        Tag *tag = new Tag(tagList);

        tag->set_id(receivedTag->get_id());
        tag->set_name(receivedTag->get_name());
        tag->set_type(receivedTag->get_type());
        tag->set_value(receivedTag->get_value());
        tag->set_is_editable(receivedTag->get_is_editable());
        tag->set_is_immutable(receivedTag->get_is_immutable());
        tag->set_is_required(receivedTag->get_is_required());

        tagList->tags().append(tag);
    }
    m_song->set_tags(tagList); // set_tags removes old ones

    m_songData.songID = this->getSongTagValueByID(1 /*ID*/).toInt();
    m_songData.title = this->getSongTagValueByID(2/*Name*/);
    m_songData.thumbnail = this->validThumbnailPath(
        this->getSongTagValueByID(10/*Thumbnail*/));
    m_songData.begin = this->getSongTagValueByID(5/*Begin*/).toLongLong();
    m_songData.end = this->getSongTagValueByID(6/*End*/).toLongLong();

    DB << "song was changed";
    emit this->songChanged();
}

void Player::resetPlayer()
{
    m_playerStarted = false;

    if(m_player != nullptr) delete m_player;
    m_player = new QMediaPlayer(this);

    if(m_audioOutput != nullptr) delete m_audioOutput;
    m_audioOutput = new QAudioOutput(this);
    this->m_audioOutput->setVolume(m_volume);

    if(m_song != nullptr) delete m_song;
    m_song = new SongDetails(this);

    this->buildParametersConnections();
}

void Player::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::MediaStatus::BufferedMedia:
        // DB << "media player status changed to: BufferedMedia";

        break;
    case QMediaPlayer::MediaStatus::BufferingMedia:
        // DB << "media player status changed to: BufferingMedia";

        break;
    case QMediaPlayer::MediaStatus::EndOfMedia:
        DB << "media player status changed to: EndOfMedia";
        emit this->songEnded();
        break;
    case QMediaPlayer::MediaStatus::InvalidMedia:
        DB << "media player status changed to: InvalidMedia";
        emit this->songEnded();
        break;
    case QMediaPlayer::MediaStatus::LoadedMedia:
        if(m_songData.position != 0) /// that means song is playing and do not set position again
            break;
        /// song need to be loaded again when position was changed (for example go back by 10s)
        DB << "media player status changed to: LoadedMedia";
        m_player->setPosition(m_songData.begin);
        m_songData.duration = m_player->duration();
        m_lastUpdatedDisplayValues = m_player->position();
        emit this->displayPositionChanged();
        emit this->displayDurationChanged();
        emit this->songFullyLoaded();
        emit this->songStarted();
        break;
    case QMediaPlayer::MediaStatus::LoadingMedia:
        // DB << "media player status changed to: LoadingMedia";

        break;
    case QMediaPlayer::MediaStatus::NoMedia:
        DB << "media player status changed to: NoMedia";
        emit this->songEnded();
        break;
    case QMediaPlayer::MediaStatus::StalledMedia:
        // DB << "media player status changed to: StalledMedia";

        break;
    }

}

void Player::updatePlayer()
{
    m_player->setSource(this->getSongTagValueByID(9/*Song Path*/));
    /// some data are set after LoadedMedia

    /// do not play song if wasn't playing before songs change
    /// it handle case when user start the app -> first song will be shown in player
    if(m_playerStarted)
    {
        m_player->play();
    }
    DB << "player updated to: " << m_songData.title;
}

void Player::updatePlayerProgress(qsizetype position)
{
    qsizetype remainingTime = m_player->duration() - position;
    m_songData.position = position - m_songData.begin;

    if(remainingTime <= m_player->duration() - m_songData.end && m_songData.end != 0)
    {
        // DB << "end";
        m_player->pause();
        emit this->songEnded();
    }

    emit this->displayPositionChanged();
}

QString Player::getSongTagValueByID(qsizetype id) const
{
    for(const Tag *tag : m_song->get_tags()->c_ref_tags())
    {
        if(tag->get_id() == id)
            return tag->get_value();
    }
    WR << "tag not found! looking for tag id="<< id << "in" << m_song;
    if(m_song != nullptr)
    {
        WR << m_song->get_id();
        WR << m_song->get_tags()->c_ref_tags();
    }
    exit(1);
}

QString Player::validThumbnailPath(QString thumbnail) const
{
    DB << "thumbnail: " << thumbnail;
    if(QFile::exists(QUrl(thumbnail).toLocalFile()))
    {
        return thumbnail;
    }
    else
    {
        DB << "song does not contains thumbnail";
        return "";
    }
}

QString Player::createDisplayTime(qsizetype time)
{
    int minutes = time / (1000 * 60);
    int seconds = time / 1000 - minutes * 60;
    // int miliseconds = time - seconds * 1000 - minutes * 1000 * 60;
    return QString(
        (minutes < 10 ? "0" : "") + QString::number(minutes) + ":" +
        (seconds < 10 ? "0" : "") + QString::number(seconds));
}

bool Player::getIsPlaying() const
{
    return this->m_player->isPlaying();
}

qsizetype Player::getSongID() const
{
    return m_songData.songID;
}

QString Player::getTitle() const
{
    return m_songData.title;
}

QString Player::getThumbnail() const
{
    return m_songData.thumbnail;
}

qsizetype Player::getDuration() const
{
    return m_songData.duration;
}

qsizetype Player::getPosition() const
{
    return m_songData.position;
}

QString Player::getDisplayDuration() const
{
    /// i know that getters shouln't compute anything, but this code is already a nice spaghetti
    return Player::createDisplayTime(m_songData.duration);
}

QString Player::getDisplayPosition() const
{
    /// i know that getters shouln't compute anything, but this code is already a nice spaghetti
    return Player::createDisplayTime(m_songData.position);
}

void Player::setPosition(qsizetype position)
{
    if(m_songData.position == position)
        return;

    m_songData.position = position;
    m_lastUpdatedDisplayValues = position;
    m_player->setPosition(position);
    emit this->songPositionChanged();
}
