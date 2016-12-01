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

CadastrarNFe::CadastrarNFe(const QString &idVenda, QWidget *parent)
    : QDialog(parent), idVenda(idVenda), ui(new Ui::CadastrarNFe) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  mapper.setModel(&modelProd);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  mapper.addMapping(ui->doubleSpinBoxICMSvbc, modelProd.fieldIndex("vBC"));
  mapper.addMapping(ui->doubleSpinBoxICMSpicms, modelProd.fieldIndex("pICMS"));
  mapper.addMapping(ui->doubleSpinBoxICMSvicms, modelProd.fieldIndex("vICMS"));
  mapper.addMapping(ui->doubleSpinBoxICMSpmvast, modelProd.fieldIndex("pMVAST"));
  mapper.addMapping(ui->doubleSpinBoxICMSvbcst, modelProd.fieldIndex("vBCST"));
  mapper.addMapping(ui->doubleSpinBoxICMSpicmsst, modelProd.fieldIndex("pICMSST"));
  mapper.addMapping(ui->doubleSpinBoxICMSvicmsst, modelProd.fieldIndex("vICMSST"));
  mapper.addMapping(ui->lineEditIPIcEnq, modelProd.fieldIndex("cEnq"));
  mapper.addMapping(ui->doubleSpinBoxPISvbc, modelProd.fieldIndex("vBCPIS"));
  mapper.addMapping(ui->doubleSpinBoxPISppis, modelProd.fieldIndex("pPIS"));
  mapper.addMapping(ui->doubleSpinBoxPISvpis, modelProd.fieldIndex("vPIS"));
  mapper.addMapping(ui->doubleSpinBoxCOFINSvbc, modelProd.fieldIndex("vBCCOFINS"));
  mapper.addMapping(ui->doubleSpinBoxCOFINSpcofins, modelProd.fieldIndex("pCOFINS"));
  mapper.addMapping(ui->doubleSpinBoxCOFINSvcofins, modelProd.fieldIndex("vCOFINS"));

  ui->lineEditModelo->setInputMask("99;_");
  ui->lineEditSerie->setInputMask("999;_");
  ui->lineEditCodigo->setInputMask("99999999;_");
  ui->lineEditNumero->setInputMask("999999999;_");
  ui->lineEditTipo->setInputMask("9;_");
  ui->lineEditFormatoPagina->setInputMask("9;_");

  ui->itemBoxEnderecoFaturamento->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoEntrega->setSearchDialog(SearchDialog::enderecoCliente(this));

  if (idVenda.isEmpty()) QMessageBox::critical(this, "Erro!", "idVenda vazio!");

  connect(&modelProd, &QAbstractItemModel::dataChanged, this, &CadastrarNFe::updateImpostos);
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
  modelProd.setHeaderData("codBarras", "Cód. Barras");
  modelProd.setHeaderData("ncm", "NCM");
  modelProd.setHeaderData("cfop", "CFOP");

  ui->tableItens->setModel(&modelProd);
  ui->tableItens->setItemDelegateForColumn("total", new ReaisDelegate(this));
  ui->tableItens->hideColumn("descUnitario");
  ui->tableItens->hideColumn("estoque_promocao");
  ui->tableItens->hideColumn("valorUnid");
  ui->tableItens->hideColumn("valor");
  ui->tableItens->hideColumn("codBarrasTrib");
  ui->tableItens->hideColumn("unTrib");
  ui->tableItens->hideColumn("quantTrib");
  ui->tableItens->hideColumn("valorTrib");
  ui->tableItens->hideColumn("compoeTotal");
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

bool CadastrarNFe::guardarNotaBD() {
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

  const QTime wait = QTime::currentTime().addSecs(10);

  while (QTime::currentTime() < wait) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if (fileResposta.exists()) break;
  }

  progressDialog->cancel();

  if (not fileResposta.exists()) {
    QMessageBox::critical(this, "Erro!", "ACBr não respondeu, verificar se ele está aberto e funcionando!");

    QFile entrada(UserSession::settings("User/pastaEntACBr").toString() + "/" + chaveNum + ".txt");

    if (entrada.exists()) entrada.remove();

    return false;
  }

  if (not fileResposta.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + fileResposta.errorString());
    return false;
  }

  QTextStream ts(&fileResposta);

  const QString resposta = ts.readAll();
  fileResposta.remove();

  // TODO: se a reposta tiver 'rejeicao' retornar falso para a tela nao fechar (guardar nota rejeitada sem associar e
  // refazer o MAX(numeroNFe))
  if (resposta.contains("xMotivo=Autorizado o uso da NF-e") or resposta.contains("OK")) {
    QMessageBox::information(this, "Aviso!", "Resposta do ACBr: " + resposta);
  } else {
    QMessageBox::critical(this, "Erro!", "Resposta do ACBr: " + resposta);
    if (resposta.contains("Requisição não enviada.")) return false;
  }

  QFile file(UserSession::settings("User/pastaXmlACBr").toString() + "/" + chaveNum + "-nfe.xml");

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo xml: " + file.errorString());
    return false;
  }

  QSqlQuery queryNota;
  queryNota.prepare("INSERT INTO nfe (tipo, xml, status, chaveAcesso) VALUES (:tipo, "
                    ":xml, :status, :chaveAcesso)");
  queryNota.bindValue(":tipo", "SAÍDA");
  queryNota.bindValue(":xml", file.readAll());
  queryNota.bindValue(":status", "PENDENTE");
  queryNota.bindValue(":chaveAcesso", chaveNum);

  if (not queryNota.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando nota: " + queryNota.lastError().text());
    return false;
  }

  QMessageBox::information(this, "Aviso!", "Nota guardada com sucesso!");

  if (not resposta.contains("xMotivo=Autorizado o uso da NF-e")) return true;

  const QVariant id = queryNota.lastInsertId();

  QSqlQuery query;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    query.prepare(
        "UPDATE pedido_fornecedor_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status do pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'EM ENTREGA', idNfeSaida = :idNfeSaida WHERE "
                  "idVendaProduto = :idVendaProduto");
    query.bindValue(":idNfeSaida", id);
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando NFe nos produtos: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE veiculo_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando carga veiculo: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void CadastrarNFe::on_pushButtonEnviarNFE_clicked() {
  // validacao

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();

    const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();

    QFile file(dirEntrada + "/" + chaveNum + ".txt");

    if (file.exists()) file.remove();

    return;
  }

  QSqlQuery("COMMIT").exec();

  close();
}

