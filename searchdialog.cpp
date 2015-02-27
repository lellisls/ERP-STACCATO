#include "searchdialog.h"
#include "ui_searchdialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QDebug>
#include "registerdialog.h"
SearchDialog::SearchDialog(QString title, QString table, QStringList indexes, QString filter, QWidget *parent)
  : QDialog(parent), ui(new Ui::SearchDialog), indexes(indexes) {
  ui->setupUi(this);
  setWindowTitle(title);
  setWindowModality(Qt::WindowModal);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  setFilter(filter);
  model.select();
  ui->tableBusca->setModel(&model);
  ui->lineEditBusca->setFocus();

  ui->tableBusca->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBusca->verticalHeader()->setResizeContentsPrecision(5);
  ui->tableBusca->horizontalHeader()->setResizeContentsPrecision(5);
  ui->tableBusca->horizontalHeader()->setStretchLastSection(true);

  textKeys.append( indexes.front() );
  primaryKey = indexes.front();
//  RegisterDialog * reg = dynamic_cast<RegisterDialog * >(parent);
//  if(reg != nullptr) {
//    connect(this, &SearchDialog::itemSelected, reg, &RegisterDialog::changeItem);
//  }
}

SearchDialog::~SearchDialog() {
  delete ui;
}

void SearchDialog::on_lineEditBusca_textChanged(const QString &text) {
  QStringList temp = text.split(" ", QString::SkipEmptyParts);
  for (int i = 0; i < temp.size(); ++i) {
    temp[i].prepend("+");
    temp[i].append("*");
  }
  QString regex = temp.join(" ");
  if (text.isEmpty() || regex.isEmpty()) {
    model.setFilter(filter);
  } else {
    QString searchFilter = "MATCH(" + indexes.join(", ") + ") AGAINST('" + regex + "' in boolean mode)";
    if (!filter.isEmpty()) {
      searchFilter.append(" AND (" + filter + ")");
    }
    qDebug() << "FILTER: " << searchFilter;
    model.setFilter(searchFilter);
  }
  model.select();
}

void SearchDialog::sendUpdateMessage() {
  QModelIndex index = ui->tableBusca->selectionModel()->selection().indexes().front();
  selectedId = model.data(model.index(index.row(), model.fieldIndex(primaryKey)));
  QString text;
  foreach (QString key, textKeys) {
    if(!key.isEmpty()) {
      QVariant val = model.data(model.index(index.row(), model.fieldIndex(key)));
      if(val.isValid()) {
        if(!text.isEmpty())
          text.append(" - ");
        text.append(val.toString());
      }
    }
  }
  emit itemSelected(selectedId, text);
}

void SearchDialog::show() {
  model.select();
  QDialog::show();
}

void SearchDialog::on_tableBusca_doubleClicked(const QModelIndex &index) {

  Q_UNUSED(index)
  sendUpdateMessage();
  ui->lineEditBusca->clear();
  close();
}

QString SearchDialog::getFilter() const {
  return filter;
}

void SearchDialog::setFilter(const QString &value) {
  filter = value;
  model.setFilter(filter);
  model.select();
  //  qDebug() << model.lastError();
}

