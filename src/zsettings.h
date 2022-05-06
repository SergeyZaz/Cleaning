#pragma once

#include <QMap>
#include <QString>
#include <QDate>

class ZSettings
{
public:
    ZSettings() { m_UserType = -1; f_ReadOnly = false; m_PeriodId = 0; };
    QDate   m_CloseDate;
    int m_PeriodId;
    int m_UserType;// 0-администратор, 1-пользователь
    bool f_ReadOnly;
    QString m_UserName;
    QString m_Password;
    static ZSettings& Instance();
};


