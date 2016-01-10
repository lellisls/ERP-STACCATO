#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "doubledelegate.h"
#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "usersession.h"

SearchDialog::SearchDialog(const QString &title, const QString &table, const QStringList &indexes,
                           const QString &filter, QWidget *parent)
  : QDialog(parent), ui(new Ui::SearchDialog) {
  ui->setupUi(this);

  setWindowTitle(title);
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela " + table + ": " + model.lastError().text());
    return;
  }

  ui->tableBusca->setModel(&model);
  ui->tableBusca->setItemDelegate(new DoubleDelegate(this));

  if (indexes.isEmpty()) {
    ui->lineEditBusca->hide();
    ui->labelBusca->hide();
    ui->tableBusca->setFocus();
  } else {
    this->indexes = indexes;
    textKeys.append(indexes.first());
    primaryKey = indexes.first();
    ui->lineEditBusca->setFocus();
  }

  ui->radioButtonProdAtivos->hide();
  ui->radioButtonProdDesc->hide();
  ui->lineEditBusca->setFocus();
}

SearchDialog::~SearchDialog() { delete ui; }

void SearchDialog::on_lineEditBusca_textChanged(const QString &text) {
  if (model.tableName() == "produto") {
    montarFiltroAtivoDesc(ui->radioButtonProdAtivos->isChecked());
    return;
  }

  if (text.isEmpty()) {
    model.setFilter(filter);
    return;
  }

  QStringList strings = text.split(" ", QString::SkipEmptyParts);

  for (auto &string : strings) {
    string.prepend("+").append("*");
  }

  QString searchFilter = "MATCH(" + indexes.join(", ") + ") AGAINST('" + strings.join(" ") + "' IN BOOLEAN MODE)";

  if (not filter.isEmpty()) searchFilter.append(" AND (" + filter + ")");

  model.setFilter(searchFilter);
}

void SearchDialog::sendUpdateMessage() {
  auto selection = ui->tableBusca->selectionModel()->selection().indexes();

  if (selection.isEmpty()) return;

  selectedId = model.data(selection.first().row(), primaryKey);

  emit itemSelected(selectedId);
}

void SearchDialog::show() {
  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela " + model.tableName() + ": " + model.lastError().text());
    return;
  }

  ui->lineEditBusca->setFocus();
  ui->lineEditBusca->clear();

  QDialog::show();

  ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::showMaximized() {
  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela " + model.tableName() + ": " + model.lastError().text());
    return;
  }

  ui->lineEditBusca->setFocus();

  QDialog::showMaximized();
  ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::on_tableBusca_doubleClicked(const QModelIndex &) {
  sendUpdateMessage();
  close();
}

void SearchDialog::setFilter(const QString &value) {
  filter = value;
  model.setFilter(filter);
}

void SearchDialog::hideColumns(const QStringList &columns) {
  for (const auto column : columns) {
    ui->tableBusca->hideColumn(column);
  }
}

void SearchDialog::on_pushButtonSelecionar_clicked() {
  sendUpdateMessage();
  close();
}

void SearchDialog::setTextKeys(const QStringList &value) { textKeys = value; }

void SearchDialog::setPrimaryKey(const QString &value) { primaryKey = value; }

QString SearchDialog::getText(const QVariant &index) {
  QString queryText;

  for (const auto key : textKeys) {
    queryText += queryText.isEmpty() ? key : ", " + key;
  }

  queryText =
      "SELECT " + queryText + " FROM " + model.tableName() + " WHERE " + primaryKey + " = '" + index.toString() + "'";

  QSqlQuery query(queryText);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro na query getText: " + query.lastError().text());
    return QString();
  }

  if (model.tableName().contains("endereco")) {
    if (query.value("descricao").toString() == "Não há/Retira") return "Não há/Retira";
  }

  QString res;

  for (const auto key : textKeys) {
    if (query.value(key).isValid()) res += (res.isEmpty() ? "" : " - ") + query.value(key).toString();
  }

  return res;
}

void SearchDialog::setHeaderData(const QString &column, const QString &value) { model.setHeaderData(column, value); }

