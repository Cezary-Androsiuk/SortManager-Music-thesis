#include "Player.h"

Player::Player(QObject *parent)
    : QObject{parent},
    m_playerStarted(false),
    m_isPlayerEmpty(false)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);


    this->buildParametersConnections();
}

void Player::buildParametersConnections()
{
    /// is called after parameters was reinitialized (like after restart player)
    /// to reconnect broken connections

    m_player->setAudioOutput(m_audioOutput);

    QObject::connect(m_player, &QMediaPlayer::positionChanged, this, &Player::updateDisplayPosition);
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

void Player::prevSong()
{
    // this->m_player->setPosition(0);
    emit this->songPrevious();
}

void Player::setVolume(int volume)
{
    m_volume = static_cast<float>(volume) / 100.f;
    this->m_audioOutput->setVolume(m_volume);
    /// after restart m_audioOutput variable, volume need to be set again
}

void Player::changeSong(const SongDetails *receivedSong)
{
    m_isPlayerEmpty = false;
    emit this->isPlayerEmptyChanged();

    /// update player (to the same song) only when player is not playing
    if(receivedSong->get_id() ==m_songData.songID)
    {
        DB << "updating song to the same one";
        if(m_player->isPlaying())
        {
            return;
        }

        /// if not playing, then song can be changed
    }

    /// use received song as a source to create this song
    SongDetails song;
    song.set_id(receivedSong->get_id());
    TagList *tagList = new TagList(&song);

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
    song.set_tags(tagList); // set_tags removes old ones

    m_songData.songID = this->getSongTagValueByID(&song, 1  /*ID*/          ).toInt();
    m_songData.title =  this->getSongTagValueByID(&song, 2  /*Name*/        );
    m_songData.thumbnail = this->validThumbnailPath(
                        this->getSongTagValueByID(&song, 10 /*Thumbnail*/   ));
    m_player->setSource(this->getSongTagValueByID(&song, 9  /*Song Path*/   ));
    /// some data are set after LoadedMedia

    /// do not play song if wasn't playing before songs change
    /// it handle case when user start the app -> first song will be shown in player
    /// and support continues playing within the song change
    if(m_playerStarted)
    {
        m_player->play();
    }

    DB << "song was changed" << m_songData.title;
    emit this->songChanged();
}

void Player::clearPlayerNoSong()
{
    m_isPlayerEmpty = true;
    emit this->isPlayerEmptyChanged();
    resetPlayer();
}

void Player::resetPlayer()
{
    m_playerStarted = false;

    if(m_player != nullptr) delete m_player;
    m_player = new QMediaPlayer(this);

    if(m_audioOutput != nullptr) delete m_audioOutput;
    m_audioOutput = new QAudioOutput(this);
    this->m_audioOutput->setVolume(m_volume);

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
        if(m_player->position() != 0) /// that means song is playing and do not set position again
            break;
        /// song need to be loaded again when position was changed (for example go back by 10s)
        DB << "media player status changed to: LoadedMedia";

        emit this->songPositionChanged();
        this->updateDisplayPosition(0);
        this->updateDisplayDuration(m_player->duration());

        emit this->songLoaded();
        emit this->songStarted(); /// for SongTitle (i don't remember why)
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

void Player::updateDisplayPosition(qint64 position)
{
    m_displayPosition = Player::createDisplayTime(position);
    emit this->displayPositionChanged();
}

void Player::updateDisplayDuration(qint64 duration)
{
    m_displayDuration = Player::createDisplayTime(duration);
    emit this->displayDurationChanged();
}

QString Player::getSongTagValueByID(SongDetails *song, qsizetype id) const
{
    for(const Tag *tag : song->get_tags()->c_ref_tags())
    {
        if(tag->get_id() == id)
            return tag->get_value();
    }
    WR << "tag not found! looking for tag id="<< id << "in" << song;
    if(song != nullptr)
    {
        WR << song->get_id();
        WR << song->get_tags()->c_ref_tags();
    }
    WR << "CRITICAL ERROR -> EXIT";
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
    return m_player->duration();
}

qsizetype Player::getPosition() const
{
    return m_player->position();
}

QString Player::getDisplayDuration() const
{
    return m_displayDuration;
}

QString Player::getDisplayPosition() const
{
    return m_displayPosition;
}

bool Player::getIsPlayerEmpty() const
{
    return m_isPlayerEmpty;
}

void Player::setPosition(qsizetype position)
{
    if(m_player->position() == position)
        return;

    m_player->setPosition(position);
    emit this->songPositionChanged();
}
