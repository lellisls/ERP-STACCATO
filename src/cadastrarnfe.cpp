#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

CadastrarNFe::CadastrarNFe(QString idVenda, QWidget *parent)
  : QDialog(parent), ui(new Ui::CadastrarNFe), idVenda(idVenda) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  if (idVenda.isEmpty()) QMessageBox::critical(this, "Erro!", "idVenda vazio!");

  setupTables(idVenda);
}

CadastrarNFe::~CadastrarNFe() { delete ui; }

void CadastrarNFe::setupTables(QString idVenda) {
  modelVenda.setTable("venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");

  if (not modelVenda.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de vendas: " + modelVenda.lastError().text());
  }

  modelProd.setTable("venda_has_produto");
  modelProd.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableItens->setModel(&modelProd);
  ui->tableItens->hideColumn("idNfeSaida");
  ui->tableItens->hideColumn("selecionado");
  ui->tableItens->hideColumn("idVendaProduto");
  ui->tableItens->hideColumn("idLoja");
  ui->tableItens->hideColumn("item");

  modelLoja.setTable("loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelLoja.setFilter("descricao = 'ALPHAVILLE'");

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

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Erro!", "Resposta do ACBr: " + resposta);
    return;
  }

  QMessageBox::information(this, "Aviso!", "Resposta do ACBr: " + resposta);
  fileResposta.remove();

  QFile file(UserSession::settings("User/pastaXmlACBr").toString() + "/" + chaveNum + "-nfe.xml");

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  QSqlQuery queryNota;
  queryNota.prepare("INSERT INTO nfe (idCompra, tipo, xml, status, chaveAcesso) VALUES (:idCompra, :tipo, "
                    ":xml, :status, :chaveAcesso)");
  queryNota.bindValue(":idCompra", "PLACEHOLDER"); // TODO: replace placeholder
  queryNota.bindValue(":tipo", "SAÍDA");
  queryNota.bindValue(":xml", file.readAll());
  queryNota.bindValue(":status", "PENDENTE");
  queryNota.bindValue(":chaveAcesso", chaveAcesso.remove("NFE"));

  if (not queryNota.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando nota: " + queryNota.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Nota guardada com sucesso!");

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    if (not modelProd.setData(row, "idNfeSaida", queryNota.lastInsertId())) return;
  }

  if (not modelProd.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados da tabela itens da venda: " + modelProd.lastError().text());
    return;
  }
}

void CadastrarNFe::on_pushButtonEnviarNFE_clicked() {
  if (not writeTXT()) return;

  QMessageBox::information(this, "Aviso!", "NFe enviada para ACBr!");
  guardarNotaBD();
  close();
}

void CadastrarNFe::updateImpostos() {
  // FIXME: fix icms
  //  double icms = 0;

  //  for (int row = 0; row < modelProd.rowCount(); ++row) {
  //    icms += modelProd.data(row, "valorICMS").toDouble();
  //  }

  //  ui->doubleSpinBoxVlICMS->setValue(icms);

  //  const double imposto = 0.3 * ui->doubleSpinBoxFinal->value() + icms;
  const double imposto = 0;

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

  QString texto = "Venda de código " + modelVenda.data(0, "idVenda").toString() + "\n END. ENTREGA: " + endereco +
                  "\nInformações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO "
                  "3113Y\nTotal Aproximado de tributos federais, estaduais e municipais: " +
                  QString::number(imposto);
  ui->textEdit->setPlainText(texto);

  // TODO: guardar texto na 'obs' da nfe
}

