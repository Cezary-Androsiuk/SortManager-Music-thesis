#include "Personalization.h"

Personalization::Personalization(QObject *parent)
    : QObject{parent},
    m_isDarkTheme(DEFAULT_IS_DARK_THEME),
    m_saveExecQuery(DEFAULT_SAVE_EXEC_QUERY),
    m_darkAccentColor(DEFAULT_DARK_ACCENT),
    m_lightAccentColor(DEFAULT_LIGHT_ACCENT),
    m_showConstantTags(DEFAULT_SHOW_CONSTANT_TAGS),
    m_defaultAddTagType(DEFAULT_DEFAULT_ADD_TAG_TYPE),
    m_alwaysKeepListPos(DEFAULT_ALWAYS_KEEP_LIST_POS),
    m_songOpenPath(DEFAULT_SONGS_OPEN_PATH)
{

}

Personalization::~Personalization()
{

}

void Personalization::printValues() const
{
#if PRINT_LOAD_AND_SAVE_VALUES
    DB << "\t - isDarkTheme: " << m_isDarkTheme;
    DB << "\t - saveExecQuery: " << m_saveExecQuery;
    DB << "\t - darkAccentColor: " << m_darkAccentColor.rgba();
    DB << "\t - lightAccentColor: " << m_lightAccentColor.rgba();
    DB << "\t - showConstantTags: " << m_showConstantTags;
    DB << "\t - defaultAddTagType: " << m_defaultAddTagType;
    DB << "\t - alwaysKeepListPos: " << m_alwaysKeepListPos;
    DB << "\t - songOpenPath: " << m_songOpenPath;
#endif
}

void Personalization::setDefaultPersonalizationData()
{
    DB << "assigning default values";

    m_isDarkTheme = DEFAULT_IS_DARK_THEME;
    m_saveExecQuery = DEFAULT_SAVE_EXEC_QUERY;
    m_darkAccentColor = DEFAULT_DARK_ACCENT;
    m_lightAccentColor = DEFAULT_LIGHT_ACCENT;
    m_showConstantTags = DEFAULT_SHOW_CONSTANT_TAGS;
    m_defaultAddTagType = DEFAULT_DEFAULT_ADD_TAG_TYPE;
    m_alwaysKeepListPos = DEFAULT_ALWAYS_KEEP_LIST_POS;
    m_songOpenPath = DEFAULT_SONGS_OPEN_PATH;

    m_errorCodeIfOccurWhileLoading = 0;
}

void Personalization::loadPersonalizationFromJson()
{
    auto PJP = PERSONALIZATION_JSON_PATH;
    if(!QFile(PJP).exists()){
        WR << "file " << PERSONALIZATION_JSON_PATH << " not found";
        m_errorCodeIfOccurWhileLoading = 10;
        return;
    }

    QFile json_file(PJP);
    if(!json_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        WR << "Can not open personalization json file: " << PJP;
        m_errorCodeIfOccurWhileLoading = 20;
        return;
    }

    QJsonParseError json_error;
    QJsonDocument json_data = QJsonDocument::fromJson(json_file.readAll(), &json_error);
    json_file.close();

    if(json_error.error != QJsonParseError::NoError) {
        WR << "json parse error: " << json_error.errorString();
        m_errorCodeIfOccurWhileLoading = 30;
        return;
    }

    if(!json_data.isObject()){
        WR << "json file does not contains json object";
        m_errorCodeIfOccurWhileLoading = 40;
        return;
    }

    // at this point data are default

    auto jp = json_data.object();
    QString key;

    // try to load data, but if key is missing then notify
    // following code is compressed and can be used only for this purpose

    key = "is dark theme";
    CHECK_KEY(this->setIsDarkTheme(jp[key].toBool()))

    key = "save exec query";
    CHECK_KEY(this->setSaveExecQuery(jp[key].toBool()))

    key = "dark accent color";
    CHECK_KEY(this->setDarkAccentColor((QRgb)jp[key].toInteger()))

    key = "light accent color";
    CHECK_KEY(this->setLightAccentColor((QRgb)jp[key].toInteger()))

    key = "show constant tags";
    CHECK_KEY(this->setShowConstantTags(jp[key].toBool()))

    key = "default add tag type";
    CHECK_KEY(this->setDefaultAddTagType(jp[key].toInteger()))

    key = "always keep list position";
    CHECK_KEY(this->setAlwaysKeepListPos(jp[key].toInteger()))

    key = "songs open path";
    CHECK_KEY(this->setSongOpenPath(jp[key].toString()))


    DB << "personalization data readed!";
    this->printValues();
    m_errorCodeIfOccurWhileLoading = 0;
}

