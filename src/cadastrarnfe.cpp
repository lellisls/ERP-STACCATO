#include <QDate>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>

#include "acbr.h"
#include "cadastrarnfe.h"
#include "reaisdelegate.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

CadastrarNFe::CadastrarNFe(const QString &idVenda, QWidget *parent) : QDialog(parent), idVenda(idVenda), ui(new Ui::CadastrarNFe) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

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

  connect(ui->itemBoxLoja, &ItemBox::textChanged, this, &CadastrarNFe::alterarCertificado);
  ui->itemBoxLoja->setSearchDialog(SearchDialog::loja(this));
  ui->itemBoxLoja->setValue(UserSession::settings("User/lojaACBr"));

  ui->lineEditModelo->setInputMask("99;_");
  ui->lineEditSerie->setInputMask("999;_");
  ui->lineEditCodigo->setInputMask("99999999;_");
  ui->lineEditNumero->setInputMask("999999999;_");
  ui->lineEditTipo->setInputMask("9;_");
  ui->lineEditFormatoPagina->setInputMask("9;_");

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxEnderecoFaturamento->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoEntrega->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxVeiculo->setSearchDialog(SearchDialog::veiculo(this));

  if (idVenda.isEmpty()) QMessageBox::critical(this, "Erro!", "idVenda vazio!");

  connect(&modelProd, &QAbstractItemModel::dataChanged, this, &CadastrarNFe::updateImpostos);

  ui->comboBoxRegime->setCurrentIndex(1);

  ui->comboBoxFreteConta->setCurrentIndex(1);
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
  modelProd.setHeaderData("descUnitario", "R$ Unit.");
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
  ui->tableItens->setItemDelegateForColumn("descUnitario", new ReaisDelegate(this));
  ui->tableItens->hideColumn("idProduto");
  ui->tableItens->hideColumn("idVendaProduto");

  modelLoja.setTable("loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelLoja.setFilter("idLoja = " + UserSession::settings("User/lojaACBr").toString());

  if (not modelLoja.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de lojas: " + modelLoja.lastError().text());
  }
}

void CadastrarNFe::on_pushButtonEnviarNFE_clicked() {
  // TODO: ao clicar em enviar abrir um dialog mostrando as informacoes base para o usuario confirmar

  // TODO: refatorar funcao

  // se acbr enviar nota para receita mas nao receber a autor., consultar nota para atualizar com a autor.
  // 1. enviar comando para acbr criar/assinar/enviar xml
  // 2. se acbr retornar ok/autorizado guardar nota normalmente
  // 3. se acbr perder a conexao/nao responder guardar o xml nao autorizado (ele ficara na pasta do acbr)
  // TODO: 4. criar um botao para consultar notas pendentes (o acbr atualiza o xml com a autorizacao)

  //  modelLoja.setData(0, "cnpj", "99999090910270"); // TODO: test only, remove later

  if (not validar()) return;

  if (not criarChaveAcesso()) return;

  QString nfe;
  QTextStream stream(&nfe);

  // primeiro usar apenas o comando de criar nota
  // pegar o path do arquivo e mandar o comando de enviar nota
  // se ok continuar
  // se nao tiver resposta tentar consultar nota

  stream << R"(NFE.CriarNFe(")" << endl;

  writeIdentificacao(stream);
  writeEmitente(stream);
  writeDestinatario(stream);
  writeProduto(stream);
  writeTotal(stream);
  writeTransportadora(stream);
  writeVolume(stream);

  const QString infUsuario = ui->infCompUsuario->toPlainText().isEmpty() ? "" : ui->infCompUsuario->toPlainText();
  const QString infComp = (infUsuario.isEmpty() ? "" : infUsuario + ";") + ui->infCompSistema->toPlainText();

  stream << "[DadosAdicionais]" << endl;
  stream << "infCpl = " + infComp << endl;
  stream << R"(",0))" << endl; // dont return xml
  stream << "\r\n.\r\n";
  stream.flush();

  QString resposta;

  ACBr::enviarComando(nfe, resposta);

  //  QMessageBox::information(this, "Criar nota", resposta);

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Resposta CriarNFe", resposta);
    return;
  }

  const QString fileName = QString(resposta).remove("OK: ").remove("\r").remove("\n");

  {
    QFile file(fileName);

    if (not file.open(QFile::ReadOnly)) {
      QMessageBox::critical(this, "Erro!", "Erro lendo XML: " + file.errorString());
      return;
    }

    xml = file.readAll();
  }

  QSqlQuery queryNota;
  queryNota.prepare("INSERT INTO nfe (numeroNFe, tipo, xml, status, chaveAcesso, valor) VALUES (:numeroNFe, :tipo, "
                    ":xml, :status, :chaveAcesso, :valor)");
  queryNota.bindValue(":numeroNFe", ui->lineEditNumero->text());
  queryNota.bindValue(":tipo", "SAÍDA");
  queryNota.bindValue(":xml", xml);
  queryNota.bindValue(":status", "PENDENTE");
  queryNota.bindValue(":chaveAcesso", chaveNum);
  queryNota.bindValue(":valor", ui->doubleSpinBoxValorNota->value());

  if (not queryNota.exec()) {
    error = "Erro guardando nota: " + queryNota.lastError().text();
    return;
  }

  const QVariant id = queryNota.lastInsertId();

  if (queryNota.lastInsertId().isNull()) {
    QMessageBox::critical(this, "Erro!", "Erro lastInsertId");
    return;
  }

  // enviar nota

  ACBr::enviarComando("NFE.EnviarNFe(" + fileName + ", 1, 1, 0, 1)", resposta);

  //  QMessageBox::information(this, "EnviarNFe", resposta);

  if (not resposta.contains("XMotivo=Autorizado o uso da NF-e")) {
    QMessageBox::critical(this, "Resposta EnviarNFe", resposta);

    // se envio da nota falhar tentar consultar

    // se receita retornar 'nota nao consta no banco de dados' reenviar
    // XMotivo=Rejeição: NF-e não consta na base de dados da SEFAZ

    ACBr::enviarComando("NFE.ConsultarNFe(" + fileName + ")", resposta);

    QMessageBox::information(this, "ConsultarNFe", resposta);

    if (not resposta.contains("XMotivo=Autorizado o uso da NF-e")) {
      QMessageBox::critical(this, "Resposta ConsultarNFe", resposta);
      return;
    }
  }

  // se consulta falhar pedir para o usuario consultar manualmente depois
  // TODO: e mudar status para 'NOTA PENDENTE' para habilitar o botao de consulta

  {
    QFile file(fileName);

    if (not file.open(QFile::ReadOnly)) {
      QMessageBox::critical(this, "Erro!", "Erro lendo XML: " + file.errorString());
      return;
    }

    xml = file.readAll();
  }

  // -------------------------------------------------

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar(id)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    error.clear();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", resposta);

  {
    // TODO:__project public code
    const QString email = "fiscal5@ellycontabil.com.br";
    const QString copia = "logistica@staccatorevestimentos.com.br";
    const QString assunto = "NFe - " + ui->lineEditNumero->text() + " - STACCATO REVESTIMENTOS COMERCIO E REPRESENTACAO LTDA";

    const QString comando = "NFE.EnviarEmail(" + email + "," + fileName + ",1,'" + assunto + "', " + copia + ")";

    QString resposta;

    ACBr::enviarComando(comando, resposta);

    if (not resposta.contains("OK: Email enviado com sucesso")) {
      QMessageBox::critical(this, "Resposta EnviarEmail", resposta);

      // perguntar se deseja tentar enviar novamente?
    }

    QMessageBox::information(this, "Resposta", resposta);
  }

  ACBr::gerarDanfe(xml.toLatin1(), resposta);

  close();
}

