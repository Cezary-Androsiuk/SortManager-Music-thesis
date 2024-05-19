#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QCoreApplication> // QCoreApplication::quit();

#include <QColor>

#include "cpp/DebugPrint/DebugPrint.h"

#include "cpp/Personalization/Personalization.h"
#include "cpp/Database/Database.h"
#include "cpp/Player/Player.h"
#include "cpp/Playlist/Playlist.h"

class Backend : public QObject
{
    /*
     * naming convention:
     * private variables starts with "m_" and snake_case
     * getters for private variables are just like their variable but instead "m_" prefix used "get_"
     * setters for private variables are just like their variable but instead "m_" prefix used "set_"
     * signals for private variables are just like their variable but without "m_" prefix and with "Changed" postfix
     *
     * methods that acctualy do something (app logic) are named using camelCase
     *
     * getters and setters that belong to m_database contains prefix "db_"
    */

    Q_OBJECT
    Q_PROPERTY(Personalization* personalization     READ getPersonalization    CONSTANT FINAL)
    Q_PROPERTY(Database* database                   READ getDatabase           CONSTANT FINAL)
    Q_PROPERTY(Player* player                       READ getPlayer             CONSTANT FINAL)
    Q_PROPERTY(Playlist* playlist                   READ getPlaylist           CONSTANT FINAL)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();

    /// initialize Backend
    void initializeParameters();
    void initializeConnections();

    /// destroy Backend
    // none

public slots: /// after qml was loaded (to check if steps are correctly initialized or display popup)
    void checkPersonalization();

signals: /// communicate with QML to check if initialized correctly (emited by above slots)
    void personalizationLoaded();                       /// emitted after confirmed that personalization has been initialized correctly
    void personalizationLoadError(int errorCode);       /// emitted after finding out that personalization has not been initialized correctly
    void backendInitialized();                          /// emitted after confirming that the last(currently personalization) of the steps(these above) has been correctly initialized

public slots: /// initialize actions
    void reinitializePersonalization();
    void useDefaultPersonalization();

public: // getters
    Personalization *getPersonalization() const;
    Database *getDatabase() const;
    Player *getPlayer() const;
    Playlist *getPlaylist() const;

private:
    Personalization* m_personalization;
    Database* m_database;
    Player *m_player;
    Playlist *m_playlist;
};

#endif // BACKEND_H