bool CadastrarNFe::cadastrar(const bool test) {
  if (not writeTXT(test)) return false;

  QMessageBox::information(this, "Aviso!", "NFe enviada para ACBr!");

  if (not guardarNotaBD()) return false;

  return true;
}

void CadastrarNFe::updateImpostos() {
  double baseICMS = 0;
  double valorICMS = 0;
  //  double valorIPI = 0;
  double valorPIS = 0;
  double valorCOFINS = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    baseICMS += modelProd.data(row, "vBC").toDouble();
    valorICMS += modelProd.data(row, "vICMS").toDouble();
    //    valorIPI += modelProd.data(row, "vIPI").toDouble();
    valorPIS += modelProd.data(row, "vPIS").toDouble();
    valorCOFINS += modelProd.data(row, "vCOFINS").toDouble();
  }

  const double total = valorICMS + /*valorIPI +*/ valorPIS + valorCOFINS;

  ui->doubleSpinBoxBaseICMS->setValue(baseICMS);
  ui->doubleSpinBoxValorICMS->setValue(valorICMS);
  //  ui->doubleSpinBoxValorIPI->setValue(valorIPI);
  ui->doubleSpinBoxValorPIS->setValue(valorPIS);
  ui->doubleSpinBoxValorCOFINS->setValue(valorCOFINS);

  const QString endereco = ui->lineEditDestinatarioLogradouro_2->text() + ", " +
                           ui->lineEditDestinatarioNumero_2->text() + " " +
                           ui->lineEditDestinatarioComplemento_2->text() + " - " +
                           ui->lineEditDestinatarioBairro_2->text() + " - " + ui->lineEditDestinatarioCidade_2->text() +
                           " - " + ui->lineEditDestinatarioUF_2->text() + " - " + ui->lineEditDestinatarioCEP_2->text();

  const QString texto = "Venda de código " + modelVenda.data(0, "idVenda").toString() + ";END. ENTREGA: " + endereco +
                        ";Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO "
                        "313Y;Total Aproximado de tributos federais, estaduais e municipais: R$ " +
                        QLocale(QLocale::Portuguese).toString(total);
  ui->textEdit->setPlainText(texto);
}

