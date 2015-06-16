#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "registerdialog.h"
#include "doubledelegate.h"

SearchDialog::SearchDialog(QString title, QString table, QStringList indexes, QString filter, QWidget *parent)
  : QDialog(parent), ui(new Ui::SearchDialog) {
  ui->setupUi(this);
  setWindowTitle(title);
  setWindowModality(Qt::WindowModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  setFilter(filter);

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

  ui->tableBusca->setModel(&model);
  ui->tableBusca->horizontalHeader()->setStretchLastSection(false);
  ui->tableBusca->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  DoubleDelegate *doubleDelegate = new DoubleDelegate(this);
  ui->tableBusca->setItemDelegate(doubleDelegate);

  ui->lineEditBusca->setFocus();

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
}

SearchDialog::~SearchDialog() { delete ui; }

void SearchDialog::on_lineEditBusca_textChanged(const QString &text) {
  QStringList temp = text.split(" ", QString::SkipEmptyParts);

  for (int i = 0; i < temp.size(); ++i) {
    temp[i].prepend("+");
    temp[i].append("*");
  }

  QString regex = temp.join(" ");

  if (text.isEmpty() or regex.isEmpty()) {
    model.setFilter(filter);
  } else {
    QString searchFilter = "MATCH(" + indexes.join(", ") + ") AGAINST('" + regex + "' in boolean mode)";

    if (not filter.isEmpty()) {
      searchFilter.append(" AND (" + filter + ")");
    }

    model.setFilter(searchFilter);
  }

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }
}

void SearchDialog::sendUpdateMessage() {
  QModelIndex index = model.index(0, 0);

  if (not ui->tableBusca->selectionModel()->selection().indexes().isEmpty()) {
    index = ui->tableBusca->selectionModel()->selection().indexes().front();
  }

  selectedId = model.data(model.index(index.row(), model.fieldIndex(primaryKey)));
  QString text;

  foreach (QString key, textKeys) {
    if (not key.isEmpty()) {
      QVariant val = model.data(model.index(index.row(), model.fieldIndex(key)));

      if (val.isValid()) {
        if (not text.isEmpty()) {
          text.append(" - ");
        }

        text.append(val.toString());
      }
    }
  }

  emit itemSelected(selectedId);
}