void CadastrarNFe::prepararNFe(const QList<int> &items) {
  // TODO: make a query using idVenda to get info for cliente and loja
  // TODO: calcular frete proporcional

  chaveNum = criarChaveAcesso();
  chaveAcesso = "NFE" + chaveNum;

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

  ui->lineEditNatureza->setText("VENDA");
  ui->lineEditModelo->setText("55");
  ui->lineEditSerie->setText("001");
  //  ui->lineEditCodigo->setText(); // set by criarChaveAcesso
  //  ui->lineEditNumero->setText(); // set by criarChaveAcesso
  ui->lineEditEmissao->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditSaida->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditTipo->setText("1");
  ui->lineEditFormatoPagina->setText("0");

  //-----------------------

  //  QSqlQuery queryVenda;
  //  queryVenda.prepare("SELECT frete, total, idLoja, idCliente, idEnderecoFaturamento, descontoPorc, subTotalLiq FROM
  //  "
  //                     "venda WHERE idVenda = :idVenda");
  //  queryVenda.bindValue(":idVenda", idVenda);

  //  if (not queryVenda.exec() or not queryVenda.first()) {
  //    QMessageBox::critical(this, "Erro!", "Erro lendo tabela vendas: " + queryVenda.lastError().text());
  //    return;
  //  }

  //  if (not modelNFe.select()) {
  //    QMessageBox::critical(this, "Erro!", "Erro: " + modelNFe.lastError().text());
  //    return;
  //  }

  //  const int row = modelNFe.rowCount();
  //  modelNFe.insertRow(row);
  //  mapperNFe.toLast();

  //  modelNFe.setData(mapperNFe.currentIndex(), "idVenda", idVenda);
  //  modelNFe.setData(mapperNFe.currentIndex(), "frete", queryVenda.value("frete"));
  //  modelNFe.setData(mapperNFe.currentIndex(), "total", queryVenda.value("total"));
  //  modelNFe.setData(mapperNFe.currentIndex(), "idLoja", queryVenda.value("idLoja"));
  //  modelNFe.setData(mapperNFe.currentIndex(), "idCliente", queryVenda.value("idCliente"));
  //  modelNFe.setData(mapperNFe.currentIndex(), "idEnderecoFaturamento", queryVenda.value("idEnderecoFaturamento"));

  //  ui->doubleSpinBoxFinal->setValue(queryVenda.value("total").toDouble());
  //  ui->doubleSpinBoxFrete->setValue(queryVenda.value("frete").toDouble());
  //  ui->itemBoxCliente->setValue(queryVenda.value("idCliente"));
  //  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " +
  //  QString::number(ui->itemBoxCliente->value().toInt()) +
  //                                                 " AND desativado = FALSE");
  //  ui->itemBoxEndereco->setValue(queryVenda.value("idEnderecoFaturamento"));

  //  const double descontoGlobal = queryVenda.value("descontoPorc").toDouble();

  //  double frete = 0;
  //  double totalFinal = 0;

  //  for (const auto item : items) {
  //    QSqlQuery queryItens;
  //    queryItens.prepare("SELECT cst, codComercial, produto, ncm, un, quant, prcUnitario, total FROM venda_has_produto
  //    "
  //                       "NATURAL LEFT JOIN produto WHERE idVenda = :idVenda AND item = :item");
  //    queryItens.bindValue(":idVenda", idVenda);
  //    queryItens.bindValue(":item", item);

  //    if (not queryItens.exec() or not queryItens.first()) {
  //      QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + queryItens.lastError().text());
  //      return;
  //    }

  //    const int row = modelNFeItem.rowCount();
  //    modelNFeItem.insertRow(row);
  //    setItemData(row, "item", item + 1);
  //    setItemData(row, "cst", queryItens.value("cst"));
  //    setItemData(row, "cfop", "5405"); // TODO: get CFOP from NCM<>CFOP table
  //    setItemData(row, "codComercial", queryItens.value("codComercial"));
  //    setItemData(row, "descricao", queryItens.value("produto"));
  //    setItemData(row, "ncm", queryItens.value("ncm"));
  //    setItemData(row, "un", queryItens.value("un"));
  //    setItemData(row, "quant", queryItens.value("quant"));
  //    setItemData(row, "valorUnitario", queryItens.value("prcUnitario"));

  //    const double total = queryItens.value("total").toDouble() * (1. - (descontoGlobal / 100.));
  //    setItemData(row, "valorTotal", total);

  //    totalFinal += queryItens.value("total").toDouble();
  //  }

  //  frete = totalFinal / queryVenda.value("subTotalLiq").toDouble() * queryVenda.value("frete").toDouble();
  //  totalFinal += frete;

  //  ui->doubleSpinBoxFrete->setValue(frete);
  //  ui->doubleSpinBoxFinal->setValue(totalFinal);

  //  ui->tableItens->resizeColumnsToContents();
  //  updateImpostos();

  //  chaveAcesso = "NFe" + criarChaveAcesso();

  //  ui->lineEditChave->setText(criarChaveAcesso());
  //  ui->lineEditNatureza->setText("VENDA");
  //  ui->lineEditModelo->setText("55");
  //  ui->lineEditSerie->setText(chaveAcesso.mid(25, 3));
  //  ui->lineEditCodigo->setText(chaveAcesso.mid(25, 3));
  //  ui->lineEditNumero->setText(chaveAcesso.mid(28, 9));
  //  ui->lineEditEmissao->setText(QDate::currentDate().toString("dd/MM/yyyy"));
  //  ui->lineEditSaida->setText(QDate::currentDate().toString("dd/MM/yyyy"));
  //  ui->lineEditTipo->setText("1");
  //  ui->lineEditFormatoPagina->setText("0");
}