void CadastrarNFe::prepararNFe(const QList<int> &items) {
  QString filter;

  qDebug() << "items: " << items;

  // TODO: para cada item da lista pesquisar se aparece na view (consistencia venda_has_produto <> estoque_has_consumo)

  for (auto const &item : items) {
    filter += QString(filter.isEmpty() ? "" : " OR ") + "idVendaProduto = " + QString::number(item);
  }

  qDebug() << "filter: " << filter;

  modelProd.setFilter(filter);

  if (not modelProd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de itens da venda: " + modelProd.lastError().text());
    return;
  }

  ui->tableItens->resizeColumnsToContents();

  QSqlQuery queryNfe;

  if (not queryNfe.exec("SELECT COALESCE(MAX(idNFe) + 1, 1) AS idNFe FROM nfe") or not queryNfe.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando idNFe: " + queryNfe.lastError().text());
    return;
  }

  const int id = queryNfe.value("idNFe").toInt();

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

  // endereco faturamento

  ui->itemBoxEnderecoFaturamento->searchDialog()->setFilter(
      "idCliente = " + modelVenda.data(0, "idCliente").toString() + " AND desativado = FALSE OR idEndereco = 1");
  ui->itemBoxEnderecoFaturamento->setValue(modelVenda.data(0, "idEnderecoFaturamento"));

  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", modelVenda.data(0, "idEnderecoFaturamento"));

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

  // endereco entrega

  ui->itemBoxEnderecoEntrega->searchDialog()->setFilter("idCliente = " + modelVenda.data(0, "idCliente").toString() +
                                                        " AND desativado = FALSE OR idEndereco = 1");
  ui->itemBoxEnderecoEntrega->setValue(modelVenda.data(0, "idEnderecoEntrega"));

  queryDestinatarioEndereco.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", modelVenda.data(0, "idEnderecoEntrega"));

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
    return;
  }

  ui->lineEditDestinatarioLogradouro_2->setText(queryDestinatarioEndereco.value("logradouro").toString());
  ui->lineEditDestinatarioNumero_2->setText(queryDestinatarioEndereco.value("numero").toString());
  ui->lineEditDestinatarioComplemento_2->setText(queryDestinatarioEndereco.value("complemento").toString());
  ui->lineEditDestinatarioBairro_2->setText(queryDestinatarioEndereco.value("bairro").toString());
  ui->lineEditDestinatarioCidade_2->setText(queryDestinatarioEndereco.value("cidade").toString());
  ui->lineEditDestinatarioUF_2->setText(queryDestinatarioEndereco.value("uf").toString());
  ui->lineEditDestinatarioCEP_2->setText(queryDestinatarioEndereco.value("cep").toString());

  //-----------------------

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    modelProd.setData(row, "pPIS", 0.65);
    modelProd.setData(row, "vPIS",
                      modelProd.data(row, "vBCPIS").toDouble() * modelProd.data(row, "pPIS").toDouble() / 100);
    modelProd.setData(row, "pCOFINS", 3);
    modelProd.setData(row, "vCOFINS",
                      modelProd.data(row, "vBCCOFINS").toDouble() * modelProd.data(row, "pCOFINS").toDouble() / 100);
    modelProd.setData(row, "cfop", "5101");
  }

  //-----------------------

  double valorProdutos = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) valorProdutos += modelProd.data(row, "total").toDouble();

  const double frete =
      valorProdutos / modelVenda.data(0, "subTotalLiq").toDouble() * modelVenda.data(0, "frete").toDouble();

  ui->doubleSpinBoxValorProduto->setValue(valorProdutos);
  ui->doubleSpinBoxValorFrete->setValue(frete);
  ui->doubleSpinBoxValorNota->setValue(valorProdutos + frete);

  //

  QSqlQuery queryTransp;
  queryTransp.prepare("SELECT t.cnpj, t.razaoSocial, t.inscEstadual, the.logradouro, the.numero, the.complemento, "
                      "the.bairro, the.cidade, the.uf, thv.placa, thv.ufPlaca, t.antt FROM veiculo_has_produto vhp "
                      "LEFT JOIN transportadora_has_veiculo thv ON vhp.idVeiculo = thv.idVeiculo LEFT JOIN "
                      "transportadora t ON thv.idTransportadora = t.idTransportadora LEFT JOIN "
                      "transportadora_has_endereco the ON t.idTransportadora = the.idTransportadora WHERE "
                      "idVendaProduto = :idVendaProduto");
  queryTransp.bindValue(":idVendaProduto", items.first());

  if (not queryTransp.exec() or not queryTransp.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da transportadora: " + queryTransp.lastError().text());
    return;
  }

  const QString endereco = queryTransp.value("logradouro").toString() + " - " + queryTransp.value("numero").toString() +
                           " - " + queryTransp.value("complemento").toString() + " - " +
                           queryTransp.value("bairro").toString();

  ui->lineEditTransportadorCpfCnpj->setText(queryTransp.value("cnpj").toString());
  ui->lineEditTransportadorRazaoSocial->setText(queryTransp.value("razaoSocial").toString());
  ui->lineEditTransportadorInscEst->setText(queryTransp.value("inscEstadual").toString());
  ui->lineEditTransportadorEndereco->setText(endereco);
  ui->lineEditTransportadorUf->setText(queryTransp.value("uf").toString());
  ui->lineEditTransportadorMunicipio->setText(queryTransp.value("cidade").toString());

  ui->lineEditTransportadorPlaca->setText(queryTransp.value("placa").toString());
  ui->lineEditTransportadorRntc->setText(queryTransp.value("antt").toString());
  ui->lineEditTransportadorUfPlaca->setText(queryTransp.value("ufPlaca").toString());

  // somar pesos para os campos do transporte
  int caixas = 0;
  double peso = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    caixas += modelProd.data(row, "caixas").toInt();

    QSqlQuery queryProduto;
    queryProduto.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");
    queryProduto.bindValue(":idProduto", modelProd.data(row, "idProduto"));

    if (not queryProduto.exec() or not queryProduto.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando peso do produto: " + queryProduto.lastError().text());
      return;
    }

    peso += queryProduto.value("kgcx").toDouble() * modelProd.data(row, "caixas").toInt();
  }

  ui->spinBoxVolumesQuant->setValue(caixas);
  ui->lineEditVolumesEspecie->setText("Caixas");
  ui->doubleSpinBoxVolumesPesoBruto->setValue(peso);
  ui->doubleSpinBoxVolumesPesoLiq->setValue(peso);

  // limpar campos dos impostos
  for (int row = 0; row < modelProd.rowCount(); ++row) {
    for (int col = 44; col < modelProd.columnCount(); ++col) {
      modelProd.setData(row, col, 0);
    }

    modelProd.setData(row, "cstPIS", "01");
    modelProd.setData(row, "cstCOFINS", "01");
    // TODO: limpar CFOP para que o usuario preencha (e verificar que foi preenchido)
  }

  // CFOP

  if (ui->lineEditEmitenteUF->text() == ui->lineEditDestinatarioUF->text()) { // mesmo estado
    QSqlQuery queryCfop;

    if (not queryCfop.exec("SELECT CFOP_DE, NAT FROM cfop_sai WHERE CFOP_DE != ''")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando CFOP: " + queryCfop.lastError().text());
      return;
    }

    ui->comboBoxCfop->clear();

    while (queryCfop.next()) {
      ui->comboBoxCfop->addItem(queryCfop.value("CFOP_DE").toString() + " - " + queryCfop.value("NAT").toString());
    }
  } else {
    QSqlQuery queryCfop;

    if (not queryCfop.exec("SELECT CFOP_FE, NAT FROM cfop_sai WHERE CFOP_FE != ''")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando CFOP: " + queryCfop.lastError().text());
      return;
    }

    ui->comboBoxCfop->clear();

    while (queryCfop.next()) {
      ui->comboBoxCfop->addItem(queryCfop.value("CFOP_FE").toString() + " - " + queryCfop.value("NAT").toString());
    }
  }

  //

  updateImpostos();
}

