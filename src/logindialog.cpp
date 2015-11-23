#include <QMessageBox>
#include <QSqlError>

#include "logindialog.h"
#include "ui_logindialog.h"
#include "usersession.h"
#include "loginconfig.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
  ui->setupUi(this);

  setWindowTitle("ERP Login");
  setWindowModality(Qt::WindowModal);

  ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Login");
  ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancelar");

  ui->lineEditUser->setFocus();

  if (settingsContains("User/lastuser")) {
    ui->lineEditUser->setText(settings("User/lastuser").toString());
    ui->lineEditPass->setFocus();
  }

  readSettings();

  accept();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::readSettings() {
  hostname = settings("Login/hostname").toString();
  username = settings("Login/username").toString();
  password = settings("Login/password").toString();
  port = settings("Login/port").toString();
  homologacao = settings("Login/homologacao").toBool();
}

void LoginDialog::on_buttonBox_accepted() {
  verify();

  setSettings("User/lastuser", ui->lineEditUser->text());
}

void LoginDialog::on_buttonBox_rejected() { reject(); }

void LoginDialog::verify() {
  if (not dbConnect()) {
    return;
  }

  if (not UserSession::login(ui->lineEditUser->text(), ui->lineEditPass->text())) {
    QMessageBox::critical(this, "Erro!", "Login inválido!");
    ui->lineEditPass->setFocus();
    return;
  }

  accept();
}

void LoginDialog::on_pushButtonConfig_clicked() {
  LoginConfig *config = new LoginConfig(this);
  config->show();
}

QVariant LoginDialog::settings(const QString &key) const { return UserSession::getSettings(key); }

void LoginDialog::setSettings(const QString &key, const QVariant &value) const { UserSession::setSettings(key, value); }

bool LoginDialog::settingsContains(const QString &key) const { return UserSession::settingsContains(key); }

bool LoginDialog::dbConnect() {
  if (not QSqlDatabase::drivers().contains("QMYSQL")) {
    QMessageBox::critical(this, "Não foi possível carregar o banco de dados.",
                          "Este aplicativo requer o driver QMYSQL.");
    exit(1);
  }

  QSqlDatabase db = QSqlDatabase::contains() ? QSqlDatabase::database() : QSqlDatabase::addDatabase("QMYSQL");

  db.setHostName(hostname);
  db.setUserName(username);
  db.setPassword(password);
  db.setDatabaseName("mysql");

  db.setConnectOptions("CLIENT_COMPRESS=1;MYSQL_OPT_RECONNECT=1");

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
    if (query.value(0).toString() == "mydb") {
      hasMydb = true;
    }
  }

  if (not hasMydb) {
    QMessageBox::critical(
          this, "Erro!",
          "Não encontrou as tabelas do bando de dados, verifique se o servidor está funcionando corretamente.");
    return false;
  }

  db.close();

  db.setDatabaseName(homologacao ? "mydb_test" : "mydb");

  if (not db.open()) {
    QMessageBox::critical(this, "Erro", "Erro conectando no banco de dados: " + db.lastError().text());
    return false;
  }

  if (not query.exec("CALL invalidate_expired()")) {
    QMessageBox::critical(this, "Erro!", "Erro executando InvalidarExpirados: " + query.lastError().text());
    return false;
  }

  return true;
}
