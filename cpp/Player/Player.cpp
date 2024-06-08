#include "Player.h"

Player::Player(QObject *parent)
    : QObject{parent},
    m_playerStarted(false)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_song = new SongDetails(this);


    this->buildParametersConnections();

    // m_player->setSource(QUrl::fromLocalFile("C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (1080p_25fps_H264-128kbit_AAC).mp4"));


    /// connection don't need to be reconeced after restart player, cause it won't use parameters (m_player, m_song, ...)
    QObject::connect(this, &Player::songChanged, this, &Player::updatePlayer);
}

void Player::buildParametersConnections()
{
    /// is called after parameters was reinitialized (like after restart player)
    /// to reconnect broken connections

    m_audioOutput->setVolume(0.1);
    m_player->setAudioOutput(m_audioOutput);

    QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &Player::onMediaStatusChanged);
}

void Player::play()
{
    m_playerStarted = true;

    m_player->play();
}

void Player::pause()
{
    m_player->pause();

    m_playerStarted = false;
}

void Player::nextSong()
{
    emit this->songEnded();
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
    m_song->set_tags(tagList);

    m_songData.title = this->getSongTagValueByID(2/*Name*/);
    m_songData.thumbnail = this->getSongTagValueByID(10/*Thumbnail*/);

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
        // DB << "media player status changed to: LoadedMedia";

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

    /// do not play song if wasn't playing before songs change
    /// it handle case when user start the app -> first song will be shown in player
    if(m_playerStarted)
    {
        m_player->play();
    }
    DB << "player updated to: " << m_songData.title;
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

QString Player::getTitle() const
{
    return m_songData.title;
}
