#include <QDate>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "reaisdelegate.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

CadastrarNFe::CadastrarNFe(QString idVenda, QWidget *parent)
    : QDialog(parent), ui(new Ui::CadastrarNFe), idVenda(idVenda) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  mapper.setModel(&modelProd);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  mapper.addMapping(ui->lineEditICMSorig, modelProd.fieldIndex("orig"));
  mapper.addMapping(ui->lineEditICMScst, modelProd.fieldIndex("cstICMS"));
  mapper.addMapping(ui->lineEditICMSmodbc, modelProd.fieldIndex("modBC"));
  mapper.addMapping(ui->doubleSpinBoxICMSvbc, modelProd.fieldIndex("vBC"));
  mapper.addMapping(ui->doubleSpinBoxICMSpicms, modelProd.fieldIndex("pICMS"));
  mapper.addMapping(ui->doubleSpinBoxICMSvicms, modelProd.fieldIndex("vICMS"));
  mapper.addMapping(ui->lineEditICMSmodbcst, modelProd.fieldIndex("modBCST"));
  mapper.addMapping(ui->lineEditICMSpmvast, modelProd.fieldIndex("pMVAST"));
  mapper.addMapping(ui->lineEditICMSvbcst, modelProd.fieldIndex("vBCST"));
  //  mapper.addMapping(ui->lineEditICMSpicmsst, modelProd.fieldIndex("pICMSST"));
  mapper.addMapping(ui->lineEditICMSvicmsst, modelProd.fieldIndex("vICMSST"));
  mapper.addMapping(ui->lineEditIPIcEnq, modelProd.fieldIndex("cEnq"));
  mapper.addMapping(ui->lineEditIPIcst, modelProd.fieldIndex("cstIPI"));
  mapper.addMapping(ui->lineEditPIScst, modelProd.fieldIndex("cstPIS"));
  mapper.addMapping(ui->doubleSpinBoxPISvbc, modelProd.fieldIndex("vBCPIS"));
  //  mapper.addMapping(ui->doubleSpinBoxPISppis, modelProd.fieldIndex("pPIS"));
  mapper.addMapping(ui->doubleSpinBoxPISvpis, modelProd.fieldIndex("vPIS"));
  mapper.addMapping(ui->lineEditCOFINScst, modelProd.fieldIndex("cstCOFINS"));
  mapper.addMapping(ui->doubleSpinBoxCOFINSvbc, modelProd.fieldIndex("vBCCOFINS"));
  //  mapper.addMapping(ui->doubleSpinBoxCOFINSpcofins, modelProd.fieldIndex("pCOFINS"));
  mapper.addMapping(ui->doubleSpinBoxCOFINSvcofins, modelProd.fieldIndex("vCOFINS"));

  ui->lineEditModelo->setInputMask("99;_");
  ui->lineEditSerie->setInputMask("999;_");
  ui->lineEditCodigo->setInputMask("99999999;_");
  ui->lineEditNumero->setInputMask("999999999;_");
  ui->lineEditTipo->setInputMask("9;_");
  ui->lineEditFormatoPagina->setInputMask("9;_");

  if (idVenda.isEmpty()) QMessageBox::critical(this, "Erro!", "idVenda vazio!");
}

CadastrarNFe::~CadastrarNFe() { delete ui; }