bool CadastrarNFe::criarChaveAcesso(QString &chave) {
  QStringList listChave;

  listChave << modelLoja.data(0, "codUF").toString();
  listChave << QDate::currentDate().toString("yyMM");      // Ano/Mês
  listChave << clearStr(ui->lineEditEmitenteCNPJ->text()); // CNPJ
  listChave << ui->lineEditModelo->text();                 // modelo NFe
  listChave << ui->lineEditSerie->text();
  listChave << ui->lineEditNumero->text(); // número nf-e (id interno)
  listChave << ui->lineEditTipo->text();   // tpEmis - forma de emissão
  listChave << ui->lineEditCodigo->text(); // código númerico aleatorio

  chave = listChave.join("");

  if (not calculaDigitoVerificador(chave)) return false;

  return true;
}

QString CadastrarNFe::clearStr(const QString &str) const {
  return QString(str).remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

QString CadastrarNFe::removeDiacritics(const QString &str) const {
  return str == "M²" ? "M2" : QString(str).normalized(QString::NormalizationForm_KD).remove(QRegExp("[^a-zA-Z\\s]"));
}

bool CadastrarNFe::calculaDigitoVerificador(QString &chave) {
  if (chave.size() != 43) {
    QMessageBox::critical(this, "Erro!", "Erro no tamanho da chave: " + chave);
    return false;
  }

  QVector<int> chave2;

  for (int i = 0, size = chave.size(); i < size; ++i) chave2 << chave.at(i).digitValue();

  const QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                        6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  int soma = 0;

  for (int i = 0; i < 43; ++i) soma += chave2.at(i) * multiplicadores.at(i);

  const int resto = soma % 11;

  chave += QString::number(resto < 2 ? 0 : 11 - resto);

  return true;
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
  queryCliente.prepare(
      "SELECT nome_razao, pfpj, cpf, cnpj, inscEstadual, tel FROM cliente WHERE idCliente = :idCliente");
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
      stream << "indIEDest = 9" << endl;
    } else {
      stream << "IE = " + clearStr(queryCliente.value("inscEstadual").toString()) << endl;
    }
  }

  stream << "Fone = " + queryCliente.value("tel").toString() << endl;

  QSqlQuery queryEndereco;
  queryEndereco.prepare(
      "SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM cliente_has_endereco WHERE "
      "idEndereco = :idEndereco");
  queryEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoFaturamento->value());

  if (not queryEndereco.exec() or not queryEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço cliente: " + queryEndereco.lastError().text());
    return false;
  }

  if (queryEndereco.value("cep").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CEP cliente vazio!");
    return false;
  }

  stream << "CEP = " + clearStr(queryEndereco.value("cep").toString()) << endl;

  if (queryEndereco.value("logradouro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Logradouro cliente vazio!");
    return false;
  }

  stream << "Logradouro = " + removeDiacritics(queryEndereco.value("logradouro").toString()) << endl;

  if (queryEndereco.value("numero").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Número Endereço do Cliente vazio!");
    return false;
  }

  stream << "Numero = " + queryEndereco.value("numero").toString() << endl;

  stream << "Complemento = " + removeDiacritics(queryEndereco.value("complemento").toString()) << endl;

  if (queryEndereco.value("bairro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Bairro cliente vazio!");
    return false;
  }

  stream << "Bairro = " + removeDiacritics(queryEndereco.value("bairro").toString()) << endl;

  if (queryEndereco.value("cidade").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cidade cliente vazio!");
    return false;
  }

  stream << "Cidade = " + removeDiacritics(queryEndereco.value("cidade").toString()) << endl;

  if (queryEndereco.value("uf").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "UF cliente vazio!");
    return false;
  }

  stream << "UF = " + queryEndereco.value("uf").toString() << endl;

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

    if (modelProd.data(row, "ncm").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "NCM vazio!");
      return false;
    }

    stream << "NCM = " + modelProd.data(row, "ncm").toString() << endl;

    if (modelProd.data(row, "codComercial").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Código vazio!");
      return false;
    }

    stream << "Codigo = " + modelProd.data(row, "codComercial").toString() << endl;

    const QString codBarras = modelProd.data(row, "codBarras").toString();

    stream << "EAN = " + (codBarras.isEmpty() ? "0000000000000" : codBarras) << endl;

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

    if (modelProd.data(row, "prcUnitario").toDouble() == 0.) {
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
    // TODO: gravar apenas os valores necessarios seguindo as regras da situacao tributaria
    stream << "CST = " + modelProd.data(row, "cstICMS").toString().remove(0, 1) << endl;
    stream << "Modalidade = " + modelProd.data(row, "modBC").toString() << endl;
    stream << "ValorBase = " + modelProd.data(row, "vBC").toString() << endl;
    double aliquota = modelProd.data(row, "pICMS").toDouble();
    stream << "Aliquota = " + QString::number(aliquota, 'f', 2) << endl;
    stream << "Valor = " + modelProd.data(row, "vICMS").toString() << endl;

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

bool CadastrarNFe::writeTransportadora(QTextStream &stream) const {
  stream << "[Transportador]" << endl;
  stream << "FretePorConta = " << ui->comboBoxFreteConta->currentText().left(1) << endl;
  stream << "NomeRazao = " << ui->lineEditTransportadorRazaoSocial->text() << endl;

  if (ui->lineEditTransportadorRazaoSocial->text() != "RETIRA") {
    stream << "CnpjCpf = " << ui->lineEditTransportadorCpfCnpj->text() << endl;
    stream << "IE = " << ui->lineEditTransportadorInscEst->text() << endl;
    stream << "Endereco = " << ui->lineEditTransportadorEndereco->text() << endl;
    stream << "Cidade = " << ui->lineEditTransportadorMunicipio->text() << endl;
    stream << "UF = " << ui->lineEditTransportadorUf->text() << endl;
    // TODO: verificar em que etapa é digitado o valor do serviço (em montar carga?)
    //  stream << "ValorServico = " << endl;
    //  stream << "ValorBase = " << endl;
    //  stream << "Aliquota = " << endl;
    //  stream << "Valor = " << endl;
    //  stream << "CFOP = " << endl;
    //  stream << "CidadeCod = " << endl;
    stream << "Placa = " << ui->lineEditTransportadorPlaca->text().remove("-") << endl;
    stream << "UFPlaca = " << ui->lineEditTransportadorUfPlaca->text() << endl;
    //  stream << "RNTC = " << endl;
  }

  return true;
}

bool CadastrarNFe::writeVolume(QTextStream &stream) const {
  stream << "[Volume001]" << endl;

  stream << "Quantidade = " << ui->spinBoxVolumesQuant->text() << endl;
  stream << "Especie = " << ui->lineEditVolumesEspecie->text() << endl;
  stream << "Marca = " << ui->lineEditVolumesMarca->text() << endl;
  stream << "Numeracao = " << ui->lineEditVolumesNumeracao->text() << endl;
  stream << "PesoLiquido = " << ui->doubleSpinBoxVolumesPesoLiq->text() << endl;
  stream << "PesoBruto = " << ui->doubleSpinBoxVolumesPesoBruto->text() << endl;

  return true;
}

bool CadastrarNFe::writeTXT(const bool test) {
  if (not criarChaveAcesso(chaveNum)) return false;

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
  if (not writeTransportadora(stream)) return false;
  if (not writeVolume(stream)) return false;

  stream << "[DadosAdicionais]" << endl;
  stream << "infCpl = " + ui->textEdit->toPlainText() << endl;

  if (test) {
    stream << "\",0)"; // dont return xml to erp
  } else {
    stream << "\",0,1)"; // print danfe
  }

  stream.flush();
  file.close();

  QFile file2(dirEntrada + "/" + chaveNum + ".txt");

  if (not file2.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  return true;
}

void CadastrarNFe::on_pushButtonGerarNFE_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar(true)) {
    QSqlQuery("ROLLBACK").exec();

    const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();

    QFile file(dirEntrada + "/" + chaveNum + ".txt");

    if (file.exists()) file.remove();

    return;
  }

  QSqlQuery("COMMIT").exec();

  close();
}