bool CadastrarNFe::cadastrar(const QVariant &id) {
  QSqlQuery query;
  query.prepare("UPDATE nfe SET status = 'AUTORIZADO', xml = :xml WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":xml", xml);
  query.bindValue(":chaveAcesso", chaveNum);

  if (not query.exec()) {
    error = "Erro guardando nota: " + query.lastError().text();
    return false;
  }

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando status do pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'EM ENTREGA', idNfeSaida = :idNfeSaida WHERE "
                  "idVendaProduto = :idVendaProduto");
    query.bindValue(":idNfeSaida", id);
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro salvando NFe nos produtos: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE veiculo_has_produto SET status = 'EM ENTREGA', idNfeSaida = :idNfeSaida WHERE idVendaProduto "
                  "= :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProd.data(row, "idVendaProduto"));
    query.bindValue(":idNfeSaida", id);

    if (not query.exec()) {
      error = "Erro atualizando carga veiculo: " + query.lastError().text();
      return false;
    }
  }

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

  const QString endereco = ui->itemBoxEnderecoEntrega->getValue() == 1
                               ? "Não há/Retira"
                               : ui->lineEditDestinatarioLogradouro_2->text() + ", " + ui->lineEditDestinatarioNumero_2->text() + " " + ui->lineEditDestinatarioComplemento_2->text() + " - " +
                                     ui->lineEditDestinatarioBairro_2->text() + " - " + ui->lineEditDestinatarioCidade_2->text() + " - " + ui->lineEditDestinatarioUF_2->text() + " - " +
                                     ui->lineEditDestinatarioCEP_2->text();

  const QString texto = "Venda de código " + modelVenda.data(0, "idVenda").toString() + ";END. ENTREGA: " + endereco +
                        ";Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO "
                        "313Y;Total Aproximado de tributos federais, estaduais e municipais: R$ " +
                        QLocale(QLocale::Portuguese).toString(total);
  ui->infCompSistema->setPlainText(texto);
}

bool CadastrarNFe::preencherNumeroNFe() {
  if (ui->itemBoxLoja->text().isEmpty()) return true;

  QSqlQuery queryCnpj;
  queryCnpj.prepare("SELECT cnpj FROM loja WHERE idLoja = :idLoja");
  queryCnpj.bindValue(":idLoja", ui->itemBoxLoja->getValue());

  if (not queryCnpj.exec() or not queryCnpj.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando CNPJ: " + queryCnpj.lastError().text());
    return false;
  }

  const QString cnpj = queryCnpj.value("cnpj").toString().remove(".").remove("/").remove("-");

  QSqlQuery queryNfe;

  if (not queryNfe.exec("SELECT COALESCE(MAX(numeroNFe), 0) + 1 AS numeroNFe FROM nfe WHERE tipo = 'SAÍDA' AND status = 'AUTORIZADO' AND mid(chaveAcesso, 7, 14) = '" + cnpj +
                        "' AND created BETWEEN DATE_ADD(CURDATE(), INTERVAL - 30 DAY) AND CURDATE()") or
      not queryNfe.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando idNFe: " + queryNfe.lastError().text());
    return false;
  }

  const int id = queryNfe.value("numeroNFe").toInt();

  ui->lineEditNumero->setText(QString("%1").arg(id, 9, 10, QChar('0')));
  ui->lineEditCodigo->setText(QString("%1").arg(id, 8, 10, QChar('0')));

  return true;
}

