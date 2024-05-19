#ifndef PERSONALIZATION_H
#define PERSONALIZATION_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "cpp/DebugPrint/DebugPrint.h"



#define PERSONALIZATION_JSON_PATH "./SortManager-Music/assets/personalization.json"
#define PRINT_LOAD_AND_SAVE_VALUES false


#define DEFAULT_NOTE "### Any change to the contents of this file, may lead to unexpected results ###"
#define DEFAULT_IS_DARK_THEME true
#define DEFAULT_SAVE_EXEC_QUERY true
#define DEFAULT_SHOW_CONSTANT_TAGS true
#define DEFAULT_DARK_ACCENT qRgb(206,147,216)
#define DEFAULT_LIGHT_ACCENT qRgb( 97, 53,131)
#define DEFAULT_DEFAULT_ADD_TAG_TYPE -1
#define DEFAULT_ALWAYS_KEEP_LIST_POS false
#define DEFAULT_SONGS_OPEN_PATH "C:/"

#define CHECK_KEY(arg) if(jp.contains(key)) arg;\
else WR << PERSONALIZATION_JSON_PATH << " file not contains value related with '" << key << "' key";

class Personalization : public QObject
{
    Q_OBJECT
    // frontend app personalisation accessors
    Q_PROPERTY(bool isDarkTheme         READ getIsDarkTheme         WRITE setIsDarkTheme        NOTIFY isDarkThemeChanged       FINAL)
    Q_PROPERTY(bool saveExecQuery       READ getSaveExecQuery       WRITE setSaveExecQuery      NOTIFY saveExecQueryChanged     FINAL)
    Q_PROPERTY(QColor darkAccentColor   READ getDarkAccentColor     WRITE setDarkAccentColor    NOTIFY darkAccentColorChanged   FINAL)
    Q_PROPERTY(QColor lightAccentColor  READ getLightAccentColor    WRITE setLightAccentColor   NOTIFY lightAccentColorChanged  FINAL)
    Q_PROPERTY(bool showConstantTags    READ getShowConstantTags    WRITE setShowConstantTags   NOTIFY showConstantTagsChanged  FINAL)
    Q_PROPERTY(int defaultAddTagType    READ getDefaultAddTagType   WRITE setDefaultAddTagType  NOTIFY defaultAddTagTypeChanged FINAL)
    Q_PROPERTY(bool alwaysKeepListPos   READ getAlwaysKeepListPos   WRITE setAlwaysKeepListPos  NOTIFY alwaysKeepListPosChanged FINAL)
    Q_PROPERTY(QString songOpenPath     READ getSongOpenPath        WRITE setSongOpenPath       NOTIFY songOpenPathChanged      FINAL)

public:
    explicit Personalization(QObject *parent = nullptr);
    ~Personalization();

private:
    void printValues() const;

public:
    void setDefaultPersonalizationData();
    void loadPersonalizationFromJson();
    void savePersonalizationToJson();

public: // getters / setters
    /**
     * returns 0 if none error occur while loading personalization from json,
     * if any error occur while loading personalizations from json error code will be returned
     */
    int getErrorCodeIfOccurWhileLoading() const;

    bool getIsDarkTheme() const;
    bool getSaveExecQuery() const;
    QColor getDarkAccentColor() const;
    QColor getLightAccentColor() const;
    bool getShowConstantTags() const;
    int getDefaultAddTagType() const;
    int getAlwaysKeepListPos() const;
    QString getSongOpenPath() const;

    void setIsDarkTheme(bool isDarkTheme);
    void setSaveExecQuery(bool saveExecQuery);
    void setDarkAccentColor(const QColor &darkAccentColor);
    void setLightAccentColor(const QColor &lightAccentColor);
    void setShowConstantTags(bool showConstantTags);
    void setDefaultAddTagType(int defaultAddTagType);
    void setAlwaysKeepListPos(bool alwaysKeepListPos);
    void setSongOpenPath(const QString &songOpenPath);

signals:
    void isDarkThemeChanged();
    void saveExecQueryChanged();
    void saveExecQueryNotify(bool saveExecQuery);
    void darkAccentColorChanged();
    void lightAccentColorChanged();
    void showConstantTagsChanged();
    void showConstantTagsNotify(bool showConstantTags);
    void defaultAddTagTypeChanged();
    void alwaysKeepListPosChanged();
    void songOpenPathChanged();

private:
    int m_errorCodeIfOccurWhileLoading;

    bool m_isDarkTheme;
    bool m_saveExecQuery;
    QColor m_darkAccentColor;
    QColor m_lightAccentColor;
    bool m_showConstantTags;
    int m_defaultAddTagType;
    bool m_alwaysKeepListPos;
    QString m_songOpenPath;
};

#endif // PERSONALIZATION_H
