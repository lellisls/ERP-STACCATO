#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrousuario.h"
#include "checkboxdelegate.h"
#include "searchdialog.h"
#include "ui_cadastrousuario.h"
#include "usersession.h"

CadastroUsuario::CadastroUsuario(QWidget *parent) : RegisterDialog("usuario", "idUsuario", parent), ui(new Ui::CadastroUsuario) {
  ui->setupUi(this);

  for (auto const *line : findChildren<QLineEdit *>()) connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") ui->table->hide();

  setupTables();
  fillCombobox();
  setupMapper();
  newRegister();

  sdUsuario = SearchDialog::usuario(this);
  connect(sdUsuario, &SearchDialog::itemSelected, this, &CadastroUsuario::viewRegisterById);
}

CadastroUsuario::~CadastroUsuario() { delete ui; }

void CadastroUsuario::setupTables() {
  modelPermissoes.setTable("usuario_has_permissao");
  modelPermissoes.setEditStrategy(SqlTableModel::OnManualSubmit);

  modelPermissoes.setHeaderData("view_tab_orcamento", "Ver Orçamentos?");
  modelPermissoes.setHeaderData("view_tab_venda", "Ver Vendas?");
  modelPermissoes.setHeaderData("view_tab_compra", "Ver Compras?");
  modelPermissoes.setHeaderData("view_tab_logistica", "Ver Logística?");
  modelPermissoes.setHeaderData("view_tab_nfe", "Ver NFe?");
  modelPermissoes.setHeaderData("view_tab_estoque", "Ver Estoque?");
  modelPermissoes.setHeaderData("view_tab_financeiro", "Ver Financeiro?");
  modelPermissoes.setHeaderData("view_tab_relatorio", "Ver Relatório?");

  modelPermissoes.setFilter("0");

  if (not modelPermissoes.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela permissões: " + modelPermissoes.lastError().text());
  }

  ui->table->setModel(&modelPermissoes);
  ui->table->hideColumn("idUsuario");
  ui->table->hideColumn("created");
  ui->table->hideColumn("lastUpdated");
  ui->table->setItemDelegate(new CheckBoxDelegate(this));
}

void CadastroUsuario::modificarUsuario() {
  ui->pushButtonBuscar->hide();
  ui->pushButtonNovoCad->hide();
  ui->pushButtonRemover->hide();

  ui->lineEditNome->setDisabled(true);
  ui->lineEditUser->setDisabled(true);
  ui->comboBoxLoja->setDisabled(true);
  ui->comboBoxTipo->setDisabled(true);
}

bool CadastroUsuario::verifyFields() {
  for (auto const &line : ui->tab->findChildren<QLineEdit *>()) {
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
    if (not query.first()) return false;
    if (not setData("passwd", query.value(0))) return false;
  }

  return true;
}

bool CadastroUsuario::viewRegister() {
  if (not RegisterDialog::viewRegister()) return false;

  ui->lineEditPasswd->setText("********");
  ui->lineEditPasswd_2->setText("********");

  modelPermissoes.setFilter("idUsuario = " + data("idUsuario").toString());

  for (int col = 0; col < ui->table->model()->columnCount(); ++col) ui->table->openPersistentEditor(0, col);

  return true;
}

void CadastroUsuario::fillCombobox() {
  QSqlQuery query("SELECT descricao, idLoja FROM loja");

  while (query.next()) ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja"));

  ui->comboBoxLoja->setCurrentValue(UserSession::idLoja());
}

void CadastroUsuario::on_pushButtonCadastrar_clicked() { save(); }

void CadastroUsuario::on_pushButtonAtualizar_clicked() { save(); }

void CadastroUsuario::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroUsuario::on_pushButtonRemover_clicked() { remove(); }

void CadastroUsuario::on_pushButtonBuscar_clicked() { sdUsuario->show(); }

bool CadastroUsuario::cadastrar() {
  currentRow = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) {
    error = "Linha -1 usuário: " + QString::number(isUpdate) + "\nMapper: " + QString::number(mapper.currentIndex()) + "\nModel: " + QString::number(model.rowCount());
    return false;
  }

  if (not isUpdate and not model.insertRow(currentRow)) return false;

  if (not savingProcedures()) return false;

  for (int column = 0; column < model.rowCount(); ++column) {
    const QVariant dado = model.data(currentRow, column);
    if (dado.type() == QVariant::String) {
      if (not model.setData(currentRow, column, dado.toString().toUpper())) return false;
    }
  }

  if (not model.submitAll()) {
    error = "Erro salvando dados na tabela " + model.tableName() + ": " + model.lastError().text();
    return false;
  }

  primaryId = data(currentRow, primaryKey).isValid() ? data(currentRow, primaryKey).toString() : getLastInsertId().toString();

  if (primaryId.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "primaryId está vazio!");
    return false;
  }

  if (not isUpdate) {
    QSqlQuery query;
    query.prepare("CREATE USER :user@'%' IDENTIFIED BY '1234'");
    query.bindValue(":user", ui->lineEditUser->text().toLower());

    if (not query.exec()) {
      error = "Erro criando usuário do banco de dados: " + query.lastError().text();
      return false;
    }

    query.prepare("GRANT ALL PRIVILEGES ON *.* TO :user@'%' WITH GRANT OPTION");
    query.bindValue(":user", ui->lineEditUser->text().toLower());

    if (not query.exec()) {
      error = "Erro guardando privilégios do usuário do banco de dados: " + query.lastError().text();
      return false;
    }

    QSqlQuery("FLUSH PRIVILEGES").exec();

    //

    const int row2 = modelPermissoes.rowCount();
    modelPermissoes.insertRow(row2);

    modelPermissoes.setData(row2, "idUsuario", primaryId);

    if (not modelPermissoes.submitAll()) {
      error = "Erro salvando permissões: " + modelPermissoes.lastError().text();
      return false;
    }
  }

  if (isUpdate) {
    if (not modelPermissoes.submitAll()) {
      error = "Erro salvando permissões: " + modelPermissoes.lastError().text();
      return false;
    }
  }

  return true;
}

bool CadastroUsuario::save() {
  if (not verifyFields()) return false;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  successMessage();

  return true;
}

void CadastroUsuario::successMessage() { QMessageBox::information(this, "Aviso!", isUpdate ? "Cadastro atualizado!" : "Usuário cadastrado com sucesso!"); }

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

// TODO: 1colocar permissoes padroes para cada tipo de usuario