void CadastrarNFe::prepararNFe(const QList<int> &items) {
  QString filter;

  for (auto const &item : items) {
    filter += QString(filter.isEmpty() ? "" : " OR ") + "idVendaProduto = " + QString::number(item);

    QSqlQuery query;
    query.prepare("SELECT idVendaProduto FROM view_produto_estoque WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", item);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "idVendaProduto " + QString::number(item) + " não encontrado na tabela!");
      return;
    }
  }

  modelProd.setFilter(filter);

  if (not modelProd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de itens da venda: " + modelProd.lastError().text());
    return;
  }

  ui->tableItens->resizeColumnsToContents();

  preencherNumeroNFe();

  ui->lineEditNatureza->setText("VENDA");
  ui->lineEditModelo->setText("55");
  ui->lineEditSerie->setText("001");
  ui->lineEditEmissao->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditSaida->setText(QDate::currentDate().toString("dd/MM/yy"));
  ui->lineEditTipo->setText("1");
  ui->lineEditFormatoPagina->setText("0");

  //-----------------------

  QSqlQuery queryEmitente;
  queryEmitente.prepare("SELECT razaoSocial, nomeFantasia, cnpj, inscEstadual, tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryEmitente.bindValue(":idLoja", ui->itemBoxLoja->getValue());

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
  queryEmitenteEndereco.bindValue(":idLoja", ui->itemBoxLoja->getValue());

  if (not queryEmitenteEndereco.exec() or not queryEmitenteEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do emitente: " + queryEmitenteEndereco.lastError().text());
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
  queryDestinatario.prepare("SELECT nome_razao, pfpj, cpf, cnpj, inscEstadual, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  queryDestinatario.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

  if (not queryDestinatario.exec() or not queryDestinatario.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo dados do cliente: " + queryDestinatario.lastError().text());
    return;
  }

  ui->lineEditDestinatarioNomeRazao->setText(queryDestinatario.value("nome_razao").toString());
  ui->lineEditDestinatarioCPFCNPJ->setText(queryDestinatario.value(queryDestinatario.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString());
  ui->lineEditDestinatarioInscEst->setText(queryDestinatario.value("inscEstadual").toString());
  ui->lineEditDestinatarioTel1->setText(queryDestinatario.value("tel").toString());
  ui->lineEditDestinatarioTel2->setText(queryDestinatario.value("telCel").toString());

  ui->itemBoxCliente->setValue(modelVenda.data(0, "idCliente"));

  // endereco faturamento

  ui->itemBoxEnderecoFaturamento->getSearchDialog()->setFilter("idCliente = " + modelVenda.data(0, "idCliente").toString() + " AND desativado = FALSE OR idEndereco = 1");
  ui->itemBoxEnderecoFaturamento->setValue(modelVenda.data(0, "idEnderecoFaturamento"));

  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", modelVenda.data(0, "idEnderecoFaturamento"));

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
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

  ui->itemBoxEnderecoEntrega->getSearchDialog()->setFilter("idCliente = " + modelVenda.data(0, "idCliente").toString() + " AND desativado = FALSE OR idEndereco = 1");
  ui->itemBoxEnderecoEntrega->setValue(modelVenda.data(0, "idEnderecoEntrega"));

  queryDestinatarioEndereco.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", modelVenda.data(0, "idEnderecoEntrega"));

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
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

  double valorProdutos = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) valorProdutos += modelProd.data(row, "total").toDouble();

  const double frete = valorProdutos / (modelVenda.data(0, "subTotalLiq").toDouble() - modelVenda.data(0, "descontoReais").toDouble()) * modelVenda.data(0, "frete").toDouble();

  ui->doubleSpinBoxValorProduto->setValue(valorProdutos);
  ui->doubleSpinBoxValorFrete->setValue(frete);
  ui->doubleSpinBoxValorNota->setValue(valorProdutos + frete);

  //------------------------

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    for (int col = modelProd.fieldIndex("numeroPedido"); col < modelProd.columnCount(); ++col) {
      modelProd.setData(row, col, 0); // limpar campos dos imposto
    }

    modelProd.setData(row, "cfop", "5403");

    modelProd.setData(row, "tipoICMS", "ICMS60");
    modelProd.setData(row, "cstICMS", "60");

    const double total = modelProd.data(row, "total").toDouble();
    const double freteProporcional = total == 0 ? 0 : total / ui->doubleSpinBoxValorProduto->value() * ui->doubleSpinBoxValorFrete->value();

    modelProd.setData(row, "vBCPIS", total + freteProporcional);
    modelProd.setData(row, "cstPIS", "01");
    modelProd.setData(row, "pPIS", 1.65);
    modelProd.setData(row, "vPIS", modelProd.data(row, "vBCPIS").toDouble() * modelProd.data(row, "pPIS").toDouble() / 100);

    modelProd.setData(row, "vBCCOFINS", total + freteProporcional);
    modelProd.setData(row, "cstCOFINS", "01");
    modelProd.setData(row, "pCOFINS", 7.6);
    modelProd.setData(row, "vCOFINS", modelProd.data(row, "vBCCOFINS").toDouble() * modelProd.data(row, "pCOFINS").toDouble() / 100);
  }

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

  const QString endereco = queryTransp.value("logradouro").toString() + " - " + queryTransp.value("numero").toString() + " - " + queryTransp.value("complemento").toString() + " - " +
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
  double caixas = 0;
  double peso = 0;

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    caixas += modelProd.data(row, "caixas").toDouble();

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

bool CadastrarNFe::criarChaveAcesso() {
  QStringList listChave;

  listChave << modelLoja.data(0, "codUF").toString();
  listChave << QDate::currentDate().toString("yyMM");      // Ano/Mês
  listChave << clearStr(ui->lineEditEmitenteCNPJ->text()); // CNPJ
  listChave << ui->lineEditModelo->text();                 // modelo NFe
  listChave << ui->lineEditSerie->text();
  listChave << ui->lineEditNumero->text(); // número nf-e (id interno)
  listChave << ui->lineEditTipo->text();   // tpEmis - forma de emissão
  listChave << ui->lineEditCodigo->text(); // código númerico aleatorio

  chaveNum = listChave.join("");

  return calculaDigitoVerificador(chaveNum);
}

QString CadastrarNFe::clearStr(const QString &str) const { return QString(str).remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")"); }

bool CadastrarNFe::calculaDigitoVerificador(QString &chave) {
  if (chave.size() != 43) {
    QMessageBox::critical(this, "Erro!", "Erro no tamanho da chave: " + chave);
    return false;
  }

  int soma = 0;
  int mult = 4;

  for (auto const &i : chave) {
    soma += i.digitValue() * mult--;
    mult = mult == 1 ? 9 : mult;
  }

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

void CadastrarNFe::writeEmitente(QTextStream &stream) const {
  stream << "[Emitente]" << endl;
  stream << "CNPJ = " + clearStr(modelLoja.data(0, "cnpj").toString()) << endl;
  stream << "IE = " + modelLoja.data(0, "inscEstadual").toString() << endl;
  stream << "Razao = " + modelLoja.data(0, "razaoSocial").toString() << endl;
  stream << "Fantasia = " + modelLoja.data(0, "nomeFantasia").toString() << endl;
  stream << "Fone = " + modelLoja.data(0, "tel").toString() << endl;
  stream << "CEP = " + clearStr(queryLojaEnd.value("CEP").toString()) << endl;
  stream << "Logradouro = " + queryLojaEnd.value("logradouro").toString() << endl;
  stream << "Numero = " + queryLojaEnd.value("numero").toString() << endl;
  stream << "Complemento = " + queryLojaEnd.value("complemento").toString() << endl;
  stream << "Bairro = " + queryLojaEnd.value("bairro").toString() << endl;
  stream << "Cidade = " + queryLojaEnd.value("cidade").toString() << endl;
  stream << "UF = " + queryLojaEnd.value("uf").toString() << endl;
}

void CadastrarNFe::writeDestinatario(QTextStream &stream) const {
  stream << "[Destinatario]" << endl;
  stream << "NomeRazao = " + queryCliente.value("nome_razao").toString() << endl;

  if (queryCliente.value("pfpj").toString() == "PF") {
    stream << "CPF = " + clearStr(queryCliente.value("cpf").toString()) << endl;
    stream << "indIEDest = 9" << endl;
  }

  if (queryCliente.value("pfpj").toString() == "PJ") {
    stream << "CNPJ = " + clearStr(queryCliente.value("cnpj").toString()) << endl;

    const QString inscEst = queryCliente.value("inscEstadual").toString();

    stream << (inscEst == "ISENTO" or inscEst.isEmpty() ? "indIEDest = 9" : "IE = " + clearStr(inscEst)) << endl;
  }

  stream << "Fone = " + queryCliente.value("tel").toString() << endl;
  stream << "CEP = " + clearStr(queryEndereco.value("cep").toString()) << endl;
  stream << "Logradouro = " + queryEndereco.value("logradouro").toString().left(60) << endl;
  stream << "Numero = " + queryEndereco.value("numero").toString() << endl;
  stream << "Complemento = " + queryEndereco.value("complemento").toString().left(60) << endl;
  stream << "Bairro = " + queryEndereco.value("bairro").toString() << endl;
  stream << "cMun = " + queryIBGE.value("CT_IBGE").toString() << endl;
  stream << "Cidade = " + queryEndereco.value("cidade").toString() << endl;
  stream << "UF = " + queryEndereco.value("uf").toString() << endl;
}

void CadastrarNFe::writeProduto(QTextStream &stream) const {
  for (int row = 0; row < modelProd.rowCount(); ++row) {
    const QString numProd = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding with zeros
    stream << "[Produto" + numProd + "]" << endl;
    stream << "CFOP = " + modelProd.data(row, "cfop").toString() << endl;
    stream << "CEST = 1003001" << endl;
    stream << "NCM = " + modelProd.data(row, "ncm").toString() << endl;
    stream << "Codigo = " + modelProd.data(row, "codComercial").toString() << endl;
    const QString codBarras = modelProd.data(row, "codBarras").toString();
    stream << "EAN = " + (codBarras.isEmpty() ? "" : codBarras) << endl;
    const QString produto = modelProd.data(row, "produto").toString();
    QString formato = modelProd.data(row, "formComercial").toString();
    formato = formato.isEmpty() ? "" : " - " + formato;
    const QString caixas = modelProd.data(row, "caixas").toString();
    stream << "Descricao = " + produto + formato + " (" + caixas + " Cx.)" << endl;
    stream << "Unidade = " + modelProd.data(row, "un").toString() << endl;
    stream << "Quantidade = " + modelProd.data(row, "quant").toString() << endl;
    const double total = modelProd.data(row, "total").toDouble();
    const double quant = modelProd.data(row, "quant").toDouble();
    stream << "ValorUnitario = " + QString::number(total / quant, 'f', 10) << endl;
    stream << "ValorTotal = " + modelProd.data(row, "total").toString() << endl;
    const double frete = total / ui->doubleSpinBoxValorProduto->value() * ui->doubleSpinBoxValorFrete->value();
    stream << "vFrete = " + QString::number(frete) << endl;

    stream << "[ICMS" + numProd + "]" << endl;
    stream << "CST = " + modelProd.data(row, "cstICMS").toString() << endl;
    stream << "Modalidade = " + modelProd.data(row, "modBC").toString() << endl;
    stream << "ValorBase = " + modelProd.data(row, "vBC").toString() << endl;
    const double aliquota = modelProd.data(row, "pICMS").toDouble();
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

void CadastrarNFe::writeTransportadora(QTextStream &stream) const {
  stream << "[Transportador]" << endl;
  stream << "FretePorConta = " << ui->comboBoxFreteConta->currentText().left(1) << endl;

  if (ui->lineEditTransportadorRazaoSocial->text() != "RETIRA") {
    stream << "NomeRazao = " << ui->lineEditTransportadorRazaoSocial->text() << endl;
    stream << "CnpjCpf = " << ui->lineEditTransportadorCpfCnpj->text() << endl;
    stream << "IE = " << ui->lineEditTransportadorInscEst->text() << endl;
    stream << "Endereco = " << ui->lineEditTransportadorEndereco->text() << endl;
    stream << "Cidade = " << ui->lineEditTransportadorMunicipio->text() << endl;
    stream << "UF = " << ui->lineEditTransportadorUf->text() << endl;
    stream << "ValorServico = " << endl;
    stream << "ValorBase = " << endl;
    stream << "Aliquota = " << endl;
    stream << "Valor = " << endl;
    stream << "CFOP = " << endl;
    stream << "CidadeCod = " << endl;
    stream << "Placa = " << ui->lineEditTransportadorPlaca->text().remove("-") << endl;
    stream << "UFPlaca = " << ui->lineEditTransportadorUfPlaca->text() << endl;
    stream << "RNTC = " << endl;
  }
}

void CadastrarNFe::writeVolume(QTextStream &stream) const {
  stream << "[Volume001]" << endl;

  stream << "Quantidade = " << ui->spinBoxVolumesQuant->text() << endl;
  stream << "Especie = " << ui->lineEditVolumesEspecie->text() << endl;
  stream << "Marca = " << ui->lineEditVolumesMarca->text() << endl;
  stream << "Numeracao = " << ui->lineEditVolumesNumeracao->text() << endl;
  stream << "PesoLiquido = " << QString::number(ui->doubleSpinBoxVolumesPesoLiq->value()) << endl;
  stream << "PesoBruto = " << QString::number(ui->doubleSpinBoxVolumesPesoBruto->value()) << endl;
}

// TODO: 1criar tela para configurar emails de saida: inicialmente para contabilidade, posteriormente para cliente
// tambem

void CadastrarNFe::on_tableItens_entered(const QModelIndex &) { ui->tableItens->resizeColumnsToContents(); }

void CadastrarNFe::on_tableItens_clicked(const QModelIndex &index) {
  ui->groupBox_7->setEnabled(true);

  ui->comboBoxCfop->setCurrentIndex(ui->comboBoxCfop->findText(modelProd.data(index.row(), "cfop").toString(), Qt::MatchStartsWith));
  ui->comboBoxICMSOrig->setCurrentIndex(ui->comboBoxICMSOrig->findText(modelProd.data(index.row(), "orig").toString(), Qt::MatchStartsWith));
  ui->comboBoxSituacaoTributaria->setCurrentIndex(ui->comboBoxSituacaoTributaria->findText(modelProd.data(index.row(), "cstICMS").toString(), Qt::MatchStartsWith));
  ui->comboBoxICMSModBc->setCurrentIndex(modelProd.data(index.row(), "modBC").toInt());
  ui->comboBoxICMSModBcSt->setCurrentIndex(modelProd.data(index.row(), "modBCST").toInt());
  ui->comboBoxIPIcst->setCurrentIndex(ui->comboBoxIPIcst->findText(modelProd.data(index.row(), "cstIPI").toString(), Qt::MatchStartsWith));
  ui->comboBoxPIScst->setCurrentIndex(ui->comboBoxPIScst->findText(modelProd.data(index.row(), "cstPIS").toString(), Qt::MatchStartsWith));
  ui->comboBoxCOFINScst->setCurrentIndex(ui->comboBoxCOFINScst->findText(modelProd.data(index.row(), "cstCOFINS").toString(), Qt::MatchStartsWith));

  QSqlQuery query;
  query.prepare("SELECT tipoICMS, cfop, orig, cstICMS, modBC, vBC, pICMS, vICMS, modBCST, pMVAST, vBCST, pICMSST, vICMSST, "
                "cEnq, cstIPI, cstPIS, vBCPIS, pPIS, vPIS, cstCOFINS, vBCCOFINS, pCOFINS, vCOFINS FROM "
                "view_produto_estoque WHERE idVendaProduto = :idVendaProduto");
  query.bindValue(":idVendaProduto", modelProd.data(index.row(), "idVendaProduto"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando impostos do produto: " + query.lastError().text());
    return;
  }

  ui->comboBoxRegime_2->setCurrentIndex(query.value("tipoICMS").toString().length() == 6 ? 1 : 2); // ICMSXX : ICMSSQN

  ui->comboBoxRegime->setCurrentText(query.value("tipoICMS").toString().length() == 6 ? "Tributação Normal" : "Simples Nacional");

  QSqlQuery queryCfop;
  queryCfop.prepare("SELECT NAT FROM cfop_sai WHERE cfop_de = :cfop OR cfop_fe = :cfop");
  queryCfop.bindValue(":cfop", query.value("cfop"));

  if (not queryCfop.exec() or not queryCfop.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando CFOP: " + queryCfop.lastError().text());
    return;
  }

  // ICMS
  ui->comboBoxCfop_2->setCurrentText(query.value("cfop").toString() + " - " + queryCfop.value("NAT").toString());
  ui->comboBoxICMSOrig_2->setCurrentIndex(ui->comboBoxICMSOrig_2->findText(query.value("orig").toString(), Qt::MatchStartsWith));
  ui->comboBoxSituacaoTributaria_2->setCurrentIndex(ui->comboBoxSituacaoTributaria_2->findText(query.value("cstICMS").toString(), Qt::MatchStartsWith));
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
  ui->comboBoxIPIcst_2->setCurrentIndex(ui->comboBoxIPIcst_2->findText(query.value("cstIPI").toString(), Qt::MatchStartsWith));
  ui->lineEditIPIcEnq_3->setText(query.value("cEnq").toString());

  // PIS
  ui->comboBoxPIScst_2->setCurrentIndex(ui->comboBoxPIScst_2->findText(query.value("cstPIS").toString(), Qt::MatchStartsWith));
  ui->doubleSpinBoxPISvbc_3->setValue(query.value("vBCPIS").toDouble());
  ui->doubleSpinBoxPISppis_3->setValue(query.value("pPIS").toDouble());
  ui->doubleSpinBoxPISvpis_3->setValue(query.value("vPIS").toDouble());

  // COFINS
  ui->comboBoxCOFINScst_3->setCurrentIndex(ui->comboBoxCOFINScst_3->findText(query.value("cstCOFINS").toString(), Qt::MatchStartsWith));
  ui->doubleSpinBoxCOFINSvbc_4->setValue(query.value("vBCCOFINS").toDouble());
  ui->doubleSpinBoxCOFINSpcofins_4->setValue(query.value("pCOFINS").toDouble());
  ui->doubleSpinBoxCOFINSvcofins_4->setValue(query.value("vCOFINS").toDouble());

  //

  mapper.setCurrentModelIndex(index);
}

void CadastrarNFe::on_tabWidget_currentChanged(int index) {
  if (index == 4) updateImpostos();
}

void CadastrarNFe::on_doubleSpinBoxICMSvbc_valueChanged(double) { ui->doubleSpinBoxICMSvicms->setValue(ui->doubleSpinBoxICMSvbc->value() * ui->doubleSpinBoxICMSpicms->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxICMSvbcst_valueChanged(double) { ui->doubleSpinBoxICMSvicmsst->setValue(ui->doubleSpinBoxICMSvbcst->value() * ui->doubleSpinBoxICMSpicmsst->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxICMSpicms_valueChanged(double) { ui->doubleSpinBoxICMSvicms->setValue(ui->doubleSpinBoxICMSvbc->value() * ui->doubleSpinBoxICMSpicms->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxPISvbc_valueChanged(double) { ui->doubleSpinBoxPISvpis->setValue(ui->doubleSpinBoxPISvbc->value() * ui->doubleSpinBoxPISppis->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxPISppis_valueChanged(double) { ui->doubleSpinBoxPISvpis->setValue(ui->doubleSpinBoxPISvbc->value() * ui->doubleSpinBoxPISppis->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxCOFINSvbc_valueChanged(double) { ui->doubleSpinBoxCOFINSvcofins->setValue(ui->doubleSpinBoxCOFINSvbc->value() * ui->doubleSpinBoxCOFINSpcofins->value() / 100); }

void CadastrarNFe::on_doubleSpinBoxCOFINSpcofins_valueChanged(double) { ui->doubleSpinBoxCOFINSvcofins->setValue(ui->doubleSpinBoxCOFINSvbc->value() * ui->doubleSpinBoxCOFINSpcofins->value() / 100); }

void CadastrarNFe::on_itemBoxEnderecoFaturamento_textChanged(const QString &) {
  QSqlQuery queryDestinatarioEndereco;
  queryDestinatarioEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM "
                                    "cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryDestinatarioEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoFaturamento->getValue());

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
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
  queryDestinatarioEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoEntrega->getValue());

  if (not queryDestinatarioEndereco.exec() or not queryDestinatarioEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do cliente: " + queryDestinatarioEndereco.lastError().text());
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
                        "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por substituição tributária",
                        "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por substituição tributária",
                        "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS por substituição tributária",
                        "300 - Imune",
                        "400 - Não tributada pelo Simples Nacional",
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
                        "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por substituição tributária",
                        "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por substituição tributária",
                        "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS por substituição tributária",
                        "300 - Imune",
                        "400 - Não tributada pelo Simples Nacional",
                        "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação",
                        "900 - Outros"};

    ui->comboBoxSituacaoTributaria_2->clear();
    ui->comboBoxSituacaoTributaria_2->addItems(list);
  }
}

void CadastrarNFe::on_comboBoxSituacaoTributaria_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  const bool tribNormal = ui->comboBoxRegime->currentText() == "Tributação Normal";

  modelProd.setData(list.first().row(), "tipoICMS", tribNormal ? "ICMS" + text.left(2) : "ICMSSN" + text.left(3));
  modelProd.setData(list.first().row(), "cstICMS", text.left(tribNormal ? 2 : 3));

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

  if (text == "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "400 - Não tributada pelo Simples Nacional") {
  }

  if (text == "300 - Imune") {
  }

  if (text == "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação") {
  }

  if (text == "900 - Outros") {
  }
}

void CadastrarNFe::on_comboBoxSituacaoTributaria_2_currentTextChanged(const QString &text) {
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

  if (text == "201 - Tributada pelo Simples Nacional com permissão de crédito e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "202 - Tributada pelo Simples Nacional sem permissão de crédito e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "203 - Isenção do ICMS no Simples Nacional para faixa de receita bruta e com cobrança do ICMS por substituição tributária") {
  }

  if (text == "400 - Não tributada pelo Simples Nacional") {
  }

  if (text == "300 - Imune") {
  }

  if (text == "500 - ICMS cobrado anteriormente por substituição tributária (substituído) ou por antecipação") {
  }

  if (text == "900 - Outros") {
  }
}

void CadastrarNFe::on_comboBoxICMSOrig_currentIndexChanged(int index) {
  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "orig", index - 1);
}

void CadastrarNFe::on_comboBoxICMSModBc_currentIndexChanged(int index) {
  // modBC

  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "modBC", index - 1);
}

void CadastrarNFe::on_comboBoxICMSModBcSt_currentIndexChanged(int index) {
  // modBCST

  if (index == 0) return;

  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "modBCST", index - 1);
}

void CadastrarNFe::on_doubleSpinBoxICMSvicms_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "vICMS", value);
}

void CadastrarNFe::on_doubleSpinBoxICMSvicmsst_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "vICMSST", value);
}

void CadastrarNFe::on_comboBoxIPIcst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "cstIPI", text.left(2));
}

void CadastrarNFe::on_comboBoxPIScst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "cstPIS", text.left(2));
}

void CadastrarNFe::on_doubleSpinBoxPISvpis_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "vPIS", value);
}

