#include <QCheckBox>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrousuario.h"
#include "searchdialog.h"
#include "ui_cadastrousuario.h"
#include "usersession.h"

CadastroUsuario::CadastroUsuario(QWidget *parent)
  : RegisterDialog("usuario", "idUsuario", parent), ui(new Ui::CadastroUsuario) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  setupTablePermissoes();
  fillCombobox();

  ui->tablePermissoes->setEnabled(false);
  ui->tablePermissoes->setToolTip("Função indisponível nesta versão!");
  ui->tablePermissoes->resizeColumnsToContents();

  ui->tabWidget->setTabEnabled(1, false);

  setupMapper();
  newRegister();
}

CadastroUsuario::~CadastroUsuario() { delete ui; }

void CadastroUsuario::modificarUsuario() {
  ui->pushButtonBuscar->hide();
  ui->pushButtonNovoCad->hide();
  ui->pushButtonRemover->hide();

  ui->lineEditNome->setDisabled(true);
  ui->lineEditUser->setDisabled(true);
  ui->comboBoxLoja->setDisabled(true);
  ui->comboBoxTipo->setDisabled(true);
}

void CadastroUsuario::setupTablePermissoes() {
  ui->tablePermissoes->resizeColumnsToContents();
  ui->tablePermissoes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tablePermissoes->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  for (int row = 0, rowCount = ui->tablePermissoes->rowCount(); row < rowCount; ++row) {
    for (int column = 0, columnCount = ui->tablePermissoes->columnCount(); column < columnCount; ++column) {
      QWidget *widget = new QWidget();
      QCheckBox *checkBox = new QCheckBox();
      QHBoxLayout *layout = new QHBoxLayout(widget);
      layout->addWidget(checkBox);
      layout->setAlignment(Qt::AlignCenter);
      layout->setContentsMargins(0, 0, 0, 0);
      widget->setLayout(layout);
      ui->tablePermissoes->setCellWidget(row, column, widget);
    }
  }
}

bool CadastroUsuario::verifyFields() {
  for (auto const &line : ui->gridLayout_2->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (ui->lineEditPasswd->text() != ui->lineEditPasswd_2->text()) {
    ui->lineEditPasswd->setFocus();
    QMessageBox::critical(this, "Erro!", "As senhas não batem!");
    return false;
  }

  return true;
}

void CadastroUsuario::clearFields() { RegisterDialog::clearFields(); }

void CadastroUsuario::setupMapper() {
  addMapping(ui->comboBoxLoja, "idLoja", "currentValue");
  addMapping(ui->comboBoxTipo, "tipo");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditUser, "user");
}

void CadastroUsuario::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroUsuario::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroUsuario::savingProcedures() {
  if (not setData("nome", ui->lineEditNome->text())) return false;
  if (not setData("idLoja", ui->comboBoxLoja->getCurrentValue())) return false;
  if (not setData("tipo", ui->comboBoxTipo->currentText())) return false;
  if (not setData("user", ui->lineEditUser->text())) return false;
  if (not setData("email", ui->lineEditEmail->text())) return false;
  if (not setData("user", ui->lineEditUser->text())) return false;

  if (ui->lineEditPasswd->text() != "********") {
    QSqlQuery query("SELECT PASSWORD('" + ui->lineEditPasswd->text() + "')");
    query.first();
    if (not setData("passwd", query.value(0))) return false;
  }

  return true;
}

bool CadastroUsuario::viewRegister(const QModelIndex &index) {
  if (not RegisterDialog::viewRegister(index)) return false;

  ui->lineEditPasswd->setText("********");
  ui->lineEditPasswd_2->setText("********");

  return true;
}

void CadastroUsuario::fillCombobox() {
  QSqlQuery query("SELECT descricao, idLoja FROM loja");

  while (query.next()) {
    ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja"));
  }

  ui->comboBoxLoja->setCurrentValue(UserSession::idLoja());
}

void CadastroUsuario::on_pushButtonCadastrar_clicked() { save(); }

void CadastroUsuario::on_pushButtonAtualizar_clicked() { update(); }

void CadastroUsuario::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroUsuario::on_pushButtonRemover_clicked() { remove(); }

void CadastroUsuario::on_pushButtonBuscar_clicked() {
  SearchDialog *sdUsuario = SearchDialog::usuario(this);
  connect(sdUsuario, &SearchDialog::itemSelected, this, &CadastroUsuario::viewRegisterById);
  sdUsuario->show();
}

bool CadastroUsuario::save() {
  if (not verifyFields()) return false;

  if (not isUpdate and not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    viewRegister(model.index(row, 0));
    return false;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Linha -1 usuário: " + QString::number(isUpdate) + "\nMapper: " +
        QString::number(mapper.currentIndex()) + "\nModel: " + QString::number(model.rowCount()));
    QSqlQuery("ROLLBACK").exec();
    viewRegister(model.index(row, 0));
    return false;
  }

  if (not isUpdate) model.insertRow(row);

  if (not savingProcedures()) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    viewRegister(model.index(row, 0));
    return false;
  }

  for (int column = 0; column < model.rowCount(); ++column) {
    QVariant dado = model.data(row, column);
    if (dado.type() == QVariant::String) model.setData(row, column, dado.toString().toUpper());
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados na tabela " + model.tableName() + ": " + model.lastError().text());
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    viewRegister(model.index(row, 0));
    return false;
  }

  if (not isUpdate) {
    QSqlQuery query;
    query.prepare("CREATE USER :user@'%' IDENTIFIED BY '1234'");
    query.bindValue(":user", ui->lineEditUser->text().toLower());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro criando usuário do banco de dados: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      viewRegister(model.index(row, 0));
      return false;
    }

    query.prepare("GRANT ALL PRIVILEGES ON *.* TO :user@'%' WITH GRANT OPTION");
    query.bindValue(":user", ui->lineEditUser->text().toLower());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro guardando privilégios do usuário do banco de dados: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      viewRegister(model.index(row, 0));
      return false;
    }

    QSqlQuery("FLUSH PRIVILEGES").exec();
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegister(model.index(row, 0));

  if (not silent) successMessage();

  return true;
}

void CadastroUsuario::successMessage() { QMessageBox::information(this, "Aviso!", "Usuário atualizado com sucesso!"); }

void CadastroUsuario::on_lineEditUser_textEdited(const QString &text) {
  QSqlQuery query;
  query.prepare("SELECT idUsuario FROM usuario WHERE user = :user");
  query.bindValue(":user", text);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando usuário: " + query.lastError().text());
    return;
  }

  if (query.first()) {
    QMessageBox::critical(this, "Erro!", "Nome de usuário já existe!");
    return;
  }
}