void CadastrarNFe::setupTables() {
  modelVenda.setTable("venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");

  if (not modelVenda.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de vendas: " + modelVenda.lastError().text());
  }

  modelProd.setTable("view_produto_estoque");
  modelProd.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProd.setHeaderData("fornecedor", "Fornecedor");
  modelProd.setHeaderData("produto", "Produto");
  modelProd.setHeaderData("obs", "Obs.");
  modelProd.setHeaderData("caixas", "Caixas");
  modelProd.setHeaderData("quant", "Quant.");
  modelProd.setHeaderData("un", "Un.");
  modelProd.setHeaderData("unCaixa", "Un./Caixa");
  modelProd.setHeaderData("codComercial", "Cód. Com.");
  modelProd.setHeaderData("formComercial", "Form. Com.");
  modelProd.setHeaderData("total", "Total");

  ui->tableItens->setModel(&modelProd);
  ui->tableItens->setItemDelegateForColumn("total", new ReaisDelegate(this));
  ui->tableItens->hideColumn("obs");
  ui->tableItens->hideColumn("status");
  ui->tableItens->hideColumn("idVenda");
  ui->tableItens->hideColumn("idProduto");
  ui->tableItens->hideColumn("prcUnitario");
  ui->tableItens->hideColumn("parcial");
  ui->tableItens->hideColumn("desconto");
  ui->tableItens->hideColumn("parcialDesc");
  ui->tableItens->hideColumn("descGlobal");
  ui->tableItens->hideColumn("idCompra");
  ui->tableItens->hideColumn("dataPrevCompra");
  ui->tableItens->hideColumn("dataRealCompra");
  ui->tableItens->hideColumn("dataPrevConf");
  ui->tableItens->hideColumn("dataRealConf");
  ui->tableItens->hideColumn("dataPrevFat");
  ui->tableItens->hideColumn("dataRealFat");
  ui->tableItens->hideColumn("dataPrevColeta");
  ui->tableItens->hideColumn("dataRealColeta");
  ui->tableItens->hideColumn("dataPrevReceb");
  ui->tableItens->hideColumn("dataRealReceb");
  ui->tableItens->hideColumn("dataPrevEnt");
  ui->tableItens->hideColumn("dataRealEnt");
  ui->tableItens->hideColumn("idNfeSaida");
  ui->tableItens->hideColumn("selecionado");
  ui->tableItens->hideColumn("idVendaProduto");
  ui->tableItens->hideColumn("idLoja");
  ui->tableItens->hideColumn("item");

  modelLoja.setTable("loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelLoja.setFilter("idLoja = " + UserSession::settings("User/lojaACBr").toString());

  if (not modelLoja.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de lojas: " + modelLoja.lastError().text());
  }
}

void CadastrarNFe::guardarNotaBD() {
  QFile fileResposta(UserSession::settings("User/pastaSaiACBr").toString() + "/" + chaveNum + "-resp.txt");

  QProgressDialog *progressDialog = new QProgressDialog(this);
  progressDialog->reset();
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Esperando ACBr...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMaximum(0);
  progressDialog->setMinimum(0);
  progressDialog->show();

  QTime wait = QTime::currentTime().addSecs(10);

  while (QTime::currentTime() < wait) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if (fileResposta.exists()) break;
  }

  progressDialog->cancel();

  if (not fileResposta.exists()) {
    QMessageBox::critical(this, "Erro!", "ACBr não respondeu, verificar se ele está aberto e funcionando!");

    QFile entrada(UserSession::settings("User/pastaEntACBr").toString() + "/" + chaveNum + ".txt");

    if (entrada.exists()) entrada.remove();

    return;
  }

  if (not fileResposta.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + fileResposta.errorString());
    return;
  }

  QString resposta = fileResposta.readAll();
  fileResposta.remove();

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Erro!", "Resposta do ACBr: " + resposta);
    return;
  }

  QMessageBox::information(this, "Aviso!", "Resposta do ACBr: " + resposta);

  QFile file(UserSession::settings("User/pastaXmlACBr").toString() + "/" + chaveNum + "-nfe.xml");

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  QSqlQuery queryNota;
  queryNota.prepare("INSERT INTO nfe (idVenda, tipo, xml, status, chaveAcesso) VALUES (:idVenda, :tipo, "
                    ":xml, :status, :chaveAcesso)");
  queryNota.bindValue(":idVenda", idVenda);
  queryNota.bindValue(":tipo", "SAÍDA");
  queryNota.bindValue(":xml", file.readAll());
  queryNota.bindValue(":status", "PENDENTE");
  queryNota.bindValue(":chaveAcesso", chaveNum);

  if (not queryNota.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando nota: " + queryNota.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Nota guardada com sucesso!");

  QVariant id = queryNota.lastInsertId();

  QSqlQuery queryVenda;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    queryVenda.prepare("UPDATE venda_has_produto SET idNfeSaida = :idNfeSaida WHERE idVendaProduto = :idVendaProduto");
    queryVenda.bindValue(":idNfeSaida", id);
    queryVenda.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not queryVenda.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando NFe nos produtos: " + queryVenda.lastError().text());
      return;
    }
  }
}

