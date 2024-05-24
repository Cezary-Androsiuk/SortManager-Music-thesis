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
    /// on any change in personalization, change database equivalent value
    QObject::connect(m_personalization, &Personalization::saveExecQueryChanged, this, [&](){
        m_database->setSaveExecQuery(m_personalization->getSaveExecQuery());
    });
    QObject::connect(m_personalization, &Personalization::showConstantTagsChanged, this, [&](){
        m_database->setShowConstantTags(m_personalization->getShowConstantTags());
    });

    // QObject::connect(m_database &Database::signalPlaylistListLoaded, m_playlist, &Playlist::loadNewPlaylistList);
}

void Backend::initializeParameters()
{
    m_personalization->setDefaultPersonalizationData(); /// why is called here, is explained in methods body
    m_personalization->loadPersonalizationFromJson(); /// if an error occur will be handled in checkPersonalization()
    m_database->initializeOnStart();
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
