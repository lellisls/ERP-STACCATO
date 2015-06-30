#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "endereco.h"
#include "mainwindow.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

#include <QFile>
#include <QDate>
#include <QFileInfo>
#include <QInputDialog>
#include <QSqlDriver>

CadastrarNFe::CadastrarNFe(QString idVenda, QWidget *parent)
  : QDialog(parent), ui(new Ui::CadastrarNFe), idVenda(idVenda) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdEndereco = SearchDialog::enderecoCliente(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  RegisterDialog *cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  modelNFe.setTable("NFe");
  modelNFe.setEditStrategy(EditableSqlModel::OnManualSubmit);

  if (not modelNFe.select()) {
    qDebug() << "erro modelNFe: " << modelNFe.lastError();
    return;
  }

  modelNFeItem.setTable("NFe_has_Itens");
  modelNFeItem.setEditStrategy(EditableSqlModel::OnManualSubmit);

  if (not modelNFeItem.select()) {
    qDebug() << "erro modelNFeItem: " << modelNFeItem.lastError();
    return;
  }

  ui->tableItens->setModel(&modelNFeItem);

  mapperNFe.setModel(&modelNFe);
  mapperNFe.addMapping(ui->itemBoxCliente, modelNFe.fieldIndex("idCliente"));
  mapperNFe.addMapping(ui->itemBoxEndereco, modelNFe.fieldIndex("idEnderecoFaturamento"));
  mapperNFe.addMapping(ui->doubleSpinBoxFrete, modelNFe.fieldIndex("frete"));
  mapperNFe.addMapping(ui->doubleSpinBoxFinal, modelNFe.fieldIndex("total"));
  mapperNFe.addMapping(ui->textEdit, modelNFe.fieldIndex("obs"));
  mapperNFe.toLast();

  connect(ui->tableItens->model(), &QAbstractItemModel::dataChanged, this, &CadastrarNFe::onDataChanged);

  modelLoja.setTable("Loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelLoja.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelLoja.select()) {
    qDebug() << "erro modelLoja: " << modelLoja.lastError();
    return;
  }

  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");

  if (not modelVenda.select()) {
    qDebug() << "erro modelVenda: " << modelVenda.lastError();
    return;
  }

  modelProd.setTable("Venda_has_Produto");
  modelProd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelProd.setFilter("idVenda = '" + idVenda + "'");

  if (not modelProd.select()) {
    qDebug() << "erro modelProd: " << modelProd.lastError();
    return;
  }
}

CadastrarNFe::~CadastrarNFe() { delete ui; }

void CadastrarNFe::guardarNotaBD() {
  const QString arquivo = getFromLoja("pastaXmlACBr").toString() + chaveNum + "-nfe.xml";
  qDebug() << "arquivo: " << arquivo;
  const QFile file(arquivo);

  bool espera = true;

  while (espera) {
    if (file.exists()) {
      qDebug() << "file found!";

      QSqlQuery queryNota;
      queryNota.prepare("INSERT INTO NFe (idVenda, idLoja, idCliente, "
                        "idEnderecoFaturamento, NFe, status, chaveAcesso, "
                        "obs, frete, total) VALUES (:idVenda, :idLoja, "
                        ":idCliente, :idEnderecoFaturamento, :NFe, "
                        ":status, :chaveAcesso, :obs, :frete, :total)");
      queryNota.bindValue(":idVenda", idVenda);
      queryNota.bindValue(":idLoja", getFromLoja("idLoja"));
      queryNota.bindValue(":idCliente", getFromVenda("idCliente"));
      queryNota.bindValue(":idEnderecoFaturamento", getFromVenda("idEnderecoFaturamento"));
      queryNota.bindValue(":NFe", "load_file('" + arquivo + "')");
      queryNota.bindValue(":status", "''");
      queryNota.bindValue(":chaveAcesso", chaveNum);
      queryNota.bindValue(":obs", "''");
      queryNota.bindValue(":frete", ui->doubleSpinBoxFrete->value());
      queryNota.bindValue(":total", ui->doubleSpinBoxFinal->value());

      if (not queryNota.exec()) {
        qDebug() << "Erro guardando nota: " << queryNota.lastError();
      } else {
        qDebug() << "Nota guardada com sucesso!";
        QMessageBox::information(this, "Aviso!", "Nota guardada com sucesso.");
      }

      espera = false;
    } else {
      qDebug() << ":`(";
      // TODO: caso não encontre o arquivo, guarde os outros dados e
      // posteriormente na tela de nfe pesquisa pelo arquivo
      QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Esperar ACBr?", QMessageBox::Yes | QMessageBox::No, this);
      msgBox.setButtonText(QMessageBox::Yes, "Sim");
      msgBox.setButtonText(QMessageBox::No, "Não");

      if (msgBox.exec() == QMessageBox::Yes) {
        espera = true;
      } else {
        espera = false;
      }
    }
  }
}