void CadastrarNFe::on_pushButtonEnviarNFE_clicked() {
  if (not writeTXT()) return;

  QMessageBox::information(this, "Aviso!", "NFe enviada para ACBr!");
  guardarNotaBD();
  close();
}

void CadastrarNFe::updateImpostos() {
  double baseICMS = 0;
  double valorICMS = 0;
  //  double valorIPI = 0;
  double valorPIS = 0;
  double valorCOFINS = 0;
  double total = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    baseICMS += modelProd.data(row, "total").toDouble();
    valorICMS += modelProd.data(row, "total").toDouble() * modelProd.data(row, "pICMS").toDouble() / 100.;
    //    valorIPI += modelProd.data(row, "vIPI").toDouble();
    valorPIS += modelProd.data(row, "vPIS").toDouble();
    valorCOFINS += modelProd.data(row, "vCOFINS").toDouble();
  }

  total = valorICMS + /*valorIPI +*/ valorPIS + valorCOFINS;

  ui->doubleSpinBoxBaseICMS->setValue(baseICMS);
  ui->doubleSpinBoxValorICMS->setValue(valorICMS);
  //  ui->doubleSpinBoxValorIPI->setValue(valorIPI);
  ui->doubleSpinBoxValorPIS->setValue(valorPIS);
  ui->doubleSpinBoxValorCOFINS->setValue(valorCOFINS);

  QSqlQuery queryEnd;
  queryEnd.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf FROM cliente_has_endereco WHERE "
                   "idCliente = :idCliente");
  queryEnd.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

  if (not queryEnd.exec() or not queryEnd.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço: " + queryEnd.lastError().text());
    return;
  }

  QString endereco = queryEnd.value("logradouro").toString() + ", " + queryEnd.value("numero").toString() + " " +
                     queryEnd.value("complemento").toString() + " - " + queryEnd.value("bairro").toString() + " - " +
                     queryEnd.value("cidade").toString() + " - " + queryEnd.value("uf").toString();

  QString texto = "Venda de código " + modelVenda.data(0, "idVenda").toString() + ";END. ENTREGA: " + endereco +
                  ";Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO "
                  "3113Y;Total Aproximado de tributos federais, estaduais e municipais: " +
                  QString::number(total);
  ui->textEdit->setPlainText(texto);
}

