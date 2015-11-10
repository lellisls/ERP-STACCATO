#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "doubledelegate.h"

typedef QPair<QString, QString> Pair;

SearchDialog::SearchDialog(QString title, QString table, QStringList indexes, QString filter, QWidget *parent)
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
  ui->tableBusca->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableBusca->horizontalHeader()->setResizeContentsPrecision(0);

  ui->tableBusca->setItemDelegate(new DoubleDelegate(this));

  if (indexes.isEmpty()) {
    ui->lineEditBusca->hide();
    ui->labelBusca->hide();
    ui->tableBusca->setFocus();
  } else {
    this->indexes = indexes;
    textKeys.append(indexes.front());
    primaryKey = indexes.front();
    ui->lineEditBusca->setFocus();
  }

  ui->groupBoxFiltrosProduto->hide();
  ui->lineEditBusca->setFocus();
}

SearchDialog::~SearchDialog() { delete ui; }

void SearchDialog::on_lineEditBusca_textChanged(const QString &text) {
  if (model.tableName() == "produto") {
    montarFiltroAtivoDesc(ui->radioButtonProdAtivos->isChecked());
    return;
  }

  if (text.isEmpty()) {
    setFilter(filter);
    return;
  }

  QStringList strings = text.split(" ", QString::SkipEmptyParts);

  for (auto &string : strings) {
    string.prepend("+").append("*");
  }

  QString searchFilter = "MATCH(" + indexes.join(", ") + ") AGAINST('" + strings.join(" ") + "' IN BOOLEAN MODE)";

  if (not filter.isEmpty()) {
    searchFilter.append(" AND (" + filter + ")");
  }

  model.setFilter(searchFilter);
}

void SearchDialog::sendUpdateMessage() {
  auto selection = ui->tableBusca->selectionModel()->selection().indexes();

  if (selection.isEmpty()) {
    return;
  }

  selectedId = model.data(selection.first().row(), primaryKey);

  emit itemSelected(selectedId);
}

void SearchDialog::show() {
  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela " + model.tableName() + ": " + model.lastError().text());
    return;
  }

  ui->lineEditBusca->setFocus();

  QDialog::show();

  ui->lineEditBusca->clear();

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

void SearchDialog::on_tableBusca_doubleClicked(const QModelIndex &index) {
  Q_UNUSED(index);

  sendUpdateMessage();
  ui->lineEditBusca->clear();
  close();
}

QString SearchDialog::getFilter() const { return filter; }

void SearchDialog::setFilter(const QString &value) {
  filter = value;
  model.setFilter(filter);
}

void SearchDialog::hideColumns(const QStringList columns) {
  for (const auto column : columns) {
    ui->tableBusca->setColumnHidden(model.fieldIndex(column), true);
  }
}

void SearchDialog::on_pushButtonSelecionar_clicked() {
  sendUpdateMessage();
  ui->lineEditBusca->clear();
  close();
}

void SearchDialog::on_pushButtonCancelar_clicked() {
  ui->lineEditBusca->clear();
  close();
}

QStringList SearchDialog::getTextKeys() const { return textKeys; }

void SearchDialog::setTextKeys(const QStringList &value) { textKeys = value; }

QString SearchDialog::getPrimaryKey() const { return primaryKey; }

void SearchDialog::setPrimaryKey(const QString &value) { primaryKey = value; }

QString SearchDialog::getText(const QVariant index) {
  QString queryText;

  for (const auto key : textKeys) {
    queryText += queryText.isEmpty() ? key : ", " + key;
  }

  queryText =
      "SELECT " + queryText + " FROM " + model.tableName() + " WHERE " + primaryKey + " = '" + index.toString() + "';";

  QSqlQuery query(queryText);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro na query getText: " + query.lastError().text());
    return QString();
  }

  if (model.tableName().contains("endereco")) {
    if (query.value("descricao").toString() == "Não há/Retira") {
      return "Não há/Retira";
    }
  }

  QString res;

  for (const auto key : textKeys) {
    if (query.value(key).isValid()) {
      res += (res.isEmpty() ? "" : " - ") + query.value(key).toString();
    }
  }

  return res;
}

void SearchDialog::setHeaderData(const QVector<Pair> headerData) {
  for (auto const &pair : headerData) {
    model.setHeaderData(model.fieldIndex(pair.first), Qt::Horizontal, pair.second);
  }
}