void CadastrarNFe::on_pushButtonGerarNFE_clicked() {
  if (writeTXT()) {
    QMessageBox::information(this, "Aviso!", "NFe gerada com sucesso!");

    guardarNotaBD();

  } else {
    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro, NFe não foi gerada.");
  }
}

void CadastrarNFe::on_pushButtonCancelar_clicked() { close(); }

void CadastrarNFe::updateImpostos() {
  double icms = 0;

  for (int row = 0, rowCount = modelNFeItem.rowCount(); row < rowCount; ++row) {
    icms += getItemData(row, "valorICMS").toDouble();
  }

  ui->doubleSpinBoxVlICMS->setValue(icms);
  const double imposto = 0.593 * ui->doubleSpinBoxFinal->value() + icms;
  Endereco end(ui->itemBoxEndereco->value().toInt(), "Cliente_has_Endereco");
  // TODO: fix this
  QString texto = "Venda de código " +
                  modelNFe.data(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idVenda"))).toString() +
                  "\n" + "END. ENTREGA: " + end.umaLinha() + "\n" +
                  "Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO "
                  "ANTECIPADAMENTE CONFORME ARTIGO 3113Y\n" +
                  "Total aproximado de tributos federais, estaduais e municipais: " + QString::number(imposto);
  ui->textEdit->setPlainText(texto);
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("obs")), texto);
}

void CadastrarNFe::setItemData(const int row, const QString &key, const QVariant &value) {
  if (modelNFeItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }

  modelNFeItem.setData(modelNFeItem.index(row, modelNFeItem.fieldIndex(key)), value);
}

QVariant CadastrarNFe::getItemData(const int row, const QString &key) const {
  if (modelNFeItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }

  return (modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex(key))));
}

void CadastrarNFe::prepararNFe(const QList<int> items) {
  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM Venda WHERE idVenda = :idVenda");
  queryVenda.bindValue(":idVenda", idVenda);

  if (not queryVenda.exec() or not queryVenda.first()) {
    qDebug() << "Erro lendo venda: " << queryVenda.lastError();
  }

  if (not modelNFe.select()) {
    qDebug() << "erro modelNFe: " << modelNFe.lastError();
    return;
  }

  const int row = modelNFe.rowCount();
  modelNFe.insertRow(row);
  mapperNFe.toLast();

  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idVenda")), idVenda);
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("frete")), queryVenda.value("frete"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("total")), queryVenda.value("total"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idLoja")), queryVenda.value("idLoja"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idCliente")),
                   queryVenda.value("idCliente"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idEnderecoFaturamento")),
                   queryVenda.value("idEnderecoFaturamento"));

  ui->doubleSpinBoxFinal->setValue(queryVenda.value("total").toDouble());
  ui->doubleSpinBoxFrete->setValue(queryVenda.value("frete").toDouble());
  ui->itemBoxCliente->setValue(queryVenda.value("idCliente"));
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                 " AND desativado = FALSE");
  ui->itemBoxEndereco->setValue(queryVenda.value("idEnderecoFaturamento"));

  const double descontoGlobal = queryVenda.value("descontoPorc").toDouble();

  double frete = 0;
  double total = 0;

  foreach (const int item, items) {
    QSqlQuery queryItens;
    queryItens.prepare("SELECT * FROM Venda_has_Produto NATURAL LEFT JOIN "
                       "Produto WHERE idVenda = :idVenda AND item = :item");
    queryItens.bindValue(":idVenda", idVenda);
    queryItens.bindValue(":item", item);

    if (not queryItens.exec() or not queryItens.first()) {
      qDebug() << "Erro buscando produto: " << queryItens.lastError();
      qDebug() << "Last query: " << queryItens.lastQuery();
      return;
    }

    const int row = modelNFeItem.rowCount();
    modelNFeItem.insertRow(row);

    setItemData(row, "item", row + 1);
    setItemData(row, "cst", queryItens.value("cst"));
    setItemData(row, "cfop", "5405"); // TODO: get CFOP from NCM<>CFOP table
    setItemData(row, "codComercial", queryItens.value("codComercial"));
    setItemData(row, "descricao", queryItens.value("produto"));
    setItemData(row, "ncm", queryItens.value("ncm"));
    setItemData(row, "un", queryItens.value("un"));
    setItemData(row, "qte", queryItens.value("qte"));
    setItemData(row, "valorUnitario", queryItens.value("prcUnitario"));

    const double total = queryItens.value("total").toDouble() * (1.0 - (descontoGlobal / 100.0));
    setItemData(row, "valorTotal", total);

    frete += queryItens.value("total").toDouble();
  }

  total = frete;
  frete /= queryVenda.value("subTotalLiq").toDouble();
  frete *= queryVenda.value("frete").toDouble();
  total += frete;
  qDebug() << "frete: " << frete;
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxFinal->setValue(total);

  ui->tableItens->resizeColumnsToContents();
  updateImpostos();

  chaveAcesso = "NFe" + criarChaveAcesso();

  ui->lineEditChave->setText(criarChaveAcesso());
  ui->lineEditNatureza->setText("VENDA");
  ui->lineEditModelo->setText("55");
  ui->lineEditSerie->setText(chaveAcesso.mid(25, 3));
  ui->lineEditCodigo->setText(chaveAcesso.mid(25, 3));
  ui->lineEditNumero->setText(chaveAcesso.mid(28, 9));
  ui->lineEditEmissao->setText(QDate::currentDate().toString("dd/MM/yyyy"));
  ui->lineEditSaida->setText(QDate::currentDate().toString("dd/MM/yyyy"));
  ui->lineEditTipo->setText("1");
  ui->lineEditFormatoPagina->setText("0");
}