void CadastrarNFe::prepararNFe(const QList<int> &items) {
  QString filter;

  for (auto const &item : items) {
    filter += QString(filter.isEmpty() ? "" : " OR ") + "idVendaProduto = " + QString::number(item);
  }

  modelProd.setFilter(filter);

  if (not modelProd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de itens da venda: " + modelProd.lastError().text());
    return;
  }

  ui->tableItens->resizeColumnsToContents();

  QSqlQuery queryNfe;

  if (not queryNfe.exec("SELECT MAX(idNFe) AS idNFe FROM nfe")) {
    QMessageBox::critical(this, "Erro!", "Erro buscando idNFe: " + queryNfe.lastError().text());
    return;
  }

  const int id = queryNfe.first() ? queryNfe.value("idNFe").toInt() + 1 : 1;

  ui->lineEditNumero->setText(QString("%1").arg(id, 9, 10, QChar('0')));
  ui->lineEditCodigo->setText(QString("%1").arg(id, 8, 10, QChar('0')));

  ui->lineEditNatureza->setText("VENDA");
  ui->lineEditModelo->setText("55");
  ui->lineEditSerie->setText("001");
  ui->lineEditEmissao->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditSaida->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditTipo->setText("1");
  ui->lineEditFormatoPagina->setText("0");

  //-----------------------

  QSqlQuery queryEmitente;
  queryEmitente.prepare(
      "SELECT razaoSocial, nomeFantasia, cnpj, inscEstadual, tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryEmitente.bindValue(":idLoja", UserSession::settings("User/lojaACBr"));

  if (not queryEmitente.exec() or not queryEmitente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo dados do emitente: " + queryEmitente.lastError().text());
    return;
  }

  ui->lineEditEmitenteNomeRazao->setText(queryEmitente.value("razaoSocial").toString());
  ui->lineEditEmitenteFantasia->setText(queryEmitente.value("nomeFantasia").toString());
  ui->lineEditEmitenteCNPJ->setText(queryEmitente.value("cnpj").toString());
  ui->lineEditEmitenteInscEstadual->setText(queryEmitente.value("inscEstadual").toString());
  ui->lineEditEmitenteTel1->setText(queryEmitente.value("tel").toString());
  ui->lineEditEmitenteTel2->setText(queryEmitente.value("tel2").toString());

  QSqlQuery queryEmitenteEndereco;
  queryEmitenteEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM "
                                "loja_has_endereco WHERE idLoja = :idLoja");
  queryEmitenteEndereco.bindValue(":idLoja", UserSession::settings("User/lojaACBr"));

  if (not queryEmitenteEndereco.exec() or not queryEmitenteEndereco.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo endereço do emitente: " + queryEmitenteEndereco.lastError().text());
    return;
  }

  ui->lineEditEmitenteLogradouro->setText(queryEmitenteEndereco.value("logradouro").toString());
  ui->lineEditEmitenteNumero->setText(queryEmitenteEndereco.value("numero").toString());
  ui->lineEditEmitenteComplemento->setText(queryEmitenteEndereco.value("complemento").toString());
  ui->lineEditEmitenteBairro->setText(queryEmitenteEndereco.value("bairro").toString());
  ui->lineEditEmitenteCidade->setText(queryEmitenteEndereco.value("cidade").toString());
  ui->lineEditEmitenteUF->setText(queryEmitenteEndereco.value("uf").toString());
  ui->lineEditEmitenteCEP->setText(queryEmitenteEndereco.value("cep").toString());

  //-----------------------

  QSqlQuery queryDestinatario;
  queryDestinatario.prepare(
      "SELECT nome_razao, pfpj, cpf, cnpj, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  queryDestinatario.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

  if (not queryDestinatario.exec() or not queryDestinatario.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo dados do cliente: " + queryDestinatario.lastError().text());
    return;
  }

  ui->lineEditDestinatarioNomeRazao->setText(queryDestinatario.value("nome_razao").toString());
  ui->lineEditDestinatarioCPFCNPJ->setText(
      queryDestinatario.value(queryDestinatario.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString());
  ui->lineEditDestinatarioTel1->setText(queryDestinatario.value("tel").toString());
  ui->lineEditDestinatarioTel2->setText(queryDestinatario.value("telCel").toString());

  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM "
                                    "cliente_has_endereco WHERE idCliente = :idCliente");
  queryDestinatarioEndereco.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
    return;
  }

  ui->lineEditDestinatarioLogradouro->setText(queryDestinatarioEndereco.value("logradouro").toString());
  ui->lineEditDestinatarioNumero->setText(queryDestinatarioEndereco.value("numero").toString());
  ui->lineEditDestinatarioComplemento->setText(queryDestinatarioEndereco.value("complemento").toString());
  ui->lineEditDestinatarioBairro->setText(queryDestinatarioEndereco.value("bairro").toString());
  ui->lineEditDestinatarioCidade->setText(queryDestinatarioEndereco.value("cidade").toString());
  ui->lineEditDestinatarioUF->setText(queryDestinatarioEndereco.value("uf").toString());
  ui->lineEditDestinatarioCEP->setText(queryDestinatarioEndereco.value("cep").toString());
  //-----------------------

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    modelProd.setData(row, "pPIS", 0.65);
    modelProd.setData(row, "vPIS",
                      modelProd.data(row, "vBCPIS").toDouble() * modelProd.data(row, "pPIS").toDouble() / 100);
    modelProd.setData(row, "pCOFINS", 3);
    modelProd.setData(row, "vCOFINS",
                      modelProd.data(row, "vBCCOFINS").toDouble() * modelProd.data(row, "pCOFINS").toDouble() / 100);
  }

  //-----------------------

  double valorProdutos = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    valorProdutos += modelProd.data(row, "total").toDouble();
  }

  const double frete =
      valorProdutos / modelVenda.data(0, "subTotalLiq").toDouble() * modelVenda.data(0, "frete").toDouble();

  ui->doubleSpinBoxValorProduto->setValue(valorProdutos);
  ui->doubleSpinBoxValorFrete->setValue(frete);
  ui->doubleSpinBoxValorNota->setValue(valorProdutos + frete);

  updateImpostos();
}