SearchDialog *SearchDialog::cliente(QWidget *parent) {
  SearchDialog *sdCliente = new SearchDialog("Buscar Cliente", "cliente", {"nome_razao", "nomeFantasia", "cpf", "cnpj"},
                                             "desativado = FALSE", parent);

  sdCliente->setPrimaryKey("idCliente");
  sdCliente->setTextKeys({"nomeFantasia", "nome_razao"});

  sdCliente->hideColumns({"idCliente", "rg", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca",
                          "idEnderecoEntrega", "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto",
                          "desativado"});

  QVector<Pair> header;
  header.push_back({"pfpj", "Tipo"});
  header.push_back({"nome_razao", "Cliente"});
  header.push_back({"cpf", "CPF"});
  header.push_back({"cnpj", "CNPJ"});
  header.push_back({"dataNasc", "Data Nasc."});
  header.push_back({"contatoNome", "Nome - Contato"});
  header.push_back({"contatoCPF", "CPF - Contato"});
  header.push_back({"contatoApelido", "Apelido - Contato"});
  header.push_back({"contatoRG", "RG - Contato"});
  header.push_back({"nomeFantasia", "Fantasia/Apelido"});
  header.push_back({"tel", "Tel."});
  header.push_back({"telCel", "Tel. Cel."});
  header.push_back({"telCom", "Tel. Com."});
  header.push_back({"idNextel", "id Nextel"});
  header.push_back({"nextel", "Nextel"});
  header.push_back({"email", "E-mail"});
  sdCliente->setHeaderData(header);

  return sdCliente;
}

SearchDialog *SearchDialog::loja(QWidget *parent) {
  SearchDialog *sdLoja =
      new SearchDialog("Buscar Loja", "loja", {"descricao, nomeFantasia, razaoSocial"}, "desativado = FALSE", parent);

  sdLoja->setPrimaryKey("idLoja");
  sdLoja->setTextKeys({"nomeFantasia"});

  sdLoja->hideColumns({"idLoja", "idEndereco", "codUF", "desativado"});

  QVector<Pair> header;
  header.push_back({"descricao", "Descrição"});
  header.push_back({"nomeFantasia", "Nome Fantasia"});
  header.push_back({"razaoSocial", "Razão Social"});
  header.push_back({"tel", "Tel."});
  header.push_back({"inscEstadual", "Insc. Est."});
  header.push_back({"sigla", "Sigla"});
  header.push_back({"cnpj", "CNPJ"});
  header.push_back({"porcentagemFrete", "% Frete"});
  header.push_back({"valorMinimoFrete", "R$ Mínimo Frete"});
  sdLoja->setHeaderData(header);

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

  for (int i = 1, fieldIndex = sdProd->model.fieldIndex("descontinuadoUpd"); i <= fieldIndex; i += 2) {
    sdProd->ui->tableBusca->setColumnHidden(i, true); // this hides *Upd fields
  }

  QVector<Pair> header;
  header.push_back({"fornecedor", "Fornecedor"});
  header.push_back({"descricao", "Descrição"});
  header.push_back({"estoque", "Estoque"});
  header.push_back({"un", "Un."});
  header.push_back({"un2", "Un.2"});
  header.push_back({"colecao", "Coleção"});
  header.push_back({"tipo", "Tipo"});
  header.push_back({"m2cx", "M/Cx."});
  header.push_back({"pccx", "Pç./Cx."});
  header.push_back({"kgcx", "Kg./Cx."});
  header.push_back({"formComercial", "Form. Com."});
  header.push_back({"codComercial", "Cód. Com."});
  header.push_back({"precoVenda", "R$"});
  header.push_back({"validade", "Validade"});
  header.push_back({"ui", "UI"});
  sdProd->setHeaderData(header);

  sdProd->ui->groupBoxFiltrosProduto->show();
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
                             "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto", "desativado"});

  QVector<Pair> header;
  header.push_back({"razaoSocial", "Razão Social"});
  header.push_back({"nomeFantasia", "Nome Fantasia"});
  header.push_back({"contatoNome", "Nome do Contato"});
  header.push_back({"cnpj", "CNPJ"});
  header.push_back({"contatoCPF", "CPF do Contato"});
  header.push_back({"contatoApelido", "Apelido do Contato"});
  header.push_back({"contatoRG", "RG do Contato"});
  sdFornecedor->setHeaderData(header);

  return sdFornecedor;
}

SearchDialog *SearchDialog::transportadora(QWidget *parent) {
  SearchDialog *sdTransportadora = new SearchDialog("Buscar Transportadora", "transportadora",
  {"razaoSocial", "nomeFantasia"}, "desativado = FALSE", parent);

  sdTransportadora->setPrimaryKey("idTransportadora");
  sdTransportadora->setTextKeys({"nomeFantasia"});

  sdTransportadora->hideColumns({"idTransportadora", "idEndereco", "desativado"});

  QVector<Pair> header;
  header.push_back({"razaoSocial", "Razão Social"});
  header.push_back({"nomeFantasia", "Nome Fantasia"});
  header.push_back({"cnpj", "CNPJ"});
  header.push_back({"inscEstadual", "Insc. Est."});
  header.push_back({"placaVeiculo", "Placa"});
  header.push_back({"antt", "ANTT"});
  header.push_back({"tel", "Tel."});
  sdTransportadora->setHeaderData(header);

  return sdTransportadora;
}