void CadastrarNFe::on_tableItens_entered(const QModelIndex &) { ui->tableItens->resizeColumnsToContents(); }

void CadastrarNFe::on_tableItens_clicked(const QModelIndex &index) {
  ui->groupBox_7->setEnabled(true);

  ui->comboBoxCfop->setCurrentIndex(
      ui->comboBoxCfop->findText(modelProd.data(index.row(), "cfop").toString(), Qt::MatchStartsWith));
  ui->comboBoxICMSOrig->setCurrentIndex(
      ui->comboBoxICMSOrig->findText(modelProd.data(index.row(), "orig").toString(), Qt::MatchStartsWith));
  ui->comboBoxSituacaoTributaria->setCurrentIndex(
      ui->comboBoxSituacaoTributaria->findText(modelProd.data(index.row(), "cstICMS").toString(), Qt::MatchStartsWith));
  ui->comboBoxICMSModBc->setCurrentIndex(modelProd.data(index.row(), "modBC").toInt());
  ui->comboBoxICMSModBcSt->setCurrentIndex(modelProd.data(index.row(), "modBCST").toInt());
  ui->comboBoxIPIcst->setCurrentIndex(
      ui->comboBoxIPIcst->findText(modelProd.data(index.row(), "cstIPI").toString(), Qt::MatchStartsWith));
  ui->comboBoxPIScst->setCurrentIndex(
      ui->comboBoxPIScst->findText(modelProd.data(index.row(), "cstPIS").toString(), Qt::MatchStartsWith));
  ui->comboBoxCOFINScst->setCurrentIndex(
      ui->comboBoxCOFINScst->findText(modelProd.data(index.row(), "cstCOFINS").toString(), Qt::MatchStartsWith));

  QSqlQuery query;
  query.prepare(
      "SELECT tipoICMS, cfop, orig, cstICMS, modBC, vBC, pICMS, vICMS, modBCST, pMVAST, vBCST, pICMSST, vICMSST, "
      "cEnq, cstIPI, cstPIS, vBCPIS, pPIS, vPIS, cstCOFINS, vBCCOFINS, pCOFINS, vCOFINS FROM "
      "view_produto_estoque WHERE "
      "idVendaProduto = :idVendaProduto");
  query.bindValue(":idVendaProduto", modelProd.data(index.row(), "idVendaProduto"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando impostos do produto: " + query.lastError().text());
    return;
  }

  ui->comboBoxRegime_2->setCurrentIndex(query.value("tipoICMS").toString().length() == 6 ? 1 : 2); // ICMSXX : ICMSSQN

  QSqlQuery queryCfop;
  queryCfop.prepare("SELECT NAT FROM cfop_sai WHERE cfop_de = :cfop OR cfop_fe = :cfop");
  queryCfop.bindValue(":cfop", query.value("cfop"));

  if (not queryCfop.exec() or not queryCfop.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando CFOP: " + queryCfop.lastError().text());
    return;
  }

  // ICMS
  ui->comboBoxCfop_2->setCurrentText(query.value("cfop").toString() + " - " + queryCfop.value("NAT").toString());
  ui->comboBoxICMSOrig_2->setCurrentIndex(
      ui->comboBoxICMSOrig_2->findText(query.value("orig").toString(), Qt::MatchStartsWith));
  ui->comboBoxSituacaoTributaria_2->setCurrentIndex(
      ui->comboBoxSituacaoTributaria_2->findText(query.value("cstICMS").toString(), Qt::MatchStartsWith));
  ui->comboBoxICMSModBc_2->setCurrentIndex(query.value("modBC").toInt() + 1);
  ui->doubleSpinBoxICMSvbc_3->setValue(query.value("vBC").toDouble());
  ui->doubleSpinBoxICMSpicms_3->setValue(query.value("pICMS").toDouble());
  ui->doubleSpinBoxICMSvicms_3->setValue(query.value("vICMS").toDouble());
  ui->comboBoxICMSModBcSt_2->setCurrentIndex(query.value("modBCST").toInt() + 1);
  ui->doubleSpinBoxICMSpmvast_2->setValue(query.value("pMVAST").toDouble());
  ui->doubleSpinBoxICMSvbcst_2->setValue(query.value("vBCST").toDouble());
  ui->doubleSpinBoxICMSpicmsst_2->setValue(query.value("pICMSST").toDouble());
  ui->doubleSpinBoxICMSvicmsst_2->setValue(query.value("pICMSST").toDouble());

  // IPI
  ui->comboBoxIPIcst_2->setCurrentIndex(
      ui->comboBoxIPIcst_2->findText(query.value("cstIPI").toString(), Qt::MatchStartsWith));
  ui->lineEditIPIcEnq_3->setText(query.value("cEnq").toString());

  // PIS
  ui->comboBoxPIScst_2->setCurrentIndex(
      ui->comboBoxPIScst_2->findText(query.value("cstPIS").toString(), Qt::MatchStartsWith));
  ui->doubleSpinBoxPISvbc_3->setValue(query.value("vBCPIS").toDouble());
  ui->doubleSpinBoxPISppis_3->setValue(query.value("pPIS").toDouble());
  ui->doubleSpinBoxPISvpis_3->setValue(query.value("vPIS").toDouble());

  // COFINS
  ui->comboBoxCOFINScst_3->setCurrentIndex(
      ui->comboBoxCOFINScst_3->findText(query.value("cstCOFINS").toString(), Qt::MatchStartsWith));
  ui->doubleSpinBoxCOFINSvbc_4->setValue(query.value("vBCCOFINS").toDouble());
  ui->doubleSpinBoxCOFINSpcofins_4->setValue(query.value("pCOFINS").toDouble());
  ui->doubleSpinBoxCOFINSvcofins_4->setValue(query.value("vCOFINS").toDouble());

  //

  mapper.setCurrentModelIndex(index);
}

void CadastrarNFe::on_tabWidget_currentChanged(int index) {
  if (index == 4) updateImpostos();
}

void CadastrarNFe::on_doubleSpinBoxICMSvbc_valueChanged(double) {
  ui->doubleSpinBoxICMSvicms->setValue(ui->doubleSpinBoxICMSvbc->value() * ui->doubleSpinBoxICMSpicms->value() / 100);
}

void CadastrarNFe::on_doubleSpinBoxICMSvbcst_valueChanged(double) {
  ui->doubleSpinBoxICMSvicmsst->setValue(ui->doubleSpinBoxICMSvbcst->value() * ui->doubleSpinBoxICMSpicmsst->value() /
                                         100);
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

void CadastrarNFe::on_itemBoxEnderecoFaturamento_textChanged(const QString &) {
  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoFaturamento->value());

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
}

void CadastrarNFe::on_itemBoxEnderecoEntrega_textChanged(const QString &) {
  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoEntrega->value());

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
    return;
  }

  ui->lineEditDestinatarioLogradouro_2->setText(queryDestinatarioEndereco.value("logradouro").toString());
  ui->lineEditDestinatarioNumero_2->setText(queryDestinatarioEndereco.value("numero").toString());
  ui->lineEditDestinatarioComplemento_2->setText(queryDestinatarioEndereco.value("complemento").toString());
  ui->lineEditDestinatarioBairro_2->setText(queryDestinatarioEndereco.value("bairro").toString());
  ui->lineEditDestinatarioCidade_2->setText(queryDestinatarioEndereco.value("cidade").toString());
  ui->lineEditDestinatarioUF_2->setText(queryDestinatarioEndereco.value("uf").toString());
  ui->lineEditDestinatarioCEP_2->setText(queryDestinatarioEndereco.value("cep").toString());

  updateImpostos();
}