void CadastrarNFe::on_comboBoxCOFINScst_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "cstCOFINS", text.left(2));
}

void CadastrarNFe::on_doubleSpinBoxCOFINSvcofins_valueChanged(double value) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "vCOFINS", value);
}

void CadastrarNFe::on_doubleSpinBoxICMSpicmsst_valueChanged(double) { ui->doubleSpinBoxICMSvicmsst->setValue(ui->doubleSpinBoxICMSvbcst->value() * ui->doubleSpinBoxICMSpicmsst->value() / 100); }

void CadastrarNFe::on_itemBoxVeiculo_textChanged(const QString &) {
  QSqlQuery queryTransp;
  queryTransp.prepare("SELECT t.cnpj, t.razaoSocial, t.inscEstadual, the.logradouro, the.numero, the.complemento, the.bairro, "
                      "the.cidade, the.uf, thv.placa, thv.ufPlaca, t.antt FROM transportadora_has_veiculo thv LEFT JOIN transportadora "
                      "t ON thv.idTransportadora = t.idTransportadora LEFT JOIN transportadora_has_endereco the ON "
                      "the.idTransportadora = t.idTransportadora WHERE thv.idVeiculo = :idVeiculo");
  queryTransp.bindValue(":idVeiculo", ui->itemBoxVeiculo->getValue());

  if (not queryTransp.exec() or not queryTransp.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da transportadora: " + queryTransp.lastError().text());
    return;
  }

  const QString endereco = queryTransp.value("logradouro").toString() + " - " + queryTransp.value("numero").toString() + " - " + queryTransp.value("complemento").toString() + " - " +
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
}