void CadastrarNFe::on_tableItens_activated(const QModelIndex &index) {
  Q_UNUSED(index);

  updateImpostos();
}

void CadastrarNFe::on_tableItens_pressed(const QModelIndex &index) {
  Q_UNUSED(index);

  updateImpostos();
}

QString CadastrarNFe::criarChaveAcesso() {
  QSqlQuery query;

  if (not query.exec("SELECT idNFe FROM NFe ORDER BY idNFe DESC LIMIT 1")) {
    qDebug() << "Erro buscando idNFe: " << query.lastError();
  }

  int id = 1;

  if (query.first()) {
    id = query.value("idNFe").toInt() + 1;
  }

  if (id > 999999999) {
    id = 1;
  }

  QSqlQuery queryLojaEnd;
  queryLojaEnd.prepare("SELECT * FROM Loja_has_Endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", UserSession::getLoja());

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    qDebug() << "End. loja failed! : " << queryLojaEnd.lastError();
    return QString();
  }

  QStringList listChave;

  listChave.push_back(queryLojaEnd.value("codUF").toString()); // cUF - código da UF
  //  listChave.push_back(ui->lineEditcodUF->text());

  listChave.push_back(QDate::currentDate().toString("yyMM")); // Ano/Mês
  //  listChave.push_back(ui->lineEditAnoMes->text());

  const QString cnpj = clearStr(getFromLoja("cnpj").toString());
  listChave.push_back(cnpj); // CNPJ
  //  listChave.push_back(ui->lineEditCNPJ->text());

  listChave.push_back("55"); // modelo
  //  listChave.push_back(ui->lineEditModelo->text());

  QString serie = ui->lineEditSerie->text();
  serie = QString("%1").arg(serie.toInt(), 3, 10, QChar('0'));
  listChave.push_back(serie); // série

  listChave.push_back(QString("%1").arg(id, 9, 10, QChar('0'))); // número nf-e (id interno)
  listChave.push_back("1");                                      // tpEmis - forma de emissão
  //  listChave.push_back(ui);
  listChave.push_back("00000001"); // código númerico aleatorio

  QString chave = listChave.join("");

  chave += calculaDigitoVerificador(chave);

  return chave;
}