QString CadastrarNFe::criarChaveAcesso() {
  QStringList listChave;

  listChave << modelLoja.data(0, "codUF").toString();
  listChave << QDate::currentDate().toString("yyMM");      // Ano/Mês
  listChave << clearStr(ui->lineEditEmitenteCNPJ->text()); // CNPJ
  listChave << ui->lineEditModelo->text();                 // modelo NFe
  listChave << ui->lineEditSerie->text();
  listChave << ui->lineEditNumero->text(); // número nf-e (id interno)
  listChave << ui->lineEditTipo->text();   // tpEmis - forma de emissão
  listChave << ui->lineEditCodigo->text(); // código númerico aleatorio

  QString chave = listChave.join("");
  chave += calculaDigitoVerificador(chave);

  return chave;
}

QString CadastrarNFe::clearStr(const QString &str) const {
  return QString(str).remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

QString CadastrarNFe::removeDiacritics(const QString &str) const {
  return str == "M²" ? "M2" : QString(str).normalized(QString::NormalizationForm_KD).remove(QRegExp("[^a-zA-Z\\s]"));
}

QString CadastrarNFe::calculaDigitoVerificador(const QString &chave) {
  if (chave.size() != 43) {
    QMessageBox::critical(this, "Erro!", "Erro no tamanho da chave: " + chave);
    return QString();
  }

  QVector<int> chave2;

  for (int i = 0, size = chave.size(); i < size; ++i) {
    chave2 << chave.at(i).digitValue();
  }

  const QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                        6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  int soma = 0;

  for (int i = 0; i < 43; ++i) {
    soma += chave2.at(i) * multiplicadores.at(i);
  }

  const int resto = soma % 11;

  return QString::number(resto < 2 ? 0 : 11 - resto);
}

void CadastrarNFe::writeIdentificacao(QTextStream &stream) const {
  stream << "[Identificacao]" << endl;
  stream << "NaturezaOperacao = " + ui->lineEditNatureza->text() << endl;
  stream << "Modelo = " + ui->lineEditModelo->text() << endl;
  stream << "Serie = " + ui->lineEditSerie->text() << endl;
  stream << "Codigo = " << ui->lineEditCodigo->text() << endl; // 1
  stream << "Numero = " << ui->lineEditNumero->text() << endl; // 1
  stream << "Emissao = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Saida = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Tipo = " + ui->lineEditTipo->text() << endl;
  stream << "FormaPag = " + ui->lineEditFormatoPagina->text() << endl;
  stream << "indPres = 1" << endl;
  stream << "indFinal = 1" << endl;
}

bool CadastrarNFe::writeEmitente(QTextStream &stream) {
  stream << "[Emitente]" << endl;

  if (clearStr(modelLoja.data(0, "cnpj").toString()).isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CNPJ emitente vazio!");
    return false;
  }

  stream << "CNPJ = " + clearStr(modelLoja.data(0, "cnpj").toString()) << endl;

  stream << "IE = " + modelLoja.data(0, "inscEstadual").toString() << endl;

  if (modelLoja.data(0, "razaoSocial").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Razão Social emitente vazio!");
    return false;
  }

  stream << "Razao = " + removeDiacritics(modelLoja.data(0, "razaoSocial").toString()) << endl;

  if (modelLoja.data(0, "nomeFantasia").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nome Fantasia emitente vazio!");
    return false;
  }

  stream << "Fantasia = " + removeDiacritics(modelLoja.data(0, "nomeFantasia").toString()) << endl;

  if (modelLoja.data(0, "tel").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Telefone emitente vazio!");
    return false;
  }

  stream << "Fone = " + modelLoja.data(0, "tel").toString() << endl;

  QSqlQuery queryLojaEnd;
  queryLojaEnd.prepare(
      "SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", modelLoja.data(0, "idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de endereços da loja: " + queryLojaEnd.lastError().text());
    return false;
  }

  if (clearStr(queryLojaEnd.value("CEP").toString()).isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CEP vazio!");
    return false;
  }

  stream << "CEP = " + clearStr(queryLojaEnd.value("CEP").toString()) << endl;

  if (queryLojaEnd.value("logradouro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Logradouro vazio!");
    return false;
  }

  stream << "Logradouro = " + removeDiacritics(queryLojaEnd.value("logradouro").toString()) << endl;

  if (queryLojaEnd.value("numero").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Número vazio!");
    return false;
  }

  stream << "Numero = " + queryLojaEnd.value("numero").toString() << endl;

  stream << "Complemento = " + removeDiacritics(queryLojaEnd.value("complemento").toString()) << endl;

  if (queryLojaEnd.value("bairro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Bairro vazio!");
    return false;
  }

  stream << "Bairro = " + removeDiacritics(queryLojaEnd.value("bairro").toString()) << endl;

  if (queryLojaEnd.value("cidade").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cidade vazio!");
    return false;
  }

  stream << "Cidade = " + removeDiacritics(queryLojaEnd.value("cidade").toString()) << endl;

  if (queryLojaEnd.value("uf").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "UF vazio!");
    return false;
  }

  stream << "UF = " + queryLojaEnd.value("uf").toString() << endl;

  return true;
}