void CadastrarNFe::on_itemBoxCliente_textChanged(const QString &) {
  QSqlQuery query;
  query.prepare("SELECT nome_razao, pfpj, cpf, cnpj, inscEstadual, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  query.bindValue(":idCliente", ui->itemBoxCliente->getValue());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do cliente: " + query.lastError().text());
    return;
  }

  ui->lineEditDestinatarioNomeRazao->setText(query.value("nome_razao").toString());
  ui->lineEditDestinatarioCPFCNPJ->setText(query.value(query.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString());
  ui->lineEditDestinatarioInscEst->setText(query.value("inscEstadual").toString());
  ui->lineEditDestinatarioTel1->setText(query.value("tel").toString());
  ui->lineEditDestinatarioTel2->setText(query.value("telCel").toString());

  ui->itemBoxEnderecoFaturamento->getSearchDialog()->setFilter("idCliente = " + ui->itemBoxCliente->getValue().toString() + " AND desativado = FALSE OR idEndereco = 1");
  ui->itemBoxEnderecoFaturamento->setValue(1);

  ui->itemBoxEnderecoEntrega->getSearchDialog()->setFilter("idCliente = " + ui->itemBoxCliente->getValue().toString() + " AND desativado = FALSE OR idEndereco = 1");

  ui->itemBoxEnderecoEntrega->setValue(1);
}