QString CadastrarNFe::clearStr(QString str) {
  return str.remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

void CadastrarNFe::onDataChanged(const QModelIndex &topLeft,
                                 const QModelIndex &bottomRight) { // TODO: finish implementation
  if (topLeft == bottomRight) {                                    // only one cell changed
    //    qDebug() << "cell changed: " << topLeft;

    if (topLeft.column() == 11) { // baseICMS (valor produto + frete)
    }

    if (topLeft.column() == 12) { // valorICMS
    }

    if (topLeft.column() == 13) { // valorIPI
    }

    if (topLeft.column() == 14) { // aliquotaICMS
      if (topLeft.data().toInt() > 100) {
        //        qDebug() << "bigger than 100! limit.";
        modelNFeItem.setData(topLeft, 100);
      }

      const double icms =
          modelNFeItem.data(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelNFeItem.setData(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorICMS")), icms);
    }

    if (topLeft.column() == 15) { // aliquotaIPI
      if (topLeft.data().toInt() > 100) {
        qDebug() << "bigger than 100! limit.";
        modelNFeItem.setData(topLeft, 100);
      }

      const double icms =
          modelNFeItem.data(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelNFeItem.setData(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorIPI")), icms);
    }
  }
}

QVariant CadastrarNFe::getFromVenda(const QString column) const {
  return (modelVenda.data(modelVenda.index(0, modelVenda.fieldIndex(column))));
}

QVariant CadastrarNFe::getFromLoja(const QString column) const {
  return (modelLoja.data(modelLoja.index(0, modelLoja.fieldIndex(column))));
}

QVariant CadastrarNFe::getFromItemModel(const int row, const QString column) const {
  return (modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex(column))));
}

QVariant CadastrarNFe::getFromProdModel(const int row, const QString column) const {
  return (modelProd.data(modelProd.index(row, modelProd.fieldIndex(column))));
}

QString CadastrarNFe::calculaDigitoVerificador(const QString chave) {
  if (chave.size() != 43) {
    qDebug() << "Erro na chave: " << chave;
    return QString();
  }

  QVector<int> chave2;
  for (int i = 0, size = chave.size(); i < size; ++i) {
    chave2.push_back(chave.at(i).digitValue());
  }

  const QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                        6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  int soma = 0;

  for (int i = 0; i < 43; ++i) {
    soma += chave2.at(i) * multiplicadores.at(i);
  }

  const int resto = soma % 11;
  int cDV = 11 - resto;

  if (resto == 1) {
    cDV = 0;
  }

  return QString::number(cDV);
}

void CadastrarNFe::writeIdentificacao(QTextStream &stream) {
  stream << "[Identificacao]" << endl;
  stream << "NaturezaOperacao = " << ui->lineEditNatureza->text() << endl;
  stream << "Modelo = " << ui->lineEditModelo->text() << endl;
  stream << "Serie = " << ui->lineEditSerie->text() << endl;
  stream << "Codigo = " << ui->lineEditCodigo->text() << endl;
  stream << "Numero = " << ui->lineEditNumero->text() << endl;
  stream << "Emissao = " << ui->lineEditEmissao->text() << endl;
  stream << "Saida = " << ui->lineEditSaida->text() << endl;
  stream << "Tipo = " << ui->lineEditTipo->text() << endl;
  stream << "FormaPag = " << ui->lineEditFormatoPagina->text() << endl;
}

bool CadastrarNFe::writeEmitente(QTextStream &stream) {
  stream << "[Emitente]" << endl;

  if (clearStr(getFromLoja("cnpj").toString()).isEmpty()) {
    qDebug() << "cnpj vazio";
    return false;
  }

  stream << "CNPJ = " + clearStr(getFromLoja("cnpj").toString()) << endl;

  stream << "IE = " + getFromLoja("inscEstadual").toString() << endl;

  if (getFromLoja("razaoSocial").toString().isEmpty()) {
    qDebug() << "razaoSocial vazio";
    return false;
  }

  stream << "Razao = " + getFromLoja("razaoSocial").toString() << endl;

  if (getFromLoja("nomeFantasia").toString().isEmpty()) {
    qDebug() << "nomeFantasia vazio";
    return false;
  }

  stream << "Fantasia = " + getFromLoja("nomeFantasia").toString() << endl;

  if (getFromLoja("tel").toString().isEmpty()) {
    qDebug() << "tel vazio";
    return false;
  }

  stream << "Fone = " + getFromLoja("tel").toString() << endl;

  QSqlQuery queryLojaEnd;
  queryLojaEnd.prepare("SELECT * FROM Loja_has_Endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", UserSession::getLoja());

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    qDebug() << "End. loja failed! : " << queryLojaEnd.lastError();
    return false;
  }

  if (clearStr(queryLojaEnd.value("CEP").toString()).isEmpty()) {
    qDebug() << "CEP vazio";
    return false;
  }

  stream << "CEP = " + clearStr(queryLojaEnd.value("CEP").toString()) << endl;

  if (queryLojaEnd.value("logradouro").toString().isEmpty()) {
    qDebug() << "logradouro vazio";
    return false;
  }

  stream << "Logradouro = " + queryLojaEnd.value("logradouro").toString() << endl;

  if (queryLojaEnd.value("numero").toString().isEmpty()) {
    qDebug() << "numero vazio";
    return false;
  }

  stream << "Numero = " + queryLojaEnd.value("numero").toString() << endl;

  stream << "Complemento = " + queryLojaEnd.value("complemento").toString() << endl;

  if (queryLojaEnd.value("bairro").toString().isEmpty()) {
    qDebug() << "bairro vazio";
    return false;
  }

  stream << "Bairro = " + queryLojaEnd.value("bairro").toString() << endl;

  if (queryLojaEnd.value("cidade").toString().isEmpty()) {
    qDebug() << "cidade vazio";
    return false;
  }

  stream << "Cidade = " + queryLojaEnd.value("cidade").toString() << endl;

  if (queryLojaEnd.value("uf").toString().isEmpty()) {
    qDebug() << "uf vazio";
    return false;
  }

  stream << "UF = " + queryLojaEnd.value("uf").toString() << endl;

  return true;
}

bool CadastrarNFe::writeDestinatario(QTextStream &stream) {
  stream << "[Destinatario]" << endl;

  const QString idCliente = getFromVenda("idCliente").toString();

  if (idCliente.isEmpty()) {
    qDebug() << "idCliente vazio";
    return false;
  }

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM Cliente LEFT JOIN Cliente_has_Endereco "
                       "ON Cliente.idCliente = "
                       "Cliente_has_Endereco.idCliente WHERE "
                       "Cliente_has_Endereco.idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando endereço do destinatário: " << queryCliente.lastError();
    return false;
  }

  if (queryCliente.value("nome_razao").toString().isEmpty()) {
    qDebug() << "nome_razao vazio";
    return false;
  }

  stream << "NomeRazao = " + queryCliente.value("nome_razao").toString() << endl;

  if (queryCliente.value("pfpj").toString() == "PF") {
    if (clearStr(queryCliente.value("cpf").toString()).isEmpty()) {
      qDebug() << "cpf vazio";
      return false;
    }

    stream << "CPF = " + clearStr(queryCliente.value("cpf").toString()) << endl;

    stream << "indIEDest = 9" << endl;
  }

  if (queryCliente.value("pfpj").toString() == "PJ") {
    if (clearStr(queryCliente.value("cnpj").toString()).isEmpty()) {
      qDebug() << "cnpj dest vazio";
      return false;
    }

    stream << "CNPJ = " + clearStr(queryCliente.value("cnpj").toString()) << endl;
    stream << "IE = " + clearStr(queryCliente.value("inscEstadual").toString()) << endl;

    if (queryCliente.value("inscEstadual").toString() == "ISENTO") {
      stream << "indIEDest = 2" << endl;
    }
  }

  if (queryCliente.value("tel").toString().isEmpty()) {
    qDebug() << "tel vazio";
    return false;
  }

  stream << "Fone = " + queryCliente.value("tel").toString() << endl;

  if (queryCliente.value("CEP").toString().isEmpty()) {
    qDebug() << "CEP vazio";
    return false;
  }

  stream << "CEP = " + clearStr(queryCliente.value("CEP").toString()) << endl;

  if (queryCliente.value("logradouro").toString().isEmpty()) {
    qDebug() << "logradouro vazio";
    return false;
  }

  stream << "Logradouro = " + queryCliente.value("logradouro").toString() << endl;

  if (queryCliente.value("numero").toString().isEmpty()) {
    qDebug() << "numero vazio";
    return false;
  }

  stream << "Numero = " + queryCliente.value("numero").toString() << endl;

  stream << "Complemento = " + queryCliente.value("complemento").toString() << endl;

  if (queryCliente.value("bairro").toString().isEmpty()) {
    qDebug() << "bairro vazio";
    return false;
  }

  stream << "Bairro = " + queryCliente.value("bairro").toString() << endl;

  if (queryCliente.value("cidade").toString().isEmpty()) {
    qDebug() << "cidade vazio";
    return false;
  }

  stream << "Cidade = " + queryCliente.value("cidade").toString() << endl;

  if (queryCliente.value("uf").toString().isEmpty()) {
    qDebug() << "uf vazio";
    return false;
  }

  stream << "UF = " + queryCliente.value("uf").toString() << endl;

  return true;
}

bool CadastrarNFe::writeProduto(QTextStream &stream, double &total, double &icmsTotal) {
  for (int row = 0, rowCount = modelNFeItem.rowCount(); row < rowCount; ++row) {
    QSqlQuery queryProd;
    queryProd.prepare("SELECT * FROM Produto WHERE idProduto = :idProduto");
    queryProd.bindValue(":idProduto", getFromProdModel(row, "idProduto"));

    if (not queryProd.exec() or not queryProd.first()) {
      qDebug() << "Erro buscando produtos: " << queryProd.lastError();
      return false;
    }

    const QString numProd = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding with zeros
    stream << "[Produto" + numProd + "]" << endl;
    stream << "CFOP = " + modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex("cfop"))).toString()
           << endl;
    stream << "NCM = " + clearStr(modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex("ncm"))).toString())
           << endl;

    if (not queryProd.value("codBarras").toString().isEmpty()) {
      stream << "Codigo - " + queryProd.value("codBarras").toString() << endl;
    }

    if (not queryProd.value("descricao").toString().isEmpty()) {
      stream << "Descricao = " + queryProd.value("descricao").toString() << endl;
    } else {
      qDebug() << "descricao vazio";
      return false;
    }

    if (not queryProd.value("un").toString().isEmpty()) {
      stream << "Unidade = " + queryProd.value("un").toString() << endl;
    } else {
      qDebug() << "un vazio";
      return false;
    }

    if (not getFromProdModel(row, "qte").toString().isEmpty()) {
      stream << "Quantidade = " + getFromProdModel(row, "qte").toString() << endl;
    } else {
      qDebug() << "qte vazio";
      return false;
    }

    if (not queryProd.value("precoVenda").toDouble() == 0) {
      const double preco = queryProd.value("precoVenda").toDouble();
      const double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.0;
      stream << "ValorUnitario = " + QString::number(rounded_number) << endl;
    } else {
      qDebug() << "precoVenda = 0";
      return false;
    }

    if (not getFromProdModel(row, "parcial").toString().isEmpty()) {
      stream << "ValorTotal = " + getFromProdModel(row, "parcial").toString() << endl;
    } else {
      qDebug() << "parcial vazio";
      return false;
    }

    total += getFromProdModel(row, "total").toDouble();

    stream << "[ICMS" + numProd + "]" << endl;
    stream << "CST = " + getFromProdModel(row, "cst").toString() << endl;
    stream << "ValorBase = " + getFromProdModel(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl; // TODO: get aliquota from model

    const double icms =
        getFromProdModel(row, "total").toDouble() * 0.18; // TODO: replace hardcoded icms from value from model
    icmsTotal += icms;
    stream << "Valor = " + QString::number(icms) << endl;
  }

  return true;
}

