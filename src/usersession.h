#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSqlQuery>

class UserSession {
  public:
    static bool login(const QString &user, const QString &password);
    static int idUsuario();
    static int loja();
    static QString nome();
    static QString tipoUsuario();
    static QString fromLoja(const QString &parameter, const QString &user = nome());
    static QVariant settings(const QString &key);
    static void setSettings(const QString &key, const QVariant &value);
    static bool settingsContains(const QString &key);
    static void free();

  private:
    // attributes
    static QSqlQuery *query;
    // methods
    UserSession();
    static QSqlQuery *initialize();
    static void logout();
};

#endif // USERSESSION_H