SearchDialog *SearchDialog::cliente(QWidget *parent) {
  SearchDialog *sdCliente = new SearchDialog("Buscar Cliente", "cliente", {"nome_razao", "nomeFantasia", "cpf", "cnpj"},
                                             "desativado = FALSE", parent);

  sdCliente->setPrimaryKey("idCliente");
  sdCliente->setTextKeys({"nome_razao"});

  sdCliente->hideColumns({"idCliente", "rg", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca",
                          "idEnderecoEntrega", "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto",
                          "desativado"});

  sdCliente->setHeaderData("pfpj", "Tipo");
  sdCliente->setHeaderData("nome_razao", "Cliente");
  sdCliente->setHeaderData("cpf", "CPF");
  sdCliente->setHeaderData("cnpj", "CNPJ");
  sdCliente->setHeaderData("dataNasc", "Data Nasc.");
  sdCliente->setHeaderData("contatoNome", "Nome - Contato");
  sdCliente->setHeaderData("contatoCPF", "CPF - Contato");
  sdCliente->setHeaderData("contatoApelido", "Apelido - Contato");
  sdCliente->setHeaderData("contatoRG", "RG - Contato");
  sdCliente->setHeaderData("nomeFantasia", "Fantasia/Apelido");
  sdCliente->setHeaderData("tel", "Tel.");
  sdCliente->setHeaderData("telCel", "Tel. Cel.");
  sdCliente->setHeaderData("telCom", "Tel. Com.");
  sdCliente->setHeaderData("idNextel", "id Nextel");
  sdCliente->setHeaderData("nextel", "Nextel");
  sdCliente->setHeaderData("email", "E-mail");

  return sdCliente;
}

SearchDialog *SearchDialog::loja(QWidget *parent) {
  SearchDialog *sdLoja = new SearchDialog("Buscar Loja", "loja", {"descricao, nomeFantasia, razaoSocial"},
                                          "idLoja != 1 AND desativado = FALSE", parent);

  sdLoja->setPrimaryKey("idLoja");
  sdLoja->setTextKeys({"nomeFantasia"});

  sdLoja->hideColumns({"idLoja", "idEndereco", "codUF", "desativado"});

  sdLoja->setHeaderData("descricao", "Descrição");
  sdLoja->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdLoja->setHeaderData("razaoSocial", "Razão Social");
  sdLoja->setHeaderData("tel", "Tel.");
  sdLoja->setHeaderData("inscEstadual", "Insc. Est.");
  sdLoja->setHeaderData("sigla", "Sigla");
  sdLoja->setHeaderData("cnpj", "CNPJ");
  sdLoja->setHeaderData("porcentagemFrete", "% Frete");
  sdLoja->setHeaderData("valorMinimoFrete", "R$ Mínimo Frete");

  return sdLoja;
}

SearchDialog *SearchDialog::produto(QWidget *parent) {
  SearchDialog *sdProd = new SearchDialog(
                           "Buscar Produto", "produto", {"fornecedor", "descricao", "colecao", "codcomercial"}, "idProduto = 0", parent);

  sdProd->setPrimaryKey("idProduto");
  sdProd->setTextKeys({"descricao"});

  sdProd->hideColumns({"idProduto", "idFornecedor", "cst", "icms", "custo", "ipi", "markup", "comissao", "origem",
                       "descontinuado", "temLote", "observacoes", "codBarras", "qtdPallet", "st", "desativado", "cfop",
                       "ncm", "ncmEx", "atualizarTabelaPreco", "representacao"});

  for (int column = 0; column < sdProd->model.columnCount(); ++column) {
    if (sdProd->model.record().fieldName(column).endsWith("Upd")) sdProd->ui->tableBusca->setColumnHidden(column, true);
  }

  sdProd->setHeaderData("fornecedor", "Fornecedor");
  sdProd->setHeaderData("descricao", "Descrição");
  sdProd->setHeaderData("estoque", "Estoque");
  sdProd->setHeaderData("un", "Un.");
  sdProd->setHeaderData("un2", "Un.2");
  sdProd->setHeaderData("colecao", "Coleção");
  sdProd->setHeaderData("tipo", "Tipo");
  sdProd->setHeaderData("m2cx", "M/Cx.");
  sdProd->setHeaderData("pccx", "Pç./Cx.");
  sdProd->setHeaderData("kgcx", "Kg./Cx.");
  sdProd->setHeaderData("formComercial", "Form. Com.");
  sdProd->setHeaderData("codComercial", "Cód. Com.");
  sdProd->setHeaderData("precoVenda", "R$");
  sdProd->setHeaderData("validade", "Validade");
  sdProd->setHeaderData("ui", "UI");

  sdProd->ui->radioButtonProdAtivos->show();
  sdProd->ui->radioButtonProdDesc->show();
  sdProd->ui->radioButtonProdAtivos->setChecked(true);

  return sdProd;
}