void CadastrarNFe::on_comboBoxRegime_currentTextChanged(const QString &text) {
  if (text == "Tributação Normal") {
    QStringList list = {"00 - Tributada integralmente",
                        "10 - Tributada e com cobrança do ICMS por substituição tributária",
                        "20 - Com redução de base de cálculo",
                        "30 - Isenta ou não tributada e com cobrança do ICMS por substituição tributária",
                        "40 - Isenta",
                        "41 - Não tributada",
                        "50 - Suspensão",
                        "51 - Diferimento",
                        "60 - ICMS cobrado anteriormente por substituição tributária",
                        "70 - Com redução de base de cálculo e cobrança do ICMS por substituição tributária",
                        "90 - Outras"};

    ui->comboBoxSituacaoTributaria->clear();
    ui->comboBoxSituacaoTributaria->addItems(list);
  }

  if (text == "Simples Nacional") {
    QStringList list = {"101 - Tributada pelo Simples Nacional com permissão de crédito",
                        "102 - Tributada pelo Simples Nacional sem permissão de crédito",
                        "103 - Isenção do ICMS no Simples Nacional para faixa de receita bruta",
                        "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por "
                        "substituição tributária",
                        "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por "
                        "substituição tributária",
                        "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS "
                        "por substituição tributária",
                        "300 - Imune", "400 - Não tributada pelo Simples Nacional",
                        "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação",
                        "900 - Outros"};

    ui->comboBoxSituacaoTributaria->clear();
    ui->comboBoxSituacaoTributaria->addItems(list);
  }
}