void CadastrarNFe::on_tableItens_activated(const QModelIndex &) { updateImpostos(); }

void CadastrarNFe::on_tableItens_pressed(const QModelIndex &) { updateImpostos(); }

QString CadastrarNFe::criarChaveAcesso() {
  QSqlQuery query;

  if (not query.exec("SELECT idNFe FROM nfe ORDER BY idNFe DESC LIMIT 1")) {
    QMessageBox::critical(this, "Erro!", "Erro buscando idNFe: " + query.lastError().text());
    return QString();
  }

  const int id = query.first() ? query.value("idNFe").toInt() + 1 : 1;

  ui->lineEditCodigo->setText(QString("%1").arg(id, 9, 10, QChar('0')));
  ui->lineEditNumero->setText(QString("%1").arg(id, 9, 10, QChar('0')));

  QStringList listChave;

  listChave << modelLoja.data(0, "codUF").toString();

  listChave << QDate::currentDate().toString("yyMM"); // Ano/Mês

  const QString cnpj = clearStr(modelLoja.data(0, "cnpj").toString());
  listChave << cnpj; // CNPJ

  listChave << "55"; // modelo NFe

  QString serie = "1";
  serie = QString("%1").arg(serie.toInt(), 3, 10, QChar('0'));
  listChave << serie; // série

  listChave << QString("%1").arg(id, 9, 10, QChar('0')); // número nf-e (id interno)
  listChave << "1";                                      // tpEmis - forma de emissão
  listChave << QString("%1").arg(id, 8, 10, QChar('0')); // código númerico aleatorio

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
  // TODO: get those values from gui
  stream << "[Identificacao]" << endl;
  stream << "NaturezaOperacao = VENDA" << endl;
  stream << "Modelo = 55" << endl;
  stream << "Serie = 001" << endl;
  stream << "Codigo = " << ui->lineEditCodigo->text() << endl; // 1
  stream << "Numero = " << ui->lineEditNumero->text() << endl; // 1
  stream << "Emissao = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Saida = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Tipo = 1" << endl;
  stream << "FormaPag = 0" << endl;
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
    stream << "IE = " + clearStr(queryCliente.value("inscEstadual").toString()) << endl;

    if (queryCliente.value("inscEstadual").toString() == "ISENTO") stream << "indIEDest = 2" << endl;
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

bool CadastrarNFe::writeProduto(QTextStream &stream, double &total, double &icmsTotal) {
  for (int row = 0; row < modelProd.rowCount(); ++row) {
    QSqlQuery queryProd;
    queryProd.prepare(
          "SELECT codComercial, codBarras, descricao, un, precoVenda FROM produto WHERE idProduto = :idProduto");
    queryProd.bindValue(":idProduto", modelProd.data(row, "idProduto"));

    if (not queryProd.exec() or not queryProd.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + queryProd.lastError().text());
      return false;
    }

    QSqlQuery queryEstoque;
    queryEstoque.prepare("SELECT ncm FROM estoque WHERE idVendaProduto = :idVendaProduto");
    queryEstoque.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not queryEstoque.exec() or not queryEstoque.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando estoque: " + queryEstoque.lastError().text());
      return false;
    }

    const QString numProd = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding with zeros
    stream << "[Produto" + numProd + "]" << endl;
    //    stream << "CFOP = " + queryEstoque.value("cfop").toString() << endl;
    stream << "CFOP = 5101" << endl;
    stream << "NCM = " + queryEstoque.value("ncm").toString() << endl;

    if (queryProd.value("codComercial").toString().isEmpty()) {
      //      stream << "Codigo = 0000000000000" << endl;
      QMessageBox::critical(this, "Erro!", "Código vazio!");
      return false;
    }

    stream << "Codigo = " + queryProd.value("codComercial").toString() << endl;

    if (queryProd.value("codBarras").toString().isEmpty()) QMessageBox::critical(this, "Erro!", "CodBarras vazio!");

    stream << "EAN = " + queryProd.value("codBarras").toString() << endl;

    if (queryProd.value("descricao").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Descrição vazio!");
      return false;
    }

    stream << "Descricao = " + removeDiacritics(queryProd.value("descricao").toString()) << endl;

    if (queryProd.value("un").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Unidade vazio!");
      return false;
    }

    stream << "Unidade = " + removeDiacritics(queryProd.value("un").toString()) << endl;

    if (modelProd.data(row, "quant").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Quantidade vazio!");
      return false;
    }

    stream << "Quantidade = " + modelProd.data(row, "quant").toString() << endl;

    if (queryProd.value("precoVenda").toDouble() == 0) {
      QMessageBox::critical(this, "Erro!", "Preço venda = 0!");
      return false;
    }

    const double preco = queryProd.value("precoVenda").toDouble();
    const double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.;
    stream << "ValorUnitario = " + QString::number(rounded_number) << endl;

    if (modelProd.data(row, "parcial").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Parcial vazio!");
      return false;
    }

    stream << "ValorTotal = " + modelProd.data(row, "parcial").toString() << endl;

    //    const double frete = modelProd.data(row, "valorTotal").toDouble() /
    //                         (ui->doubleSpinBoxFinal->value() - ui->doubleSpinBoxFrete->value()) *
    //                         ui->doubleSpinBoxFrete->value();
    //    stream << "vFrete = " + QString::number(frete) << endl;

    total += modelProd.data(row, "parcial").toDouble();

    stream << "[ICMS" + numProd + "]" << endl;
    //    stream << "CST = " + modelProd.data(row, "cst").toString().remove(0, 1) << endl;
    stream << "ValorBase = " + modelProd.data(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl; // TODO: get aliquota from model

    // TODO: replace hardcoded icms with value from model
    const double icms = modelProd.data(row, "parcial").toDouble() * 0.18;
    icmsTotal += icms;

    stream << "Valor = " + QString::number(icms) << endl;
  }

  return true;
}