void SearchDialog::show() {
  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

  QDialog::show();
  ui->tableBusca->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableBusca->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::showMaximized() {
  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

  QDialog::showMaximized();
  ui->tableBusca->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableBusca->horizontalHeader()->setResizeContentsPrecision(0);
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

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

  ui->tableBusca->resizeColumnsToContents();
}

void SearchDialog::hideColumns(QStringList columns) {
  foreach (QString column, columns) { ui->tableBusca->setColumnHidden(model.fieldIndex(column), true); }
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

QString SearchDialog::getText(QVariant index) {
  QString qryTxt;

  foreach (QString key, textKeys) {
    if (not qryTxt.isEmpty()) {
      qryTxt += ", ";
    }

    qryTxt += key;
  }

  qryTxt =
      "SELECT " + qryTxt + " FROM " + model.tableName() + " WHERE " + primaryKey + " = '" + index.toString() + "';";

  QSqlQuery qry(qryTxt);
  qry.exec();

  if (qry.first()) {
    QString res;

    foreach (QString key, textKeys) {
      QVariant val = qry.value(key);

      if (val.isValid()) {
        if (not res.isEmpty()) {
          res += " - ";
        }

        res += val.toString();
      }
    }

    return (res);
  }

  qDebug() << objectName() << " : " << __LINE__ << " : " << qry.lastError();

  return (QString());
}

void SearchDialog::setHeaderData(QVector<QPair<QString, QString>> headerData) {
  QPair<QString, QString> pair;

  foreach (pair, headerData) { model.setHeaderData(model.fieldIndex(pair.first), Qt::Horizontal, pair.second); }
}

SearchDialog *SearchDialog::cliente(QWidget *parent) {
  SearchDialog *sdCliente = new SearchDialog("Buscar Cliente", "Cliente", {"nome_razao", "nomeFantasia", "cpf", "cnpj"},
                                             "desativado = false", parent);

  sdCliente->setPrimaryKey("idCliente");
  sdCliente->setTextKeys({"nomeFantasia", "nome_razao"});

  sdCliente->hideColumns({"idCliente", "rg", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca",
                          "idEnderecoEntrega", "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto",
                          "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("pfpj", "Tipo"));
  headerData.push_back(QPair<QString, QString>("nome_razao", "Cliente"));
  headerData.push_back(QPair<QString, QString>("cpf", "CPF"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("contatoNome", "Nome - Contato"));
  headerData.push_back(QPair<QString, QString>("contatoCPF", "CPF - Contato"));
  headerData.push_back(QPair<QString, QString>("contatoApelido", "Apelido - Contato"));
  headerData.push_back(QPair<QString, QString>("contatoRG", "RG - Contato"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Fantasia/Apelido"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("telCel", "Tel. Cel."));
  headerData.push_back(QPair<QString, QString>("telCom", "Tel. Com."));
  headerData.push_back(QPair<QString, QString>("idNextel", "id Nextel"));
  headerData.push_back(QPair<QString, QString>("nextel", "Nextel"));
  headerData.push_back(QPair<QString, QString>("email", "E-mail"));
  sdCliente->setHeaderData(headerData);

  return sdCliente;
}

SearchDialog *SearchDialog::loja(QWidget *parent) {
  SearchDialog *sdLoja =
      new SearchDialog("Buscar Loja", "Loja", {"descricao, nomeFantasia, razaoSocial"}, "desativado = false", parent);

  sdLoja->setPrimaryKey("idLoja");
  sdLoja->setTextKeys({"nomeFantasia"});

  sdLoja->hideColumns({"idLoja", "idEndereco", "codUF", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("inscEstadual", "Insc. Est."));
  headerData.push_back(QPair<QString, QString>("sigla", "Sigla"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("porcentagemFrete", "% Frete"));
  headerData.push_back(QPair<QString, QString>("valorMinimoFrete", "R$ Mínimo Frete"));
  sdLoja->setHeaderData(headerData);

  return sdLoja;
}

SearchDialog *SearchDialog::produto(QWidget *parent) {
  SearchDialog *sdProd =
      new SearchDialog("Buscar Produto", "Produto", {"fornecedor", "descricao", "colecao", "codcomercial"}, "", parent);

  sdProd->setPrimaryKey("idProduto");
  sdProd->setTextKeys({"descricao"});

  sdProd->hideColumns({"idProduto", "idFornecedor", "situacaoTributaria", "icms", "custo", "ipi", "markup", "comissao",
                       "origem", "descontinuado", "temLote", "observacoes", "codBarras", "codIndustrial", "qtdPallet",
                       "st", "desativado", "cfop", "ncm", "atualizarTabelaPreco"});

  for (int i = 1; i <= sdProd->model.fieldIndex("descontinuadoUpd"); i += 2) {
    sdProd->ui->tableBusca->setColumnHidden(i, true); // this hides *Upd fields
  }

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("fornecedor", "Fornecedor"));
  headerData.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerData.push_back(QPair<QString, QString>("estoque", "Estoque"));
  headerData.push_back(QPair<QString, QString>("un", "Un."));
  headerData.push_back(QPair<QString, QString>("colecao", "Coleção"));
  headerData.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerData.push_back(QPair<QString, QString>("m2cx", "M/Cx."));
  headerData.push_back(QPair<QString, QString>("pccx", "Pç./Cx."));
  headerData.push_back(QPair<QString, QString>("kgcx", "Kg./Cx."));
  headerData.push_back(QPair<QString, QString>("formComercial", "Form. Com."));
  headerData.push_back(QPair<QString, QString>("codComercial", "Cód. Com."));
  headerData.push_back(QPair<QString, QString>("precoVenda", "R$"));
  headerData.push_back(QPair<QString, QString>("validade", "Validade"));
  headerData.push_back(QPair<QString, QString>("ui", "UI"));
  sdProd->setHeaderData(headerData);

  sdProd->ui->groupBoxFiltrosProduto->show();
  sdProd->ui->radioButtonProdAtivos->click();

  return sdProd;
}

SearchDialog *SearchDialog::fornecedor(QWidget *parent) {
  SearchDialog *sdFornecedor = new SearchDialog(
                                 "Buscar Fornecedor", "Fornecedor", {"nome_razao", "nomeFantasia", "cpf", "cnpj"}, "desativado = false", parent);

  sdFornecedor->setPrimaryKey("idFornecedor");
  sdFornecedor->setTextKeys({"nomeFantasia", "razaoSocial"});

  sdFornecedor->hideColumns({"idFornecedor", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca",
                             "idEnderecoEntrega", "tel", "telCel", "telCom", "idNextel", "nextel", "email",
                             "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("contatoNome", "Nome do Contato"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("contatoCPF", "CPF do Contato"));
  headerData.push_back(QPair<QString, QString>("contatoApelido", "Apelido do Contato"));
  headerData.push_back(QPair<QString, QString>("contatoRG", "RG do Contato"));
  sdFornecedor->setHeaderData(headerData);

  return sdFornecedor;
}

SearchDialog *SearchDialog::transportadora(QWidget *parent) {
  SearchDialog *sdTransportadora = new SearchDialog("Buscar Transportadora", "Transportadora",
  {"razaoSocial", "nomeFantasia"}, "desativado = false", parent);

  sdTransportadora->setPrimaryKey("idTransportadora");
  sdTransportadora->setTextKeys({"nomeFantasia"});

  sdTransportadora->hideColumns({"idTransportadora", "idEndereco", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("inscEstadual", "Insc. Est."));
  headerData.push_back(QPair<QString, QString>("placaVeiculo", "Placa"));
  headerData.push_back(QPair<QString, QString>("antt", "ANTT"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  sdTransportadora->setHeaderData(headerData);

  return sdTransportadora;
}

SearchDialog *SearchDialog::usuario(QWidget *parent) {
  SearchDialog *sdUsuario =
      new SearchDialog("Buscar Usuário", "Usuario", {"nome, tipo"}, "Usuario.desativado = false", parent);

  sdUsuario->setPrimaryKey("idUsuario");
  sdUsuario->setTextKeys({"nome"});

  sdUsuario->hideColumns({"idUsuario", "user", "passwd", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("idLoja", "Loja"));
  headerData.push_back(QPair<QString, QString>("tipo", "Função"));
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("sigla", "Sigla"));
  sdUsuario->setHeaderData(headerData);

  sdUsuario->model.setRelation(sdUsuario->model.fieldIndex("idLoja"), QSqlRelation("Loja", "idLoja", "descricao"));

  return sdUsuario;
}

SearchDialog *SearchDialog::vendedor(QWidget *parent) {
  SearchDialog *sdVendedor = new SearchDialog("Buscar Vendedor", "Usuario", {"nome, tipo"},
                                              "desativado = false AND tipo = 'VENDEDOR'", parent);

  sdVendedor->setPrimaryKey("idUsuario");
  sdVendedor->setTextKeys({"nome"});

  sdVendedor->hideColumns({"idUsuario", "idLoja", "user", "passwd", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("tipo", "Função"));
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("sigla", "Sigla"));
  sdVendedor->setHeaderData(headerData);

  return sdVendedor;
}

SearchDialog *SearchDialog::enderecoCliente(QWidget *parent) {
  SearchDialog *sdEndereco = new SearchDialog("Buscar Endereço", "Cliente_has_Endereco", {}, "idEndereco = 1", parent);

  sdEndereco->setPrimaryKey("idEndereco");
  sdEndereco->setTextKeys({"descricao", "logradouro", "numero", "bairro", "cidade", "uf"});

  sdEndereco->hideColumns({"idEndereco", "idCliente", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerData.push_back(QPair<QString, QString>("cep", "CEP"));
  headerData.push_back(QPair<QString, QString>("logradouro", "End."));
  headerData.push_back(QPair<QString, QString>("numero", "Número"));
  headerData.push_back(QPair<QString, QString>("complemento", "Comp."));
  headerData.push_back(QPair<QString, QString>("bairro", "Bairro"));
  headerData.push_back(QPair<QString, QString>("cidade", "Cidade"));
  headerData.push_back(QPair<QString, QString>("uf", "UF"));
  sdEndereco->setHeaderData(headerData);

  return sdEndereco;
}

SearchDialog *SearchDialog::enderecoFornecedor(QWidget *parent) {
  SearchDialog *sdEndereco =
      new SearchDialog("Buscar Endereço", "Fornecedor_has_Endereco", {}, "idFornecedor = 0", parent);

  sdEndereco->setPrimaryKey("idEndereco");
  sdEndereco->setTextKeys({"descricao", "logradouro", "numero", "bairro", "cidade", "uf"});

  sdEndereco->hideColumns({"idEndereco", "idCliente", "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerData.push_back(QPair<QString, QString>("cep", "CEP"));
  headerData.push_back(QPair<QString, QString>("logradouro", "End."));
  headerData.push_back(QPair<QString, QString>("numero", "Número"));
  headerData.push_back(QPair<QString, QString>("complemento", "Comp."));
  headerData.push_back(QPair<QString, QString>("bairro", "Bairro"));
  headerData.push_back(QPair<QString, QString>("cidade", "Cidade"));
  headerData.push_back(QPair<QString, QString>("uf", "UF"));
  sdEndereco->setHeaderData(headerData);

  return sdEndereco;
}

SearchDialog *SearchDialog::profissional(QWidget *parent) {
  SearchDialog *sdProfissional =
      new SearchDialog("Buscar Profissional", "Profissional", {"nome_razao, tipoProf"}, "desativado = false", parent);

  sdProfissional->setPrimaryKey("idProfissional");
  sdProfissional->setTextKeys({"nome_razao"});

  sdProfissional->hideColumns({"idProfissional", "rg", "inscEstadual", "contatoNome", "contatoCPF", "contatoApelido",
                               "contatoRG", "banco", "agencia", "cc", "nomeBanco", "cpfBanco", "incompleto",
                               "desativado"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("pfpj", "Tipo"));
  headerData.push_back(QPair<QString, QString>("nome_razao", "Profissional"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Fantasia/Apelido"));
  headerData.push_back(QPair<QString, QString>("cpf", "CPF"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("telCel", "Tel. Cel."));
  headerData.push_back(QPair<QString, QString>("telCom", "Tel. Com."));
  headerData.push_back(QPair<QString, QString>("idNextel", "id Nextel"));
  headerData.push_back(QPair<QString, QString>("nextel", "Nextel"));
  headerData.push_back(QPair<QString, QString>("email", "E-mail"));
  headerData.push_back(QPair<QString, QString>("tipoProf", "Profissão"));
  sdProfissional->setHeaderData(headerData);

  return sdProfissional;
}

void SearchDialog::on_radioButtonProdAtivos_clicked() {
  model.setFilter("descontinuado = false AND desativado = false");
}

void SearchDialog::on_radioButtonProdDesc_clicked() { model.setFilter("descontinuado = true AND desativado = false"); }