SearchDialog *SearchDialog::fornecedor(QWidget *parent) {
  SearchDialog *sdFornecedor = new SearchDialog(
                                 "Buscar Fornecedor", "fornecedor", {"nome_razao", "nomeFantasia", "cpf", "cnpj"}, "desativado = FALSE", parent);

  sdFornecedor->setPrimaryKey("idFornecedor");
  sdFornecedor->setTextKeys({"nomeFantasia", "razaoSocial"});

  sdFornecedor->hideColumns({"idFornecedor", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca",
                             "idEnderecoEntrega", "tel", "telCel", "telCom", "idNextel", "nextel", "email",
                             "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "desativado", "representacao"});

  sdFornecedor->setHeaderData("razaoSocial", "Razão Social");
  sdFornecedor->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdFornecedor->setHeaderData("contatoNome", "Nome do Contato");
  sdFornecedor->setHeaderData("cnpj", "CNPJ");
  sdFornecedor->setHeaderData("contatoCPF", "CPF do Contato");
  sdFornecedor->setHeaderData("contatoApelido", "Apelido do Contato");
  sdFornecedor->setHeaderData("contatoRG", "RG do Contato");

  return sdFornecedor;
}

SearchDialog *SearchDialog::transportadora(QWidget *parent) {
  SearchDialog *sdTransportadora = new SearchDialog("Buscar Transportadora", "transportadora",
  {"razaoSocial", "nomeFantasia"}, "desativado = FALSE", parent);

  sdTransportadora->setPrimaryKey("idTransportadora");
  sdTransportadora->setTextKeys({"nomeFantasia"});

  sdTransportadora->hideColumns({"idTransportadora", "idEndereco", "desativado"});

  sdTransportadora->setHeaderData("razaoSocial", "Razão Social");
  sdTransportadora->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdTransportadora->setHeaderData("cnpj", "CNPJ");
  sdTransportadora->setHeaderData("inscEstadual", "Insc. Est.");
  sdTransportadora->setHeaderData("placaVeiculo", "Placa");
  sdTransportadora->setHeaderData("antt", "ANTT");
  sdTransportadora->setHeaderData("tel", "Tel.");

  return sdTransportadora;
}

SearchDialog *SearchDialog::usuario(QWidget *parent) {
  SearchDialog *sdUsuario =
      new SearchDialog("Buscar Usuário", "usuario", {"nome, tipo"}, "usuario.desativado = FALSE", parent);

  sdUsuario->setPrimaryKey("idUsuario");
  sdUsuario->setTextKeys({"nome"});

  sdUsuario->hideColumns({"idUsuario", "user", "passwd", "desativado"});

  sdUsuario->setHeaderData("idLoja", "Loja");
  sdUsuario->setHeaderData("tipo", "Função");
  sdUsuario->setHeaderData("nome", "Nome");
  sdUsuario->setHeaderData("sigla", "Sigla");
  sdUsuario->setHeaderData("email", "E-mail");

  sdUsuario->model.setRelation(sdUsuario->model.fieldIndex("idLoja"), QSqlRelation("loja", "idLoja", "descricao"));

  return sdUsuario;
}

SearchDialog *SearchDialog::vendedor(QWidget *parent) {
  int idLoja = UserSession::getLoja();

  QString filtro = (idLoja == 1) ? "" : " AND idLoja = " + QString::number(idLoja);

  SearchDialog *sdVendedor = new SearchDialog("Buscar Vendedor", "usuario", {"nome, tipo"},
                                              "desativado = FALSE AND tipo = 'VENDEDOR'" + filtro, parent);

  sdVendedor->setPrimaryKey("idUsuario");
  sdVendedor->setTextKeys({"nome"});

  sdVendedor->hideColumns({"idUsuario", "idLoja", "user", "passwd", "desativado"});

  sdVendedor->setHeaderData("tipo", "Função");
  sdVendedor->setHeaderData("nome", "Nome");
  sdVendedor->setHeaderData("sigla", "Sigla");
  sdVendedor->setHeaderData("email", "E-mail");

  return sdVendedor;
}

SearchDialog *SearchDialog::enderecoCliente(QWidget *parent) {
  SearchDialog *sdEndereco = new SearchDialog("Buscar Endereço", "cliente_has_endereco", {}, "idEndereco = 1", parent);

  sdEndereco->setPrimaryKey("idEndereco");
  sdEndereco->setTextKeys({"descricao", "logradouro", "numero", "bairro", "cidade", "uf"});

  sdEndereco->hideColumns({"idEndereco", "idCliente", "codUF", "desativado"});

  sdEndereco->setHeaderData("descricao", "Descrição");
  sdEndereco->setHeaderData("cep", "CEP");
  sdEndereco->setHeaderData("logradouro", "End.");
  sdEndereco->setHeaderData("numero", "Número");
  sdEndereco->setHeaderData("complemento", "Comp.");
  sdEndereco->setHeaderData("bairro", "Bairro");
  sdEndereco->setHeaderData("cidade", "Cidade");
  sdEndereco->setHeaderData("uf", "UF");

  return sdEndereco;
}

SearchDialog *SearchDialog::profissional(QWidget *parent) {
  SearchDialog *sdProfissional =
      new SearchDialog("Buscar Profissional", "profissional", {"nome_razao, tipoProf"}, "desativado = FALSE", parent);

  sdProfissional->setPrimaryKey("idProfissional");
  sdProfissional->setTextKeys({"nome_razao"});

  sdProfissional->hideColumns({"idProfissional", "rg", "inscEstadual", "contatoNome", "contatoCPF", "contatoApelido",
                               "contatoRG", "banco", "agencia", "cc", "nomeBanco", "cpfBanco", "desativado"});

  sdProfissional->setHeaderData("pfpj", "Tipo");
  sdProfissional->setHeaderData("nome_razao", "Profissional");
  sdProfissional->setHeaderData("nomeFantasia", "Fantasia/Apelido");
  sdProfissional->setHeaderData("cpf", "CPF");
  sdProfissional->setHeaderData("cnpj", "CNPJ");
  sdProfissional->setHeaderData("tel", "Tel.");
  sdProfissional->setHeaderData("telCel", "Tel. Cel.");
  sdProfissional->setHeaderData("telCom", "Tel. Com.");
  sdProfissional->setHeaderData("idNextel", "id Nextel");
  sdProfissional->setHeaderData("nextel", "Nextel");
  sdProfissional->setHeaderData("email", "E-mail");
  sdProfissional->setHeaderData("tipoProf", "Profissão");

  return sdProfissional;
}

void SearchDialog::montarFiltroAtivoDesc(const bool &ativo) {
  ui->lineEditBusca->setFocus();

  const QString text = ui->lineEditBusca->text();

  if (text.isEmpty()) {
    setFilter("idProduto = 0");
    return;
  }

  QStringList strings = text.split(" ", QString::SkipEmptyParts);

  for (auto &string : strings) {
    string.prepend("+").append("*");
  }

  const QString searchFilter = "MATCH(" + indexes.join(", ") + ") AGAINST('" + strings.join(" ") + "' IN BOOLEAN MODE)";

  setFilter(searchFilter + " AND descontinuado = " + (ativo ? "FALSE" : "TRUE") + " AND desativado = FALSE" +
            representacao);

  if (isVisible()) ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::on_tableBusca_entered(const QModelIndex &) { ui->tableBusca->resizeColumnsToContents(); }

void SearchDialog::setRepresentacao(const QString &value) { representacao = value; }

void SearchDialog::on_radioButtonProdAtivos_toggled(const bool &) { montarFiltroAtivoDesc(true); }

void SearchDialog::on_radioButtonProdDesc_toggled(const bool &) { montarFiltroAtivoDesc(false); }