SearchDialog *SearchDialog::usuario(QWidget *parent) {
  SearchDialog *sdUsuario =
      new SearchDialog("Buscar Usuário", "usuario", {"nome, tipo"}, "usuario.desativado = FALSE", parent);

  sdUsuario->setPrimaryKey("idUsuario");
  sdUsuario->setTextKeys({"nome"});

  sdUsuario->hideColumns({"idUsuario", "user", "passwd", "desativado"});

  QVector<Pair> header;
  header.push_back({"idLoja", "Loja"});
  header.push_back({"tipo", "Função"});
  header.push_back({"nome", "Nome"});
  header.push_back({"sigla", "Sigla"});
  sdUsuario->setHeaderData(header);

  sdUsuario->model.setRelation(sdUsuario->model.fieldIndex("idLoja"), QSqlRelation("loja", "idLoja", "descricao"));

  return sdUsuario;
}

SearchDialog *SearchDialog::vendedor(QWidget *parent) {
  SearchDialog *sdVendedor = new SearchDialog("Buscar Vendedor", "usuario", {"nome, tipo"},
                                              "desativado = FALSE AND tipo = 'VENDEDOR'", parent);

  sdVendedor->setPrimaryKey("idUsuario");
  sdVendedor->setTextKeys({"nome"});

  sdVendedor->hideColumns({"idUsuario", "idLoja", "user", "passwd", "desativado"});

  QVector<Pair> header;
  header.push_back({"tipo", "Função"});
  header.push_back({"nome", "Nome"});
  header.push_back({"sigla", "Sigla"});
  header.push_back({"email", "E-mail"});
  sdVendedor->setHeaderData(header);

  return sdVendedor;
}

SearchDialog *SearchDialog::enderecoCliente(QWidget *parent) {
  SearchDialog *sdEndereco = new SearchDialog("Buscar Endereço", "cliente_has_endereco", {}, "idEndereco = 1", parent);

  sdEndereco->setPrimaryKey("idEndereco");
  sdEndereco->setTextKeys({"descricao", "logradouro", "numero", "bairro", "cidade", "uf"});

  sdEndereco->hideColumns({"idEndereco", "idCliente", "codUF", "desativado"});

  QVector<Pair> headerData;
  headerData.push_back({"descricao", "Descrição"});
  headerData.push_back({"cep", "CEP"});
  headerData.push_back({"logradouro", "End."});
  headerData.push_back({"numero", "Número"});
  headerData.push_back({"complemento", "Comp."});
  headerData.push_back({"bairro", "Bairro"});
  headerData.push_back({"cidade", "Cidade"});
  headerData.push_back({"uf", "UF"});
  sdEndereco->setHeaderData(headerData);

  return sdEndereco;
}

SearchDialog *SearchDialog::profissional(QWidget *parent) {
  SearchDialog *sdProfissional =
      new SearchDialog("Buscar Profissional", "profissional", {"nome_razao, tipoProf"}, "desativado = FALSE", parent);

  sdProfissional->setPrimaryKey("idProfissional");
  sdProfissional->setTextKeys({"nome_razao"});

  sdProfissional->hideColumns({"idProfissional", "rg", "inscEstadual", "contatoNome", "contatoCPF", "contatoApelido",
                               "contatoRG", "banco", "agencia", "cc", "nomeBanco", "cpfBanco", "incompleto",
                               "desativado"});

  QVector<Pair> header;
  header.push_back({"pfpj", "Tipo"});
  header.push_back({"nome_razao", "Profissional"});
  header.push_back({"nomeFantasia", "Fantasia/Apelido"});
  header.push_back({"cpf", "CPF"});
  header.push_back({"cnpj", "CNPJ"});
  header.push_back({"tel", "Tel."});
  header.push_back({"telCel", "Tel. Cel."});
  header.push_back({"telCom", "Tel. Com."});
  header.push_back({"idNextel", "id Nextel"});
  header.push_back({"nextel", "Nextel"});
  header.push_back({"email", "E-mail"});
  header.push_back({"tipoProf", "Profissão"});
  sdProfissional->setHeaderData(header);

  return sdProfissional;
}

void SearchDialog::on_radioButtonProdAtivos_clicked() { montarFiltroAtivoDesc(true); }

void SearchDialog::on_radioButtonProdDesc_clicked() { montarFiltroAtivoDesc(false); }

void SearchDialog::montarFiltroAtivoDesc(const bool ativo) {
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

  if (isVisible()) {
    ui->tableBusca->resizeColumnsToContents();
  }
}

void SearchDialog::on_tableBusca_entered(const QModelIndex &index) {
  Q_UNUSED(index);

  ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::setRepresentacao(const QString &value) { representacao = value; }