void CadastrarNFe::on_comboBoxRegime_2_currentTextChanged(const QString &text) {
  if (text == "Tributação Normal") {
    QStringList list = {"00 - Tributada integralmente",
                        "10 - Tributada e com cobrança do ICMS por substituição tributária",
                        "20 - Com redução de base de cálculo",
                        "30 - Isenta ou não tributada e com cobrança do ICMS por substituição tributária",
                        "40 - Isenta",
                        "41 - Não tributada",
                        "50 - Suspensão",
                        "51 - Diferimento",
                        "60 - ICMS cobrado anteriormente por substituição tributária",
                        "70 - Com redução de base de cálculo e cobrança do ICMS por substituição tributária",
                        "90 - Outras"};

    ui->comboBoxSituacaoTributaria_2->clear();
    ui->comboBoxSituacaoTributaria_2->addItems(list);
  }

  if (text == "Simples Nacional") {
    QStringList list = {"101 - Tributada pelo Simples Nacional com permissão de crédito",
                        "102 - Tributada pelo Simples Nacional sem permissão de crédito",
                        "103 - Isenção do ICMS no Simples Nacional para faixa de receita bruta",
                        "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por "
                        "substituição tributária",
                        "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por "
                        "substituição tributária",
                        "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS "
                        "por substituição tributária",
                        "300 - Imune", "400 - Não tributada pelo Simples Nacional",
                        "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação",
                        "900 - Outros"};

    ui->comboBoxSituacaoTributaria_2->clear();
    ui->comboBoxSituacaoTributaria_2->addItems(list);
  }
}

