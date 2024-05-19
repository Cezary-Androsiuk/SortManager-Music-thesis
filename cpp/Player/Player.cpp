#include "Player.h"

Player::Player(QObject *parent)
    : QObject{parent}
{
    m_player = new QMediaPlayer(this);
    m_player_2 = new QMediaPlayer(this);
    m_audio_output = new QAudioOutput(this);
    m_audio_output_2 = new QAudioOutput(this);

    m_player->setAudioOutput(m_audio_output);
    m_player_2->setAudioOutput(m_audio_output_2);
    m_player->setSource(QUrl::fromLocalFile("C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (1080p_25fps_H264-128kbit_AAC).mp4"));
    m_player_2->setSource(QUrl::fromLocalFile("C:/0_Vigiland - Friday Night‬‬‬/Vigiland - Friday Night‬‬‬ (1080p_25fps_H264-128kbit_AAC).mp4"));

    // m_audio_output->setVolume(0.4);
}

void Player::play()
{
    m_player->play();
}

void Player::pause()
{
    m_player->pause();
}

void Player::play2()
{
    m_player_2->play();
}

void Player::pause2()
{
    m_player_2->pause();
}