bool CadastrarNFe::validar() {
  // NOTE: implement validation rules

  // validacao do model

  // TODO: recalcular todos os valores dos itens para verificar se os dados batem (usar o 174058 de referencia)

  //

  if (ui->itemBoxLoja->text().isEmpty()) {
    // assume no certificate in acbr
    QMessageBox::critical(this, "Erro!", "Escolha um certificado para o ACBr!");
    return false;
  }

  // [Emitente]

  if (clearStr(modelLoja.data(0, "cnpj").toString()).isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CNPJ emitente vazio!");
    return false;
  }

  if (modelLoja.data(0, "razaoSocial").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Razão Social emitente vazio!");
    return false;
  }

  if (modelLoja.data(0, "nomeFantasia").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nome Fantasia emitente vazio!");
    return false;
  }

  if (modelLoja.data(0, "tel").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Telefone emitente vazio!");
    return false;
  }

  queryLojaEnd.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", modelLoja.data(0, "idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de endereços da loja: " + queryLojaEnd.lastError().text());
    return false;
  }

  if (clearStr(queryLojaEnd.value("CEP").toString()).isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CEP vazio!");
    return false;
  }

  if (queryLojaEnd.value("logradouro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Logradouro vazio!");
    return false;
  }

  if (queryLojaEnd.value("numero").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Número vazio!");
    return false;
  }

  if (queryLojaEnd.value("bairro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Bairro vazio!");
    return false;
  }

  if (queryLojaEnd.value("cidade").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cidade vazio!");
    return false;
  }

  if (queryLojaEnd.value("uf").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "UF vazio!");
    return false;
  }

  // [Destinatario]

  if (modelVenda.data(0, "idCliente").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "idCliente vazio!");
    return false;
  }

  queryCliente.prepare("SELECT nome_razao, pfpj, cpf, cnpj, inscEstadual, tel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço do destinatário: " + queryCliente.lastError().text());
    return false;
  }

  if (queryCliente.value("nome_razao").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nome/Razão vazio!");
    return false;
  }

  if (queryCliente.value("pfpj").toString() == "PF") {
    if (clearStr(queryCliente.value("cpf").toString()).isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CPF vazio!");
      return false;
    }
  }

  if (queryCliente.value("pfpj").toString() == "PJ") {
    if (clearStr(queryCliente.value("cnpj").toString()).isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CNPJ destinatário vazio!");
      return false;
    }
  }

  queryEndereco.prepare("SELECT cep, logradouro, numero, complemento, bairro, cidade, uf FROM cliente_has_endereco WHERE "
                        "idEndereco = :idEndereco");
  queryEndereco.bindValue(":idEndereco", ui->itemBoxEnderecoFaturamento->getValue());

  if (not queryEndereco.exec() or not queryEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço cliente: " + queryEndereco.lastError().text());
    return false;
  }

  if (queryEndereco.value("cep").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "CEP cliente vazio!");
    return false;
  }

  if (queryEndereco.value("logradouro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Logradouro cliente vazio!");
    return false;
  }

  if (queryEndereco.value("numero").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Número Endereço do Cliente vazio!");
    return false;
  }

  if (queryEndereco.value("bairro").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Bairro cliente vazio!");
    return false;
  }

  queryIBGE.prepare("SELECT CT_IBGE FROM cidade WHERE CT_NOME = :cidade"); // add uf to filter
  queryIBGE.bindValue(":cidade", queryEndereco.value("cidade"));

  if (not queryIBGE.exec() or not queryIBGE.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando código do munícipio");
    return false;
  }

  if (queryEndereco.value("cidade").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cidade cliente vazio!");
    return false;
  }

  if (queryEndereco.value("uf").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "UF cliente vazio!");
    return false;
  }

  // [Produto]

  for (int row = 0; row < modelProd.rowCount(); ++row) {
    if (modelProd.data(row, "cfop").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "CFOP vazio!");
      return false;
    }

    if (modelProd.data(row, "ncm").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "NCM vazio!");
      return false;
    }

    if (modelProd.data(row, "codComercial").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Código vazio!");
      return false;
    }

    if (modelProd.data(row, "produto").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Descrição vazio!");
      return false;
    }

    if (modelProd.data(row, "un").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Unidade vazio!");
      return false;
    }

    if (modelProd.data(row, "quant").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Quantidade vazio!");
      return false;
    }

    if (modelProd.data(row, "descUnitario").toDouble() == 0.) {
      QMessageBox::critical(this, "Erro!", "Preço venda = R$ 0!");
      return false;
    }

    if (modelProd.data(row, "total").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Total produto vazio!");
      return false;
    }
  }

  return true;
}

