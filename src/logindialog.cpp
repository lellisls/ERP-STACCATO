#include <QDate>
#include <QMessageBox>
#include <QSqlError>

#include "logindialog.h"
#include "ui_logindialog.h"
#include "usersession.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
  ui->setupUi(this);

  setWindowTitle("ERP Login");
  setWindowModality(Qt::WindowModal);

  ui->lineEditUser->setFocus();

  if (UserSession::settingsContains("User/lastuser")) {
    ui->lineEditUser->setText(UserSession::settings("User/lastuser").toString());
    ui->lineEditPass->setFocus();
  }

  ui->lineEditHostname->setText(UserSession::settings("Login/hostname").toString());
  ui->checkBoxHomologacao->setChecked(UserSession::settings("Login/homologacao").toBool());

  ui->labelHostname->hide();
  ui->lineEditHostname->hide();
  ui->checkBoxHomologacao->hide();

  adjustSize();

  accept();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::on_pushButtonConfig_clicked() {
  ui->labelHostname->setVisible(not ui->labelHostname->isVisible());
  ui->lineEditHostname->setVisible(not ui->lineEditHostname->isVisible());
  ui->checkBoxHomologacao->setVisible(not ui->checkBoxHomologacao->isVisible());
  adjustSize();
}

bool LoginDialog::dbConnect() {
  if (not QSqlDatabase::drivers().contains("QMYSQL")) {
    QMessageBox::critical(this, "Não foi possível carregar o banco de dados.",
                          "Este aplicativo requer o driver QMYSQL.");
    exit(1);
  }

  QSqlDatabase db = QSqlDatabase::contains() ? QSqlDatabase::database() : QSqlDatabase::addDatabase("QMYSQL");

  db.setHostName(UserSession::settings("Login/hostname").toString());
  db.setUserName(ui->lineEditUser->text().toLower());
  db.setPassword("1234");
  db.setDatabaseName("mysql");

  db.setConnectOptions("CLIENT_COMPRESS=1;MYSQL_OPT_RECONNECT=1;MYSQL_OPT_CONNECT_TIMEOUT=5;MYSQL_OPT_READ_TIMEOUT=5;"
                       "MYSQL_OPT_WRITE_TIMEOUT=5");

  if (not db.open()) {
    QString message;

    switch (db.lastError().number()) {
      case 1045:
        message = "Verifique se o usuário e senha do banco de dados estão corretos.";
        break;
      case 2002:
        message = "Verifique se o servidor está ligado, e acessível pela rede.";
        break;
      case 2003:
        message = "Verifique se o servidor está ligado, e acessível pela rede.";
        break;
      case 2005:
        message = "Verifique se o IP do servidor foi escrito corretamente.";
        break;
      default:
        message = "Erro conectando no banco de dados: " + db.lastError().text();
        break;
    }

    QMessageBox::critical(this, "Erro: Banco de dados inacessível!", message);

    return false;
  }

  QSqlQuery query = db.exec("SHOW SCHEMAS");
  bool hasMydb = false;

  while (query.next()) {
    if (query.value(0).toString() == "mydb") hasMydb = true;
  }

  if (not hasMydb) {
    QMessageBox::critical(
          this, "Erro!",
          "Não encontrou as tabelas do bando de dados, verifique se o servidor está funcionando corretamente.");
    return false;
  }

  db.close();

  db.setDatabaseName(ui->checkBoxHomologacao->isChecked() ? "mydb_test" : "mydb");

  if (not db.open()) {
    QMessageBox::critical(this, "Erro", "Erro conectando no banco de dados: " + db.lastError().text());
    return false;
  }

  if (not query.exec("SELECT lastInvalidated FROM maintenance") or not query.first()) {
    QMessageBox::critical(this, "Erro", "Erro verificando lastInvalidated: " + query.lastError().text());
    return false;
  }

  if (query.value("lastInvalidated").toDate() < QDate::currentDate()) {
    if (not query.exec("CALL invalidate_expired()")) {
      QMessageBox::critical(this, "Erro!", "Erro executando InvalidarExpirados: " + query.lastError().text());
      return false;
    }

    if (not query.exec("UPDATE maintenance SET lastInvalidated = '" + QDate::currentDate().toString("yyyy-MM-dd") +
                       "' WHERE id = 1")) {
      QMessageBox::critical(this, "Erro", "Erro atualizando lastInvalidated: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_orcamento_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro executando update_orcamento_status: " + query.lastError().text());
    return false;
  }

  return true;
}

void LoginDialog::on_pushButtonLogin_clicked() {
  UserSession::setSettings("Login/hostname", ui->lineEditHostname->text());
  UserSession::setSettings("Login/homologacao", ui->checkBoxHomologacao->isChecked());

  if (not dbConnect()) return;

  if (not UserSession::login(ui->lineEditUser->text(), ui->lineEditPass->text())) {
    QMessageBox::critical(this, "Erro!", "Login inválido!");
    ui->lineEditPass->setFocus();

    return;
  }

  accept();

  UserSession::setSettings("User/lastuser", ui->lineEditUser->text());
}
