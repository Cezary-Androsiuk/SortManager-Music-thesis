#include "Backend.h"

Backend::Backend(QObject *parent)
    : QObject{parent},
    m_personalization(nullptr),
    m_database(nullptr),
    m_player(nullptr),
    m_playlist(nullptr)
{
    this->createParameters();
    this->initializeConnections();
    this->initializeParameters();
    DB << "Backend instance created, waiting for Frontend (QML)...";
}

Backend::~Backend()
{
    m_personalization->savePersonalizationToJson();
}

void Backend::createParameters()
{
    m_personalization =     new Personalization(this);
    m_database =            new Database(this);
    m_player =              new Player(this);
    m_playlist =            new Playlist(this);
}

void Backend::initializeConnections()
{
    /// right after successful initialization, trigger Database initialization
    QObject::connect(this, &Backend::backendInitialized, this, &Backend::initializeBackend);

    /// on any change in personalization, change database equivalent value
    QObject::connect(m_personalization, &Personalization::saveExecQueryChanged, this, [&](){
        m_database->setSaveExecQuery(m_personalization->getSaveExecQuery());
    });
    QObject::connect(m_personalization, &Personalization::showConstantTagsChanged, this, [&](){
        m_database->setShowConstantTags(m_personalization->getShowConstantTags());
    });

    /// when playlist was loaded by Database send it to Playlist and notify Player
    QObject::connect(m_database, &Database::signalPlaylistLoaded, m_player, &Player::resetPlayer);
    QObject::connect(m_database, &Database::signalPlaylistLoaded, m_playlist, &Playlist::loadPlaylist);

    /// when Player finish playing current song, notify Playlist about it
    QObject::connect(m_player, &Player::songEnded, m_playlist, &Playlist::loadNextSongForPlayer);

    /// when Playlist changed current song, notify Player about it (initialization and when Player end song)
    QObject::connect(m_playlist, &Playlist::currentSongChanged, m_player, &Player::changeSong);

}

void Backend::initializeParameters()
{
    m_personalization->setDefaultPersonalizationData(); /// why is called here, is explained in methods body
    m_personalization->loadPersonalizationFromJson(); /// if an error occur will be handled in checkPersonalization()
}

void Backend::initializeBackend()
{
    /// must be outside the constructor, because QML needs time to initialize connections with database,
    /// and now, right after the Backend emits backendInitialized, the Database initialization begins.
    /// Well, this initialization is kinda tough :c

    m_database->initializeOnStart(); /// error will be handled by Database class
}

void Backend::checkPersonalization()
{
    int errorCode = m_personalization->getErrorCodeIfOccurWhileLoading();
    if(errorCode)
    {
        WR << "Personalization load failed with code:" << errorCode;
        emit this->personalizationLoadError(errorCode);
        return;
    }

    emit this->backendInitialized(); // because it is the last step to check initialization
}

void Backend::reinitializePersonalization()
{
    m_personalization->loadPersonalizationFromJson();

    /* emits */ this->checkPersonalization();
}

void Backend::useDefaultPersonalization()
{
    m_personalization->setDefaultPersonalizationData();

    /* emits */ this->checkPersonalization();
}

Personalization *Backend::getPersonalization() const
{
    return m_personalization;
}

Database *Backend::getDatabase() const
{
    return m_database;
}

Player *Backend::getPlayer() const
{
    return m_player;
}

Playlist *Backend::getPlaylist() const
{
    return m_playlist;
}
