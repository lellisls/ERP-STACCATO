#ifndef INITDB_H
#define INITDB_H

#include <QDebug>
#include <QFile>
#include <QtSql>
#include <QMessageBox>

QSqlError loadScript(const QString &filename) {
  qDebug() << "LOADING " << filename;
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could not open script!";
    return QSqlError();
  }

  QTextStream stream(&file);
  stream.setLocale(QLocale::Portuguese);
  stream.setCodec("UTF-8");
  QString script;
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (!line.startsWith("--") && line.size() > 5) {
      script += line + "\n";
    }
  }
  //  qDebug() << script;

  QSqlQuery query;
  QStringList queryes = script.split(QChar(';'));
  foreach (QString queryString, queryes) {
    if (queryString.size() > 5) {
      queryString += ";";
      if (!query.exec(queryString)) {
        qDebug() << "Query: " << queryString;
        qDebug() << "ERROR: " << query.lastError().text();
        return query.lastError();
      }
    }
  }
  return QSqlError();
}

inline QSqlError initDb() {
  qDebug() << "Initializing mydb: ";
  qDebug() << qApp->applicationDirPath() + "/initdb.sql";
  QSqlError error = loadScript(qApp->applicationDirPath() + "/initdb.sql");
  if (error.type() != QSqlError::NoError) {
    return error;
  }
  QSqlQuery query;
  if (!query.exec("SELECT * FROM cep . sp LIMIT 1")) {
    qDebug() << "LOADING cep.sql";
    error = loadScript(qApp->applicationDirPath() + "/cep.sql");
    if (error.type() != QSqlError::NoError) {
      return error;
    }
  }
  return (QSqlDatabase::database().lastError());
}

#endif // INITDB_H