void CadastrarNFe::writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete) {
  stream << "[Total]" << endl;
  stream << "BaseICMS = " + QString::number(total) << endl;
  stream << "ValorICMS = " + QString::number(icmsTotal) << endl;
  stream << "ValorProduto = " + QString::number(total) << endl;
  stream << "ValorFrete = " + QString::number(ui->doubleSpinBoxFrete->value()) << endl;
  stream << "ValorNota = " + QString::number(total + frete) << endl;
}

bool CadastrarNFe::writeTXT() {
  const QString chave = criarChaveAcesso();

  chaveNum = chave;
  chaveAcesso = "NFe" + chave;

  const QString dir = getFromLoja("pastaEntACBr").toString();
  QFile file(dir + chaveNum + ".txt");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::warning(0, "Aviso!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as "
                                      "pastas estão corretamente configuradas.");
    return false;
  }

  const QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  QTextStream stream(&file);

  stream << "NFe.CriarEnviarNFe(\"" << endl;

  writeIdentificacao(stream);

  if (not writeEmitente(stream)) {
    return false;
  }

  if (not writeDestinatario(stream)) {
    return false;
  }

  double total = 0.0;
  double icmsTotal = 0.0;
  double frete = ui->doubleSpinBoxFrete->value();

  if (not writeProduto(stream, total, icmsTotal)) {
    return false;
  }

  writeTotal(stream, total, icmsTotal, frete);

  stream << "\",1,1)";

  stream.flush();
  file.close();

  return true;
}
