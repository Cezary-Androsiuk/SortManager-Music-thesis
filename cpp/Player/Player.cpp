#include "Player.h"

Player::Player(QObject *parent)
    : QObject{parent},
    m_playerStarted(false)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_song = new SongDetails(this);

    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(QUrl::fromLocalFile("C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (1080p_25fps_H264-128kbit_AAC).mp4"));


    QObject::connect(this, &Player::songChanged, this, &Player::updatePlayer);
    m_audioOutput->setVolume(0.1);
}

void Player::play()
{
    m_player->play();

    m_playerStarted = true;
}

void Player::pause()
{
    m_player->pause();
}

void Player::changeSong(const SongDetails *receivedSong)
{
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

    DB << "song was changed";
    emit this->songChanged();
}

void Player::resetPlayer()
{
    m_playerStarted = false;
}

void Player::updatePlayer()
{
    DB << "updating player...";
    if(!m_playerStarted)
    {
        this->startPlayer();
    }
    else
    {
        m_player->setSource(this->getSongTagValueByID(9/*Song Path*/));
        m_player->play();
    }
    DB << "player updated to: " << this->getSongTagValueByID(2/*Name*/);
}

void Player::startPlayer()
{
    DB << "starting player...";
    /// start player will not play song, it just prepare song (after playlist was loaded)
    /// to handle case when user start the app -> first song will be shown in player
    /// but it won't playing

    m_player->setSource(this->getSongTagValueByID(9/*Song Path*/));
    DB << "player started";
}

QString Player::getSongTagValueByID(qsizetype id) const
{
    for(const Tag *tag : m_song->get_tags()->c_ref_tags())
    {
        if(tag->get_id() == id)
            return tag->get_value();
    }
    WR << "tag not found";
    exit(1);
}