bool CadastrarNFe::writeDestinatario(QTextStream &stream) {
  stream << "[Destinatario]" << endl;

  const QString idCliente = modelVenda.data(0, "idCliente").toString();

  if (idCliente.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "idCliente vazio!");
    return false;
  }

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT nome_razao, pfpj, cpf, cnpj, inscEstadual, tel, cep, logradouro, numero, complemento, "
                       "bairro, cidade, uf FROM cliente AS c LEFT JOIN cliente_has_endereco AS e ON c.idCliente = "
                       "e.idCliente WHERE e.idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço do destinatário: " + queryCliente.lastError().text());
    return false;
  }

  if (queryCliente.value("nome_razao").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nome/Razão vazio!");
    return false;
  }

  stream << "NomeRazao = " + removeDiacritics(queryCliente.value("nome_razao").toString()) << endl;

  if (queryCliente.value("pfpj").toString() == "PF") {
    if (clearStr(queryCliente.value("cpf").toString()).isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CPF vazio!");
      return false;
    }

    stream << "CPF = " + clearStr(queryCliente.value("cpf").toString()) << endl;

    stream << "indIEDest = 9" << endl;
  }

  if (queryCliente.value("pfpj").toString() == "PJ") {
    if (clearStr(queryCliente.value("cnpj").toString()).isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CNPJ destinatário vazio!");
      return false;
    }

    stream << "CNPJ = " + clearStr(queryCliente.value("cnpj").toString()) << endl;

    if (queryCliente.value("inscEstadual").toString() == "ISENTO") {
      // TODO: informar tag indIEDest no caso de nao haver IE (2 ou 9?) (pedir inscricao ou ISENTO no cadastro)
      stream << "indIEDest = 2" << endl;
    } else {
      stream << "IE = " + clearStr(queryCliente.value("inscEstadual").toString()) << endl;
    }
  }

  stream << "Fone = " + queryCliente.value("tel").toString() << endl;

  if (queryCliente.value("CEP").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CEP cliente vazio!");
    return false;
  }

  stream << "CEP = " + clearStr(queryCliente.value("CEP").toString()) << endl;

  if (queryCliente.value("logradouro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Logradouro cliente vazio!");
    return false;
  }

  stream << "Logradouro = " + removeDiacritics(queryCliente.value("logradouro").toString()) << endl;

  if (queryCliente.value("numero").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Número cliente vazio!");
    return false;
  }

  stream << "Numero = " + queryCliente.value("numero").toString() << endl;

  stream << "Complemento = " + removeDiacritics(queryCliente.value("complemento").toString()) << endl;

  if (queryCliente.value("bairro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Bairro cliente vazio!");
    return false;
  }

  stream << "Bairro = " + removeDiacritics(queryCliente.value("bairro").toString()) << endl;

  if (queryCliente.value("cidade").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cidade cliente vazio!");
    return false;
  }

  stream << "Cidade = " + removeDiacritics(queryCliente.value("cidade").toString()) << endl;

  if (queryCliente.value("uf").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "UF cliente vazio!");
    return false;
  }

  stream << "UF = " + queryCliente.value("uf").toString() << endl;

  return true;
}

