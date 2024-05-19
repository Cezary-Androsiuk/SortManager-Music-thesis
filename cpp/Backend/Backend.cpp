#include "Backend.h"

Backend::Backend(QObject *parent)
    : QObject{parent},
    m_personalization(nullptr),
    m_database(nullptr),
    m_player(nullptr),
    m_playlist(nullptr)
{
    this->initializeParameters();
    this->initializeConnections();
    DB << "Backend instance created, waiting for Frontend (QML)...";
}

Backend::~Backend()
{
    m_personalization->savePersonalizationToJson();
}

void Backend::initializeParameters()
{
    m_personalization = new Personalization(this);
    m_personalization->loadPersonalizationFromJson(); /// if an error occur will be handled in checkPersonalization()

    m_database = new Database(this);
    m_database->initializeOnStart();

    m_player = new Player(this);
    m_playlist = new Playlist(this);
}

void Backend::initializeConnections()
{
    /// on any change in personalization, change database equivalent value
    QObject::connect(m_personalization, &Personalization::saveExecQueryNotify, m_database, &Database::setSaveExecQuery);
    QObject::connect(m_personalization, &Personalization::showConstantTagsNotify, m_database, &Database::setShowConstantTags);
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
