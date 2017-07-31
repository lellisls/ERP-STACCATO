#include <QFileDialog>
#include <QSqlError>

#include "sendmail.h"
#include "smtp.h"
#include "ui_sendmail.h"
#include "usersession.h"

SendMail::SendMail(QWidget *parent, const QString &arquivo, const QString &fornecedor) : QDialog(parent), fornecedor(fornecedor), ui(new Ui::SendMail) {
  // TODO: colocar arquivo como vetor de strings para multiplos anexos
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  files.append(arquivo);

  ui->lineEditAnexo->setText(arquivo);

  const QFileInfo info(arquivo);

  ui->lineEditTitulo->setText("PEDIDO " + info.baseName());

  QSqlQuery query;
  query.prepare("SELECT email, contatoNome FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", fornecedor);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando email do fornecedor: " + query.lastError().text());
  }

  QString representante;

  if (query.first()) {
    representante = query.value("contatoNome").toString();
    QStringList list = representante.split(" ");

    for (int i = 0; i < list.size(); ++i) {
      list[i] = list[i].toLower();
      list[i][0] = list[i][0].toUpper();
    }

    representante = list.join(" ");

    do {
      ui->comboBoxDest->addItem(query.value("email").toString());
    } while (query.next());
  }

  // TODO: dont hardcode this
  // TODO:__project public code
  ui->textEdit->setHtml(
      R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd"><html><head><meta name="qrichtext" content="1" /><style type="text/css">p, li { white-space: pre-wrap; }</style></head><body style=" font-family:'MS Shell Dlg 2'; font-size:8pt; font-weight:400; font-style:normal;"><p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;">)" +
      QString(QTime::currentTime().hour() > 12 ? "Boa tarde" : "Bom dia") + " prezado(a) " + (representante.isEmpty() ? "parceiro(a)" : representante) +
      R"(;</span></p><p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style=" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;">Segue pedido, aguardo espelho como confirmação e previsão de disponibilidade/ coleta do mesmo.</span></p><p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">Grato!</p><p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">   Att.</p></body></html>)");

  ui->textEdit->document()->addResource(QTextDocument::ImageResource, QUrl("cid:assinatura.png@gmail.com"), QImage("://assinatura conrado.png"));
  ui->textEdit->append(R"(<img src="cid:assinatura.png@gmail.com" />)");

  ui->lineEditServidor->setText(UserSession::settings("User/servidorSMTP").toString());
  ui->lineEditEmail->setText(UserSession::settings("User/emailCompra").toString());
  ui->lineEditPorta->setText(UserSession::settings("User/portaSMTP").toString());
  ui->lineEditPasswd->setText(UserSession::settings("User/emailSenha").toString());
  ui->lineEditCopia->setText(UserSession::settings("User/emailCopia").toString());

  progress = new QProgressDialog("Enviando...", "Cancelar", 0, 0, this);
  progress->setCancelButton(nullptr);
  progress->reset();
}

SendMail::~SendMail() { delete ui; }

void SendMail::on_pushButtonBuscar_clicked() {
  files.clear();

  files = QFileDialog::getOpenFileNames(this, "Abrir anexos", QDir::homePath());

  QString fileListString;

  for (auto const &file : files) fileListString.append(R"(")" + QFileInfo(file).fileName() + R"(" )");

  ui->lineEditAnexo->setText(fileListString);
}

void SendMail::on_pushButtonEnviar_clicked() {
  progress->show();

  UserSession::setSettings("User/servidorSMTP", ui->lineEditServidor->text());
  UserSession::setSettings("User/portaSMTP", ui->lineEditPorta->text());
  UserSession::setSettings("User/emailCompra", ui->lineEditEmail->text());
  UserSession::setSettings("User/emailSenha", ui->lineEditPasswd->text());

  Smtp *smtp = new Smtp(ui->lineEditEmail->text(), ui->lineEditPasswd->text(), ui->lineEditServidor->text(), ui->lineEditPorta->text().toInt());
  connect(smtp, &Smtp::status, this, &SendMail::mailSent);

  smtp->sendMail(ui->lineEditEmail->text(), ui->comboBoxDest->currentText(), ui->lineEditCopia->text(), ui->lineEditTitulo->text(), ui->textEdit->toHtml(), files);
}

void SendMail::mailSent(const QString &status) {
  progress->cancel();
  status == "Message sent" ? successStatus() : failureStatus(status);
}

void SendMail::successStatus() {
  QMessageBox::information(nullptr, tr("Qt Simple SMTP client"), tr("Mensagem enviada!"));

  QDialog::accept();
}

void SendMail::failureStatus(const QString &status) { QMessageBox::critical(nullptr, tr("Qt Simple SMTP client"), "Ocorreu erro: " + status); }