void Personalization::savePersonalizationToJson()
{
    QString PJP = PERSONALIZATION_JSON_PATH;

    int last_forward_slash = PJP.lastIndexOf('/');
    if(last_forward_slash == -1)
    {
        DB << "it is not possible to determine whether the personalisation save "
              "file is in any sub-pathway, we assume that it is in the application pathway";
    }
    else
    {
        // found that save file is in some directory, create it if not exist
        QDir personalization_directory(PJP.left(last_forward_slash));

        if(!personalization_directory.exists())
        {
            if(!personalization_directory.mkpath("."))
            {
                WR << "error while creating path for" << PJP << "file!";
            }
        }
    }

    QJsonObject json_object;
    json_object["!NOTE"] = QString(DEFAULT_NOTE);

    json_object["is dark theme"] = this->getIsDarkTheme();
    json_object["save exec query"] = this->getSaveExecQuery();
    json_object["dark accent color"] = (qint64) this->getDarkAccentColor().rgba();
    json_object["light accent color"] = (qint64) this->getLightAccentColor().rgba();
    json_object["show constant tags"] = this->getShowConstantTags();
    json_object["default add tag type"] = this->getDefaultAddTagType();
    json_object["always keep list position"] = this->getAlwaysKeepListPos();
    json_object["songs open path"] = this->getSongOpenPath();

    QJsonDocument json_data(json_object);

    QFile file(PJP);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        WR << "error while saving json file to " << PJP << " with error " << file.errorString();
        return;
    }

    file.write(json_data.toJson());
    file.close();

    DB << "personalization data saved!";
    this->printValues();
    return;
}

int Personalization::getErrorCodeIfOccurWhileLoading() const
{
    return m_errorCodeIfOccurWhileLoading;
}

bool Personalization::getIsDarkTheme() const
{
    return m_isDarkTheme;
}

bool Personalization::getSaveExecQuery() const
{
    return m_saveExecQuery;
}

QColor Personalization::getDarkAccentColor() const
{
    return m_darkAccentColor;
}

QColor Personalization::getLightAccentColor() const
{
    return m_lightAccentColor;
}

bool Personalization::getShowConstantTags() const
{
    return m_showConstantTags;
}

int Personalization::getDefaultAddTagType() const
{
    return m_defaultAddTagType;
}

int Personalization::getAlwaysKeepListPos() const
{
    return m_alwaysKeepListPos;
}

QString Personalization::getSongOpenPath() const
{
    return m_songOpenPath;
}

void Personalization::setIsDarkTheme(bool isDarkTheme)
{
    if(m_isDarkTheme != isDarkTheme)
    {
        m_isDarkTheme = isDarkTheme;
        emit this->isDarkThemeChanged();
    }

}

void Personalization::setSaveExecQuery(bool saveExecQuery)
{
    if(m_saveExecQuery != saveExecQuery)
    {
        m_saveExecQuery = saveExecQuery;
        emit this->saveExecQueryChanged();
    }
    emit this->saveExecQueryNotify(m_saveExecQuery);
}

void Personalization::setDarkAccentColor(const QColor &darkAccentColor)
{
    if(m_darkAccentColor != darkAccentColor)
    {
        m_darkAccentColor = darkAccentColor;
        emit this->darkAccentColorChanged();
    }

}

void Personalization::setLightAccentColor(const QColor &lightAccentColor)
{
    if(m_lightAccentColor != lightAccentColor)
    {
        m_lightAccentColor = lightAccentColor;
        emit this->lightAccentColorChanged();
    }

}

void Personalization::setShowConstantTags(bool showConstantTags)
{
    if(m_showConstantTags != showConstantTags)
    {
        m_showConstantTags = showConstantTags;
        emit this->showConstantTagsChanged();
    }
    emit this->showConstantTagsNotify(m_showConstantTags);
}

void Personalization::setDefaultAddTagType(int defaultAddTagType)
{
    if(m_defaultAddTagType != defaultAddTagType)
    {
        m_defaultAddTagType = defaultAddTagType;
        emit this->defaultAddTagTypeChanged();
    }

}

void Personalization::setAlwaysKeepListPos(bool alwaysKeepListPos)
{
    if(m_alwaysKeepListPos != alwaysKeepListPos)
    {
        m_alwaysKeepListPos = alwaysKeepListPos;
        emit this->alwaysKeepListPosChanged();
    }

}

void Personalization::setSongOpenPath(const QString &songOpenPath)
{
    if(m_songOpenPath != songOpenPath)
    {
        m_songOpenPath = songOpenPath;
        emit this->songOpenPathChanged();
    }

}
