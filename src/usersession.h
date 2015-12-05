#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSqlQuery>

class UserSession {
  public:
    static bool login(const QString &user, const QString &password);
    static int getIdUsuario();
    static int getLoja();
    static QString getNome();
    static QString getTipoUsuario();
    static QString getFromLoja(const QString &parameter, const QString &user = getNome());
    static QVariant getSettings(const QString &key);
    static void setSettings(const QString &key, const QVariant &value);
    static bool settingsContains(const QString &key);
    static void free();

  private:
    // attributes
    static QSqlQuery *query;
    // methods
    UserSession();
    static QSqlQuery *initialize();
    static QString getSigla();
    static void logout();
};

#endif // USERSESSION_H
