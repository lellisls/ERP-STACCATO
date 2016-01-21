#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "impressao.h"
#include "qtrpt.h"
#include "usersession.h"

Impressao::Impressao(QString id, QWidget *parent) : QObject(parent), id(id), parent(parent) {
  verificaTipo();

  model.setTable(type == Orcamento ? "orcamento" : "venda");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter(type == Orcamento ? "idOrcamento = '" + id + "'" : "idVenda = '" + id + "'");
  model.select();

  modelItem.setTable((type == Orcamento ? "orcamento" : "venda") + QString("_has_produto"));
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setFilter(type == Orcamento ? "idOrcamento = '" + id + "'" : "idVenda = '" + id + "'");
  modelItem.select();
}

void Impressao::verificaTipo() {
  QSqlQuery query;
  query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", id);

  if (not query.exec()) {
    QMessageBox::critical(parent, "Erro!", "Erro verificando se id é Orçamento!");
    return;
  }

  type = query.first() ? Orcamento : Venda;
}

void Impressao::print() {
  if (UserSession::settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::critical(parent, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    UserSession::setSettings("User/userFolder", QFileDialog::getExistingDirectory(parent, "Pasta PDF/Excel"));

    if (UserSession::settings("User/userFolder").toString().isEmpty()) return;
  }

  QString path = UserSession::settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists() and not dir.mkdir(path)) {
    QMessageBox::critical(parent, "Erro!", "Erro ao criar a pasta escolhida nas configurações!");
    return;
  }

  QFile modelo(qApp->applicationDirPath() + (type == Orcamento ? "/orcamento.xml" : "/venda.xml"));

  if (not modelo.exists()) {
    QMessageBox::critical(parent, "Erro!", "XML da impressão não encontrado!");
    return;
  }

  QFile file(path + "/" + id + ".pdf");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(parent, "Erro!", "Arquivo bloqueado! Por favor feche o arquivo.");
    return;
  }

  file.close();

  setQuerys();

  QtRPT *report = new QtRPT(this);
  report->loadReport(modelo.fileName());
  report->recordCount << modelItem.rowCount();
  connect(report, &QtRPT::setValue, this, &Impressao::setValue);
  report->printPDF(path + "/" + id + ".pdf");
}