bool CadastrarNFe::writeProduto(QTextStream &stream) {
  for (int row = 0; row < modelProd.rowCount(); ++row) {
    const QString numProd = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding with zeros
    stream << "[Produto" + numProd + "]" << endl;

    if (modelProd.data(row, "cfop").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CFOP vazio!");
      return false;
    }

    stream << "CFOP = " + modelProd.data(row, "cfop").toString() << endl;
    //    stream << "CFOP = 5101" << endl;

    if (modelProd.data(row, "ncm").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "NCM vazio!");
      return false;
    }

    stream << "NCM = " + modelProd.data(row, "ncm").toString() << endl;
    //    stream << "NCM = 69079000" << endl;

    if (modelProd.data(row, "codComercial").toString().isEmpty()) {
      //      stream << "Codigo = 0000000000000" << endl;
      QMessageBox::critical(this, "Erro!", "Código vazio!");
      return false;
    }

    stream << "Codigo = " + modelProd.data(row, "codComercial").toString() << endl;

    if (modelProd.data(row, "codBarras").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CodBarras vazio!");
      return false;
    }

    stream << "EAN = " + modelProd.data(row, "codBarras").toString() << endl;

    if (modelProd.data(row, "produto").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Descrição vazio!");
      return false;
    }

    stream << "Descricao = " + removeDiacritics(modelProd.data(row, "produto").toString()) << endl;

    if (modelProd.data(row, "un").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Unidade vazio!");
      return false;
    }

    stream << "Unidade = " + removeDiacritics(modelProd.data(row, "un").toString()) << endl;

    if (modelProd.data(row, "quant").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Quantidade vazio!");
      return false;
    }

    stream << "Quantidade = " + modelProd.data(row, "quant").toString() << endl;

    if (modelProd.data(row, "prcUnitario").toDouble() == 0) {
      QMessageBox::critical(this, "Erro!", "Preço venda = 0!");
      return false;
    }

    stream << "ValorUnitario = " +
                  QString::number(modelProd.data(row, "total").toDouble() / modelProd.data(row, "quant").toDouble())
           << endl;

    if (modelProd.data(row, "total").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Total produto vazio!");
      return false;
    }

    stream << "ValorTotal = " + modelProd.data(row, "total").toString() << endl;

    const double frete = modelProd.data(row, "total").toDouble() / ui->doubleSpinBoxValorProduto->value() *
                         ui->doubleSpinBoxValorFrete->value();

    stream << "vFrete = " + QString::number(frete) << endl;

    stream << "[ICMS" + numProd + "]" << endl;
    stream << "CST = " + modelProd.data(row, "cstICMS").toString().remove(0, 1) << endl;
    stream << "ValorBase = " + modelProd.data(row, "total").toString() << endl;
    double aliquota = modelProd.data(row, "pICMS").toDouble();
    stream << "Aliquota = " + QString::number(aliquota, 'f', 2) << endl;
    const double icms = modelProd.data(row, "total").toDouble() * aliquota / 100;
    stream << "Valor = " + QString::number(icms) << endl;

    stream << "[IPI" + numProd + "]" << endl;
    stream << "ClasseEnquadramento = " + modelProd.data(row, "cEnq").toString() << endl;
    stream << "CST = " + modelProd.data(row, "cstIPI").toString() << endl;

    stream << "[PIS" + numProd + "]" << endl;
    stream << "CST = " + modelProd.data(row, "cstPIS").toString() << endl;
    stream << "ValorBase = " + modelProd.data(row, "vBCPIS").toString() << endl;
    stream << "Aliquota = " + modelProd.data(row, "pPIS").toString() << endl;
    stream << "Valor = " + modelProd.data(row, "vPIS").toString() << endl;

    stream << "[COFINS" + numProd + "]" << endl;
    stream << "CST = " + modelProd.data(row, "cstCOFINS").toString() << endl;
    stream << "ValorBase = " + modelProd.data(row, "vBCCOFINS").toString() << endl;
    stream << "Aliquota = " + modelProd.data(row, "pCOFINS").toString() << endl;
    stream << "Valor = " + modelProd.data(row, "vCOFINS").toString() << endl;
  }

  return true;
}

