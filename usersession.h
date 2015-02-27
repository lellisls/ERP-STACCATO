#ifndef USERSESSION_H
#define USERSESSION_H

#include <QPointer>
#include <QSqlQuery>
#include <QString>

class UserSession {
  public:
    static bool login(QString user, QString password);
    static int getId();
    static int getLoja();
    static QSqlQuery *initialize();
    static QString getNome();
    static QString getSigla();
    static QString getSiglaLoja();
    static QString getTipo();
    static void logout();

  private:
    // attributes
    static QSqlQuery *query;
    static int loja;
    // methods
    UserSession();
};

#endif // USERSESSION_H