void CadastrarNFe::on_comboBoxCfop_currentTextChanged(const QString &text) {
  const auto list = ui->tableItens->selectionModel()->selectedRows();

  if (list.isEmpty()) return;

  modelProd.setData(list.first().row(), "cfop", text.left(4));
}

void CadastrarNFe::on_pushButtonConsultarCadastro_clicked() {
  QString resposta;

  ACBr::enviarComando("NFE.ConsultaCadastro(" + ui->lineEditDestinatarioUF->text() + ", " + ui->lineEditDestinatarioCPFCNPJ->text() + ")", resposta);

  if (resposta.contains("xMotivo=Consulta cadastro com uma ocorrência")) {
    QStringList list = resposta.mid(resposta.indexOf("IE=")).split("\n");
    const QString insc = list.first().remove("IE=");

    if (not insc.isEmpty()) {
      ui->lineEditDestinatarioInscEst->setText(insc);

      QSqlQuery query;
      query.prepare("UPDATE cliente SET inscEstadual = :inscEstadual WHERE idCliente = :idCliente");
      query.bindValue(":inscEstadual", insc);
      query.bindValue(":idCliente", modelVenda.data(0, "idCliente"));

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro atualizando Insc. Est.: " + query.lastError().text());
        return;
      }
    }
  }

  QMessageBox::information(this, "Resposta", resposta);
}

