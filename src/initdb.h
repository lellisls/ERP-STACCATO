#ifndef INITDB_H
#define INITDB_H

#include <QDebug>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

bool loadScript(const QString &filename) {
  //  qDebug() << "LOADING " << filename << ".";
  QFile file(filename);

  if (not file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "file error: " << file.errorString();
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
  QStringList queries = script.split(QChar(';'));

  foreach (QString queryString, queries) {
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
  if (not loadScript(qApp->applicationDirPath() + "/initdb.sql")) {
    QMessageBox::warning(0, "Aviso!", "Não carregou o script do BD.");
    return false;
  }

  QSqlQuery query;

  if (not query.exec("SELECT log_logradouro.log_tipo_logradouro, log_logradouro.log_no as logradouro, log_bairro.bai_no as bairro, log_localidade.loc_no as cidade, log_localidade.ufe_sg as uf, log_logradouro.cep FROM cep.`log_logradouro`, cep.`log_localidade`, cep.`log_bairro` WHERE log_logradouro.loc_nu_sequencial = log_localidade.loc_nu_sequencial AND log_logradouro.bai_nu_sequencial_ini = log_bairro.bai_nu_sequencial AND log_logradouro.cep = 12245500;")) {
    if (not loadScript(qApp->applicationDirPath() + "/cep.sql")) {
      QMessageBox::warning(0, "Aviso!", "Não carregou o script do cep.");
      return false;
    }
  }

  return true;
}

#endif // INITDB_H