void CadastrarNFe::writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete) const {
  stream << "[Total]" << endl;
  stream << "BaseICMS = " + QString::number(total) << endl;
  stream << "ValorICMS = " + QString::number(icmsTotal) << endl;
  stream << "ValorProduto = " + QString::number(total) << endl;
  //  stream << "ValorFrete = " + QString::number(ui->doubleSpinBoxFrete->value()) << endl;
  stream << "ValorNota = " + QString::number(total + frete) << endl;
}

bool CadastrarNFe::writeTXT() {
  chaveNum = criarChaveAcesso();
  chaveAcesso = "NFE" + chaveNum;

  const QString dir = UserSession::settings("User/pastaEntACBr").toString();
  QFile file(dir + "/" + chaveNum + ".txt");

  qDebug() << "file: " << dir + chaveNum + ".txt";

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  const QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  QTextStream stream(&file);

  stream << "NFE.CriarEnviarNFe(\"" << endl;
  //  stream << "NFE.CriarNFe(\"" << endl;

  writeIdentificacao(stream);

  if (not writeEmitente(stream)) return false;

  if (not writeDestinatario(stream)) return false;

  double total = 0.;
  double icmsTotal = 0.;
  //  double frete = ui->doubleSpinBoxFrete->value();
  double frete = 0.;

  if (not writeProduto(stream, total, icmsTotal)) return false;

  writeTotal(stream, total, icmsTotal, frete);

  //  stream << "\")";
  //  stream << "\",1,1)";
  stream << "\",0)";

  stream.flush();
  file.close();

  return true;
}

void CadastrarNFe::on_pushButtonGerarNFE_clicked() { // NOTE: for testing, remove later
  chaveNum = criarChaveAcesso();
  chaveAcesso = "NFE" + chaveNum;

  const QString dir = UserSession::settings("User/pastaEntACBr").toString();
  QFile file(dir + "/" + chaveNum + ".txt");

  qDebug() << "file: " << dir + chaveNum + ".txt";

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return;
  }

  const QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  QTextStream stream(&file);

  //  stream << "NFE.CriarEnviarNFe(\"" << endl;
  stream << "NFE.CriarNFe(\"" << endl;

  writeIdentificacao(stream);

  if (not writeEmitente(stream)) return;

  if (not writeDestinatario(stream)) return;

  double total = 0.;
  double icmsTotal = 0.;
  //  double frete = ui->doubleSpinBoxFrete->value();
  double frete = 0.;

  if (not writeProduto(stream, total, icmsTotal)) return;

  writeTotal(stream, total, icmsTotal, frete);

  stream << "\")";
  //  stream << "\",1,1)";
  //  stream << "\",0)";

  stream.flush();
  file.close();

  QMessageBox::information(this, "Aviso!", "NFe enviada para ACBr!");
  guardarNotaBD();
  close();
}

// TODO: replace querys with models
// TODO: if acbr folders not set ask user
// TODO: na emissão da nfe perguntar/mostrar o número da sequência
// TODO: replace QString::number with QLocale.toString where appropriate
