#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSqlQuery>

class UserSession {
  public:
    static bool login(const QString user, const QString password);
    static int getId();
    static int getLoja();
    static QSqlQuery *initialize();
    static QString getNome();
    static QString getSigla();
    static QString getSiglaLoja();
    static QString getTipoUsuario();
    static void logout();
    static void free();
    static QString getFromLoja(QString parameter);
    static QStringList getTodosCNPJ();

  private:
    // attributes
    static QSqlQuery *query;
    // methods
    UserSession();
};

#endif // USERSESSION_H