void CadastrarNFe::on_doubleSpinBoxValorFrete_valueChanged(double value) {
  // TODO: 1refazer rateamento do frete
  Q_UNUSED(value)
}

void CadastrarNFe::alterarCertificado(const QString &text) {
  if (text.isEmpty()) return;

  QString resposta;

  QSqlQuery query;
  query.prepare("SELECT certificadoSerie, certificadoSenha FROM loja WHERE idLoja = :idLoja AND certificadoSerie IS NOT NULL");
  query.bindValue(":idLoja", ui->itemBoxLoja->getValue());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando certificado: " + query.lastError().text());
    return;
  }

  if (not query.first()) {
    QMessageBox::critical(this, "Erro!", "A loja selecionada não possui certificado cadastrado no sistema!");
    return;
  }

  ACBr::enviarComando("NFE.SetCertificado(" + query.value("certificadoSerie").toString() + "," + query.value("certificadoSenha").toString() + ")", resposta);

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Erro!", resposta);
    ui->itemBoxLoja->clear();
    return;
  }

  preencherNumeroNFe();

  QSqlQuery queryEmitente;
  queryEmitente.prepare("SELECT razaoSocial, nomeFantasia, cnpj, inscEstadual, tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryEmitente.bindValue(":idLoja", ui->itemBoxLoja->getValue());

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
  queryEmitenteEndereco.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryEmitenteEndereco.bindValue(":idLoja", ui->itemBoxLoja->getValue());

  if (not queryEmitenteEndereco.exec() or not queryEmitenteEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo endereço do emitente: " + queryEmitenteEndereco.lastError().text());
    return;
  }

  ui->lineEditEmitenteLogradouro->setText(queryEmitenteEndereco.value("logradouro").toString());
  ui->lineEditEmitenteNumero->setText(queryEmitenteEndereco.value("numero").toString());
  ui->lineEditEmitenteComplemento->setText(queryEmitenteEndereco.value("complemento").toString());
  ui->lineEditEmitenteBairro->setText(queryEmitenteEndereco.value("bairro").toString());
  ui->lineEditEmitenteCidade->setText(queryEmitenteEndereco.value("cidade").toString());
  ui->lineEditEmitenteUF->setText(queryEmitenteEndereco.value("uf").toString());
  ui->lineEditEmitenteCEP->setText(queryEmitenteEndereco.value("cep").toString());

  // TODO: pedir para trocar cartao e rodar teste no acbr para verificar se esta tudo ok e funcionando
}

// TODO: colocar NCM para poder ser alterado na caixinha em baixo
// TODO: 2gerar protocolo entrega
// TODO: 3criar logo para nota
// TODO: verificar com Anderson rateamento de frete
// TODO: bloquear edicao direto na tabela
// usar dois campos de texto, um read-only com as informacoes geradas pelo sistema
