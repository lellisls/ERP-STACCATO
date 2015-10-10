#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSqlQuery>

class UserSession {
  public:
    static bool login(const QString user, const QString password);
    static int getIdUsuario();
    static int getLoja();
    static QString getNome();
    static QString getSiglaLoja();
    static QString getTipoUsuario();
    static QStringList getTodosCNPJ();
    static QString getFromLoja(QString parameter);
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