void Impressao::setValue(const int &recNo, const QString &paramName, QVariant &paramValue, const int &reportPage) {
  Q_UNUSED(reportPage);

  QLocale locale;

  if (modelItem.data(recNo, "idProduto") != queryProduto.boundValue(":idProduto")) {
    queryProduto.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
    queryProduto.bindValue(":idProduto", modelItem.data(recNo, "idProduto"));

    if (not queryProduto.exec() or not queryProduto.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando produto: " + queryProduto.lastError().text());
      return;
    }
  }

  // REPORT TITLE
  if (paramName == "Loja") paramValue = queryLoja.value("descricao");

  if (paramName == "Endereco")
    paramValue = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + " - " +
                 queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " +
                 queryLojaEnd.value("uf").toString() + " - CEP: " + queryLojaEnd.value("cep").toString() + "\n" +
                 queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();

  if (paramName == "orcamento") paramValue = id;
  if (paramName == "pedido") paramValue = id;
  if (paramName == "data") paramValue = query.value("data").toDate().toString("dd-MM-yyyy");
  if (paramName == "validade") paramValue = query.value("validade").toString() + " dias";
  if (paramName == "cliente") paramValue = queryCliente.value("nome_razao");
  if (paramName == "cpfcnpj") paramValue = queryCliente.value(queryCliente.value("pfpj") == "PF" ? "cpf" : "cnpj");
  if (paramName == "email") paramValue = queryCliente.value("email");
  if (paramName == "tel1") paramValue = queryCliente.value("tel");
  if (paramName == "tel2") paramValue = queryCliente.value("telCel");
  if (paramName == "cepfiscal") paramValue = queryEndEnt.value("cep");

  if (paramName == "endfiscal")
    paramValue = query.value("idEnderecoFaturamento").toInt() == 1
                 ? "Não há/Retira"
                 : queryEndEnt.value("logradouro").toString() + " - " + queryEndEnt.value("numero").toString() +
                   " - " + queryEndEnt.value("bairro").toString() + " - " +
                   queryEndEnt.value("cidade").toString() + " - " + queryEndEnt.value("uf").toString();

  if (paramName == "endentrega")
    paramValue = query.value("idEnderecoEntrega").toInt() == 1
                 ? "Não há/Retira"
                 : queryEndEnt.value("logradouro").toString() + " - " + queryEndEnt.value("numero").toString() +
                   " - " + queryEndEnt.value("bairro").toString() + " - " +
                   queryEndEnt.value("cidade").toString() + " - " + queryEndEnt.value("uf").toString();

  if (paramName == "cepentrega") paramValue = queryEndEnt.value("cep");

  if (paramName == "profissional")
    paramValue =
        queryProfissional.value("nome_razao").toString().isEmpty() ? "Não há" : queryProfissional.value("nome_razao");

  if (paramName == "telprofissional") paramValue = queryProfissional.value("tel");
  if (paramName == "emailprofissional") paramValue = queryProfissional.value("email");
  if (paramName == "vendedor") paramValue = queryVendedor.value("nome");
  if (paramName == "emailVendedor") paramValue = queryVendedor.value("email");
  if (paramName == "estoque") paramValue = "";
  if (paramName == "dataestoque") paramValue = "";

  // MASTER BAND
  if (paramName == "Marca") paramValue = modelItem.data(recNo, "fornecedor");
  if (paramName == "Código") paramValue = queryProduto.value("codComercial");

  if (paramName == "Nome do produto") {
    QString formComercial = modelItem.data(recNo, "formComercial").toString();
    paramValue =
        modelItem.data(recNo, "produto").toString() + (formComercial.isEmpty() ? "" : " (" + formComercial + ")");
  }

  if (paramName == "Ambiente") paramValue = modelItem.data(recNo, "obs");

  if (paramName == "Preço-R$") {
    double prcUn = modelItem.data(recNo, "prcUnitario").toDouble();
    double desc = prcUn * modelItem.data(recNo, "desconto").toDouble() / 100.;
    double porc = modelItem.data(recNo, "desconto").toDouble();
    QString preco = "R$ " + locale.toString(prcUn, 'f', 2);
    QString precoDesc =
        desc > 0.01 ? " (-" + locale.toString(porc, 'f', 2) + "% R$ " + locale.toString(prcUn - desc, 'f', 2) + ")"
                    : "";
    QString precoDescNeg = "R$ " + locale.toString((porc / -100. + 1) * prcUn, 'f', 2);

    paramValue = porc < 0 ? precoDescNeg : preco + precoDesc;
  }

  if (paramName == "Quant.") paramValue = modelItem.data(recNo, "quant");
  if (paramName == "Unid.") paramValue = modelItem.data(recNo, "un");

  if (paramName == "TotalProd") {
    double prcUn = modelItem.data(recNo, "prcUnitario").toDouble();
    double desc = prcUn * modelItem.data(recNo, "desconto").toDouble() / 100.;
    double porc = modelItem.data(recNo, "desconto").toDouble();

    QString total = "R$ " + locale.toString(modelItem.data(recNo, "parcial").toDouble(), 'f', 2);
    QString totalDesc =
        desc > 0.01 ? " (R$ " + locale.toString(modelItem.data(recNo, "total").toDouble(), 'f', 2) + ")" : "";
    QString totalDescNeg = "R$ " + locale.toString(modelItem.data(recNo, "parcialDesc").toDouble(), 'f', 2);

    paramValue = porc < 0 ? totalDescNeg : total + totalDesc;
  }

  // REPORT SUMMARY
  if (paramName == "Soma") paramValue = locale.toString(query.value("subTotalLiq").toDouble(), 'f', 2);
  if (paramName == "Desconto") paramValue = locale.toString(query.value("descontoPorc").toDouble(), 'f', 2);

  if (paramName == "Total") {
    double value = query.value("subTotalLiq").toDouble() -
                   (query.value("subTotalLiq").toDouble() * query.value("descontoPorc").toDouble() / 100.);
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Frete") paramValue = locale.toString(query.value("frete").toDouble(), 'f', 2);
  if (paramName == "TotalFinal") paramValue = locale.toString(query.value("total").toDouble(), 'f', 2);
  if (paramName == "Observacao") paramValue = query.value("observacao");

  if (paramName == "Disclaimer") {
    paramValue =
        "1- O prazo de entrega deve ser consultado no momento da compra;\n2- Não aceitamos devolução de produtos "
        "calculados com percetual de perda, cortes/rodapés de porcelanatos ou mosaicos especiais;\n3- Produtos com "
        "classificação comercial \"C\" podem apresentar algum tipo de defeito, tendo valor especial por este motivo,  "
        "e devoluções não serão aceitas.";
  }

  if (paramName == "PrazoEntrega") paramValue = query.value("prazoEntrega").toString() + " dias";

  if (paramName == "FormaPagamento1") {
    if (type == Orcamento) return;

    QSqlQuery queryPgt1(
          "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
          "' AND tipo LIKE '1%'");

    if (not queryPgt1.exec() or not queryPgt1.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
      return;
    }

    paramValue = queryPgt1.value("tipo").toString() + " - " + queryPgt1.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt1.value("valor").toDouble(), 'f', 2) +
                 (queryPgt1.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt1.value("dataEmissao").toDate().toString("dd-MM-yyyy");
  }

  if (paramName == "FormaPagamento2") {
    if (type == Orcamento) return;

    QSqlQuery queryPgt2(
          "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
          "' AND tipo LIKE '2%'");

    if (not queryPgt2.exec() or not queryPgt2.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
      return;
    }

    if (queryPgt2.value("valor") == 0) return;

    paramValue = queryPgt2.value("tipo").toString() + " - " + queryPgt2.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt2.value("valor").toDouble(), 'f', 2) +
                 (queryPgt2.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt2.value("dataEmissao").toDate().toString("dd-MM-yyyy");
  }

  if (paramName == "FormaPagamento3") {
    if (type == Orcamento) return;

    QSqlQuery queryPgt3(
          "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
          "' AND tipo LIKE '3%'");

    if (not queryPgt3.exec() or not queryPgt3.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
      return;
    }

    if (queryPgt3.value("valor") == 0) return;

    paramValue = queryPgt3.value("tipo").toString() + " - " + queryPgt3.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt3.value("valor").toDouble(), 'f', 2) +
                 (queryPgt3.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt3.value("dataEmissao").toDate().toString("dd-MM-yyyy");
  }
}

void Impressao::setQuerys() {
  if (type == Orcamento) {
    query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", model.data(0, "idOrcamento"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando dados do orçamento: " + query.lastError().text());
      return;
    }
  } else {
    query.prepare("SELECT * FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", model.data(0, "idVenda"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando dados da venda: " + query.lastError().text());
      return;
    }
  }

  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", model.data(0, "idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return;
  }

  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndEnt.bindValue(":idEndereco", model.data(0, "idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando endereço: " + queryEndEnt.lastError().text());
    return;
  }

  queryEndFat.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", model.data(0, "idEnderecoFaturamento"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados do endereço: " + queryEndFat.lastError().text());
    return;
  }

  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", model.data(0, "idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return;
  }

  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", model.data(0, "idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return;
  }

  queryLoja.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", model.data(0, "idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return;
  }

  queryLojaEnd.prepare("SELECT * FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", model.data(0, "idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando loja endereço: " + queryLojaEnd.lastError().text());
    return;
  }
}