void SearchDialog::hideColumns(QStringList columns) {
  foreach (QString column, columns) {
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

QStringList SearchDialog::getTextKeys() const {
  return textKeys;
}

void SearchDialog::setTextKeys(const QStringList & value) {
  textKeys = value;
}

QString SearchDialog::getPrimaryKey() const {
  return primaryKey;
}

void SearchDialog::setPrimaryKey(const QString &value) {
  primaryKey = value;
}


QString SearchDialog::getText(QVariant idx) {
  QString qryTxt;
  foreach (QString key, textKeys) {
    if(!qryTxt.isEmpty()) {
      qryTxt += ", ";
    }
    qryTxt += key;
  }
  qryTxt = "SELECT " + qryTxt +  " FROM " + model.tableName() + " WHERE " + primaryKey + " = '" + idx.toString() + "';";
  QSqlQuery qry(qryTxt);
  qry.exec();
  if(qry.first()) {
    QString res;
    foreach (QString key, textKeys) {
      QVariant val = qry.value(key);
      if(val.isValid()) {
        if(!res.isEmpty()) {
          res += " - ";
        }
        res += val.toString();
      }
    }
    return(res);
  }
  qDebug() << objectName() << " : " << __LINE__ << " : " << qry.lastError();
  return( QString() );
}

void SearchDialog::setHeaderData(QVector<QPair<QString, QString> > headerData) {
  QPair<QString, QString> pair;
  foreach (pair, headerData) {
    model.setHeaderData(model.fieldIndex(pair.first), Qt::Horizontal, pair.second);
  }
}

SearchDialog *SearchDialog::cliente(QWidget * parent) {
//  SearchDialog * sdCliente = new SearchDialog("Buscar Cliente", "Cadastro", {"nome", "apelido", "razaoSocial", "nomeFantasia, "cpf"},
//      "tipo = 'CLIENTE' OR tipo = 'AMBOS'", parent);
  SearchDialog * sdCliente = new SearchDialog("Buscar Cliente", "Cadastro", {"nome", "apelido", "razaoSocial", "nomeFantasia", "cpf"},
      "clienteFornecedor = 'CLIENTE'", parent);

//  QStringList columns;
//  columns.push_back("idCadastro");
//  columns.push_back("rg");
//  columns.push_back("cpf");
//  columns.push_back("cnpj");
//  columns.push_back("inscEstadual");
//  columns.push_back("idEnderecoFaturamento");
//  columns.push_back("idEnderecoCobranca");
//  columns.push_back("idEnderecoEntrega");
//  columns.push_back("idUsuarioRel");
//  columns.push_back("idCadastroRel");
//  columns.push_back("idProfissionalRel");
  sdCliente->hideColumns({"idCadastro", "clienteFornecedor", "rg", "cnpj", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca", "idEnderecoEntrega", "idUsuarioRel", "idCadastroRel", "idProfissionalRel"});
  sdCliente->setPrimaryKey("idCadastro");
  sdCliente->setTextKeys({"nomeFantasia","nome"});

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("apelido", "Apelido"));
  headerData.push_back(QPair<QString, QString>("cpf", "CPF"));
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("telCel", "Tel. Cel."));
  headerData.push_back(QPair<QString, QString>("telCom", "Tel. Com."));
  headerData.push_back(QPair<QString, QString>("idNextel", "id Nextel"));
  headerData.push_back(QPair<QString, QString>("nextel", "Nextel"));
  headerData.push_back(QPair<QString, QString>("email", "E-mail"));
  headerData.push_back(QPair<QString, QString>("pfpj", "Tipo"));
  sdCliente->setHeaderData(headerData);

  return sdCliente;
}

SearchDialog *SearchDialog::loja(QWidget * parent) {
  SearchDialog *sdLoja =
    new SearchDialog("Buscar Loja", "Loja", {"descricao, nomeFantasia, razaoSocial"},
                     "", parent);
  sdLoja->setPrimaryKey("idLoja");
  sdLoja->setTextKeys({"nomeFantasia"});

  QStringList columns;
  columns.push_back("idLoja");
  columns.push_back("idEndereco");
  columns.push_back("codUF");
  sdLoja->hideColumns(columns);

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("inscEstadual", "Insc. Est."));
  headerData.push_back(QPair<QString, QString>("sigla", "Sigla"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  sdLoja->setHeaderData(headerData);

  return sdLoja;
}

SearchDialog *SearchDialog::produto(QWidget * parent) {
  SearchDialog *sdProd = new SearchDialog("Buscar Produto", "Produto",
  {"fornecedor", "descricao", "colecao", "codcomercial"}, "", parent);
  sdProd->hideColumns({"idProduto", "idFornecedor", "ncm", "cfop", "situacaoTributaria", "icms", "custo", "ipi", "markup",
                       "comissao", "origem", "ui", "descontinuado", "temLote", "observacoes", "codBarras", "codIndustrial", "qtdPallet", "st"
                      });
  QVector<QPair<QString, QString>> headerDataProduto;
  headerDataProduto.push_back(QPair<QString, QString>("fornecedor", "Fornecedor"));
  headerDataProduto.push_back(QPair<QString, QString>("descricao", "Descrição"));
  headerDataProduto.push_back(QPair<QString, QString>("estoque", "Estoque"));
  headerDataProduto.push_back(QPair<QString, QString>("un", "Un."));
  headerDataProduto.push_back(QPair<QString, QString>("colecao", "Coleção"));
  headerDataProduto.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerDataProduto.push_back(QPair<QString, QString>("m2cx", "M2/Cx."));
  headerDataProduto.push_back(QPair<QString, QString>("pccx", "Pç./Cx."));
  headerDataProduto.push_back(QPair<QString, QString>("formComercial", "Form. Com."));
  headerDataProduto.push_back(QPair<QString, QString>("codComercial", "Cód. Com."));
  headerDataProduto.push_back(QPair<QString, QString>("precoVenda", "R$"));
  sdProd->setHeaderData(headerDataProduto);
  sdProd->setPrimaryKey("idProduto");
  sdProd->setTextKeys({"descricao"});
  return sdProd;
}

SearchDialog *SearchDialog::fornecedor(QWidget * parent) {
  SearchDialog *sdFornecedor = new SearchDialog("Buscar Fornecedor", "Cadastro", {"nome", "nomeFantasia", "razaoSocial"},
      "clienteFornecedor = 'FORNECEDOR'", parent);
  sdFornecedor->hideColumns({"idCadastro", "rg", "cpf", "cnpj", "inscEstadual", "idEnderecoFaturamento", "idEnderecoCobranca", "idEnderecoEntrega", "tel", "telCel", "telCom", "idNextel", "nextel", "email", "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto"});
  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("apelido", "Apelido"));
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  sdFornecedor->setHeaderData(headerData);
  sdFornecedor->setPrimaryKey("idCadastro");
  sdFornecedor->setTextKeys({"nomeFantasia"});
  return sdFornecedor;
}

SearchDialog *SearchDialog::transportadora(QWidget * parent) {
  SearchDialog *sdTransportadora =
    new SearchDialog("Buscar Transportadora", "Transportadora", {"razaoSocial, nomeFantasia"},
                     "", parent);
  sdTransportadora->setPrimaryKey("idTransportadora");
  sdTransportadora->setTextKeys({"nomeFantasia"});

  QStringList columns;
  columns.push_back("idTransportadora");
  columns.push_back("idEndereco");
  sdTransportadora->hideColumns(columns);

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("razaoSocial", "Razão Social"));
  headerData.push_back(QPair<QString, QString>("nomeFantasia", "Nome Fantasia"));
  headerData.push_back(QPair<QString, QString>("cnpj", "CNPJ"));
  headerData.push_back(QPair<QString, QString>("inscEstadual", "Insc. Est."));
  headerData.push_back(QPair<QString, QString>("placaVeiculo", "Placa"));
  headerData.push_back(QPair<QString, QString>("antt", "ANTT"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  sdTransportadora->setHeaderData(headerData);
  return (sdTransportadora);
}

SearchDialog *SearchDialog::usuario(QWidget * parent) {
  SearchDialog *sdUsuario =
    new SearchDialog("Buscar Usuário", "Usuario", {"nome, tipo"},
                     "", parent);
  sdUsuario->setPrimaryKey("idUsuario");
  sdUsuario->setTextKeys({"nome"});

  QStringList columns;
  columns.push_back("idUsuario");
  columns.push_back("user");
  columns.push_back("passwd");
  sdUsuario->hideColumns(columns);

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("idLoja", "Loja"));
  headerData.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("sigla", "Sigla"));
  sdUsuario->setHeaderData(headerData);
  return( sdUsuario );
}

SearchDialog *SearchDialog::profissional(QWidget * parent) {
  SearchDialog *sdProfissional =
    new SearchDialog("Buscar Profissional", "Profissional", {"nome, tipo"},
                     "", parent);
  sdProfissional->setPrimaryKey("idProfissional");
  sdProfissional->setTextKeys({"idxFT"});

  QStringList columns;
  columns.push_back("idProfissional");
  sdProfissional->hideColumns(columns);

  QVector<QPair<QString, QString>> headerData;
  headerData.push_back(QPair<QString, QString>("nome", "Nome"));
  headerData.push_back(QPair<QString, QString>("tipo", "Tipo"));
  headerData.push_back(QPair<QString, QString>("tel", "Tel."));
  headerData.push_back(QPair<QString, QString>("email", "E-mail"));
  headerData.push_back(QPair<QString, QString>("banco", "Banco"));
  headerData.push_back(QPair<QString, QString>("agencia", "Agência"));
  headerData.push_back(QPair<QString, QString>("cc", "CC"));
  headerData.push_back(QPair<QString, QString>("cp", "CP"));
  sdProfissional->setHeaderData(headerData);

  return(sdProfissional);
}


