#ifndef INITDB_H
#define INITDB_H

#include <QDebug>
#include <QFile>
#include <QtSql>
#include <QMessageBox>

bool loadScript(const QString &filename) {
//  qDebug() << "LOADING " << filename << ".";
  QFile file(filename);

  if (not file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "sql error: " << QSqlError();
    return false;
  }

  QTextStream stream(&file);
  stream.setLocale(QLocale::Portuguese);
  stream.setCodec("UTF-8");
  QString script;

  while (not stream.atEnd()) {
    QString line = stream.readLine();

    if (not line.startsWith("--") and line.size() > 5) {
      script += line + "\n";
    }
  }

  QSqlQuery query;
  QStringList queryes = script.split(QChar(';'));

  foreach (QString queryString, queryes) {
    if (queryString.size() > 5) {
      queryString += ";";

      if (not query.exec(queryString)) {
        qDebug() << "Query: " << queryString;
        qDebug() << "ERROR: " << query.lastError().text();
        return false;
      }
    }
  }

  if (QSqlError().type() != QSqlError::NoError) {
    qDebug() << "Ocorreu algum erro: " << QSqlError();
    return false;
  }

  return true;
}

inline bool initDb() {
//  qDebug() << "Initializing mydb: ";
//  qDebug() << qApp->applicationDirPath() + "/initdb.sql";
//  qDebug() << "LOADING initdb.sql";

  if (not loadScript(qApp->applicationDirPath() + "/initdb.sql")) {
    QMessageBox::warning(0, "Aviso!", "Não carregou o script do BD.");
    return false;
  }

  QSqlQuery query;

  if (not query.exec("SELECT * FROM cep.sp LIMIT 1")) {
//    qDebug() << "LOADING cep.sql";

    if (not loadScript(qApp->applicationDirPath() + "/cep.sql")) {
      QMessageBox::warning(0, "Aviso!", "Não carregou o script do cep.");
      return false;
    }
  }

  return true;
}

#endif // INITDB_H