void CadastrarNFe::writeTotal(QTextStream &stream) const {
  stream << "[Total]" << endl;
  stream << "BaseICMS = " + QString::number(ui->doubleSpinBoxBaseICMS->value(), 'f', 2) << endl;
  stream << "ValorICMS = " + QString::number(ui->doubleSpinBoxValorICMS->value(), 'f', 2) << endl;
  stream << "ValorIPI = " + QString::number(ui->doubleSpinBoxValorIPI->value(), 'f', 2) << endl;
  stream << "ValorPIS = " + QString::number(ui->doubleSpinBoxValorPIS->value(), 'f', 2) << endl;
  stream << "ValorCOFINS = " + QString::number(ui->doubleSpinBoxValorCOFINS->value(), 'f', 2) << endl;
  stream << "ValorProduto = " + QString::number(ui->doubleSpinBoxValorProduto->value(), 'f', 2) << endl;
  stream << "ValorFrete = " + QString::number(ui->doubleSpinBoxValorFrete->value(), 'f', 2) << endl;
  stream << "ValorNota = " + QString::number(ui->doubleSpinBoxValorNota->value(), 'f', 2) << endl;
}

bool CadastrarNFe::writeTXT(bool test) {
  chaveNum = criarChaveAcesso();

  const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();

  QFile file(dirEntrada + "/" + chaveNum + ".txt");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  const QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  QTextStream stream(&file);

  stream << (test ? "NFE.CriarNFe(\"" : "NFE.CriarEnviarNFe(\"") << endl;

  writeIdentificacao(stream);
  if (not writeEmitente(stream)) return false;
  if (not writeDestinatario(stream)) return false;
  if (not writeProduto(stream)) return false;
  writeTotal(stream);

  stream << "[DadosAdicionais]" << endl;
  stream << "infCpl = " + ui->textEdit->toPlainText() << endl;

  //  stream << "\")";
  //  stream << "\",1,1)";
  stream << "\",0)";

  stream.flush();
  file.close();

  return true;
}

void CadastrarNFe::on_pushButtonGerarNFE_clicked() {
  writeTXT(true);

  QMessageBox::information(this, "Aviso!", "NFe enviada para ACBr!");
  guardarNotaBD();
  close();
}

void CadastrarNFe::on_tableItens_entered(const QModelIndex &) { ui->tableItens->resizeColumnsToContents(); }

void CadastrarNFe::on_tableItens_clicked(const QModelIndex &index) { mapper.setCurrentModelIndex(index); }

void CadastrarNFe::on_tabWidget_currentChanged(int index) {
  if (index == 4) updateImpostos();
}

// TODO: verificar se estou guardando a segunda nota, a que a receita devolve com o codigo de autorizacao

void CadastrarNFe::on_doubleSpinBoxICMSvbc_valueChanged(double) {
  ui->doubleSpinBoxICMSvicms->setValue(ui->doubleSpinBoxICMSvbc->value() * ui->doubleSpinBoxICMSpicms->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxICMSpicms_valueChanged(double) {
  ui->doubleSpinBoxICMSvicms->setValue(ui->doubleSpinBoxICMSvbc->value() * ui->doubleSpinBoxICMSpicms->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxPISvbc_valueChanged(double) {
  ui->doubleSpinBoxPISvpis->setValue(ui->doubleSpinBoxPISvbc->value() * ui->doubleSpinBoxPISppis->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxPISppis_valueChanged(double) {
  ui->doubleSpinBoxPISvpis->setValue(ui->doubleSpinBoxPISvbc->value() * ui->doubleSpinBoxPISppis->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxCOFINSvbc_valueChanged(double) {
  ui->doubleSpinBoxCOFINSvcofins->setValue(ui->doubleSpinBoxCOFINSvbc->value() *
                                           ui->doubleSpinBoxCOFINSpcofins->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxCOFINSpcofins_valueChanged(double) {
  ui->doubleSpinBoxCOFINSvcofins->setValue(ui->doubleSpinBoxCOFINSvbc->value() *
                                           ui->doubleSpinBoxCOFINSpcofins->value() / 100);
}

// TODO: concatenar caixas na descricao do produto
// TODO: verificar mapper nao funcionando
// TODO: puxar ncm do cadastro do produto
// TODO: se a nota retornar rejeicao guardar nota mas nao guardar no produto
// TODO: pedir IE para realizar venda para
// TODO: ao dar erro nao fechar tela
// TODO: na tela antes dessa, colocar para marcar linha inteira
// TODO: na tela antes dessa, poder abrir nota direta
// TODO: checkbox com opcao de mandar email com xml e danfe para o cliente
// TODO: armazenar danfe em algum lugar e verificar como gerar ela novamente a partir do xml