void CadastrarNFe::on_comboBoxSituacaoTributaria_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "cstICMS", text.left(2));

  //

  if (text == "00 - Tributada integralmente") {
    ui->frame->show();
    ui->frame_2->hide();
  }

  if (text == "10 - Tributada e com cobrança do ICMS por substituição tributária") {
  }
  if (text == "20 - Com redução de base de cálculo") {
  }
  if (text == "30 - Isenta ou não tributada e com cobrança do ICMS por substituição tributária") {
  }
  if (text == "40 - Isenta") {
  }
  if (text == "41 - Não tributada") {
  }
  if (text == "50 - Suspensão") {
  }
  if (text == "51 - Diferimento") {
  }
  if (text == "60 - ICMS cobrado anteriormente por substituição tributária") {
    ui->frame->hide();
    ui->frame_2->show();
  }
  if (text == "70 - Com redução de base de cálculo e cobrança do ICMS por substituição tributária") {
  }
  if (text == "90 - Outras") {
  }

  // simples nacional

  if (text == "101 - Tributada pelo Simples Nacional com permissão de crédito") {
  }
  if (text == "102 - Tributada pelo Simples Nacional sem permissão de crédito") {
  }
  if (text == "103 - Isenção do ICMS no Simples Nacional para faixa de receita bruta") {
  }
  if (text == "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por ") {
  }
  if (text == "substituição tributária") {
  }
  if (text == "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por ") {
  }
  if (text == "substituição tributária") {
  }
  if (text == "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS ") {
  }
  if (text == "por substituição tributária") {
  }
  if (text == "300 - Imune", "400 - Não tributada pelo Simples Nacional") {
  }
  if (text == "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação") {
  }
  if (text == "900 - Outros") {
  }
}

void CadastrarNFe::on_comboBoxSituacaoTributaria_2_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "cstICMS", text.left(2));

  //

  if (text == "00 - Tributada integralmente") {
    ui->frame_3->show();
    ui->frame_4->hide();
  }

  if (text == "10 - Tributada e com cobrança do ICMS por substituição tributária") {
  }
  if (text == "20 - Com redução de base de cálculo") {
  }
  if (text == "30 - Isenta ou não tributada e com cobrança do ICMS por substituição tributária") {
  }
  if (text == "40 - Isenta") {
  }
  if (text == "41 - Não tributada") {
  }
  if (text == "50 - Suspensão") {
  }
  if (text == "51 - Diferimento") {
  }
  if (text == "60 - ICMS cobrado anteriormente por substituição tributária") {
    ui->frame_3->hide();
    ui->frame_4->show();
  }
  if (text == "70 - Com redução de base de cálculo e cobrança do ICMS por substituição tributária") {
  }
  if (text == "90 - Outras") {
  }

  // simples nacional

  if (text == "101 - Tributada pelo Simples Nacional com permissão de crédito") {
  }
  if (text == "102 - Tributada pelo Simples Nacional sem permissão de crédito") {
  }
  if (text == "103 - Isenção do ICMS no Simples Nacional para faixa de receita bruta") {
  }
  if (text == "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por ") {
  }
  if (text == "substituição tributária") {
  }
  if (text == "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por ") {
  }
  if (text == "substituição tributária") {
  }
  if (text == "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS ") {
  }
  if (text == "por substituição tributária") {
  }
  if (text == "300 - Imune", "400 - Não tributada pelo Simples Nacional") {
  }
  if (text == "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação") {
  }
  if (text == "900 - Outros") {
  }
}

void CadastrarNFe::on_comboBoxICMSOrig_currentIndexChanged(int index) {
  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "orig", index - 1);
}

void CadastrarNFe::on_comboBoxICMSModBc_currentIndexChanged(int index) {
  // modBC

  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "modBC", index - 1);
}

void CadastrarNFe::on_comboBoxICMSModBcSt_currentIndexChanged(int index) {
  // modBCST

  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "modBCST", index - 1);
}

void CadastrarNFe::on_doubleSpinBoxICMSvicms_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "vICMS", value);
}

void CadastrarNFe::on_doubleSpinBoxICMSvicmsst_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "vICMSST", value);
}

void CadastrarNFe::on_comboBoxIPIcst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "cstIPI", text.left(2));
}

void CadastrarNFe::on_comboBoxPIScst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "cstPIS", text.left(2));
}

void CadastrarNFe::on_doubleSpinBoxPISvpis_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "vPIS", value);
}

void CadastrarNFe::on_comboBoxCOFINScst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "cstCOFINS", text.left(2));
}

void CadastrarNFe::on_doubleSpinBoxCOFINSvcofins_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.size() == 0) return;

  modelProd.setData(list.first().row(), "vCOFINS", value);
}

void CadastrarNFe::on_doubleSpinBoxICMSpicmsst_valueChanged(double) {
  ui->doubleSpinBoxICMSvicmsst->setValue(ui->doubleSpinBoxICMSvbcst->value() * ui->doubleSpinBoxICMSpicmsst->value() /
                                         100);
}

// TODO: fracionar compra (caso de fracionar o frete?)
// TODO: na tela de transportador colocar lupa para pesquisar transportador
// TODO: concatenar caixas na descricao do produto
// TODO: checkbox com opcao de mandar email com xml e danfe para o cliente
// TODO: fazer logica funcionar para marcar impostos em varias linhas de uma vez (verificar com eduardo)
// TODO: na tela de destinatario colocar botao de pesquisa quando precisa trocar o cliente (ex: emitir nota para a
// empresa do cliente)
// TODO: set validators
