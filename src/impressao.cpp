#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "impressao.h"
#include "usersession.h"

Impressao::Impressao(const QString &id) : id(id) {
  verificaTipo();

  modelItem.setTable((type == Orcamento ? "orcamento" : "venda") + QString("_has_produto"));
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelItem.setFilter(type == Orcamento ? "idOrcamento = '" + id + "'" : "idVenda = '" + id + "'");

  if (not modelItem.select()) QMessageBox::critical(nullptr, "Erro!", "Erro lendo tabela: " + modelItem.lastError().text());

  report = new LimeReport::ReportEngine(this);
}

void Impressao::verificaTipo() {
  QSqlQuery query;
  query.prepare("SELECT idOrcamento FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", id);

  if (not query.exec()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro verificando se id é Orçamento!");
    return;
  }

  type = query.first() ? Orcamento : Venda;
}

void Impressao::print() {
  const QString folder = type == Orcamento ? "User/OrcamentosFolder" : "User/VendasFolder";

  if (UserSession::settings(folder).toString().isEmpty()) {
    QMessageBox::critical(nullptr, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    UserSession::setSettings(folder, QFileDialog::getExistingDirectory(nullptr, "Pasta PDF/Excel"));

    if (UserSession::settings(folder).toString().isEmpty()) return;
  }

  if (not setQuerys()) {
    QMessageBox::critical(nullptr, "Erro!", "Processo interrompido, ocorreu algum erro!");
    return;
  }

  report->dataManager()->addModel(type == Orcamento ? "orcamento" : "venda", &modelItem, true);

  if (not report->loadFromFile(type == Orcamento ? "orcamento.lrxml" : "venda.lrxml")) {
    QMessageBox::critical(nullptr, "Erro!", "Não encontrou o modelo de impressão!");
    return;
  }

  report->dataManager()->setReportVariable("Loja", queryLoja.value("descricao"));
  report->dataManager()->setReportVariable("EnderecoLoja", queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + "\n" +
                                                               queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " + queryLojaEnd.value("uf").toString() +
                                                               " - CEP: " + queryLojaEnd.value("cep").toString() + "\n" + queryLoja.value("tel").toString() + " - " +
                                                               queryLoja.value("tel2").toString());

  report->dataManager()->setReportVariable("Data", query.value("data").toDate().toString("dd-MM-yyyy"));
  report->dataManager()->setReportVariable("Cliente", queryCliente.value("nome_razao"));
  QString cpfcnpj = queryCliente.value("pfpj") == "PF" ? "CPF: " : "CNPJ: ";
  report->dataManager()->setReportVariable("CPFCNPJ", cpfcnpj + queryCliente.value(queryCliente.value("pfpj") == "PF" ? "cpf" : "cnpj").toString());
  report->dataManager()->setReportVariable("EmailCliente", queryCliente.value("email"));
  report->dataManager()->setReportVariable("Tel1", queryCliente.value("tel"));
  report->dataManager()->setReportVariable("Tel2", queryCliente.value("telCel"));
  report->dataManager()->setReportVariable("CEPFiscal", queryEndFat.value("cep"));
  report->dataManager()->setReportVariable("EndFiscal", query.value("idEnderecoFaturamento").toInt() == 1
                                                            ? "Não há/Retira"
                                                            : queryEndFat.value("logradouro").toString() + " - " + queryEndFat.value("numero").toString() + " - " +
                                                                  queryEndFat.value("complemento").toString() + " - " + queryEndFat.value("bairro").toString() + " - " +
                                                                  queryEndFat.value("cidade").toString() + " - " + queryEndFat.value("uf").toString());
  report->dataManager()->setReportVariable("CEPEntrega", queryEndEnt.value("cep"));
  report->dataManager()->setReportVariable("EndEntrega", query.value("idEnderecoEntrega").toInt() == 1
                                                             ? "Não há/Retira"
                                                             : queryEndEnt.value("logradouro").toString() + " - " + queryEndEnt.value("numero").toString() + " - " +
                                                                   queryEndEnt.value("complemento").toString() + " - " + queryEndEnt.value("bairro").toString() + " - " +
                                                                   queryEndEnt.value("cidade").toString() + " - " + queryEndEnt.value("uf").toString());
  report->dataManager()->setReportVariable("Profissional", queryProfissional.value("nome_razao").toString().isEmpty() ? "Não há" : queryProfissional.value("nome_razao"));
  report->dataManager()->setReportVariable("EmailProfissional", queryProfissional.value("email"));
  report->dataManager()->setReportVariable("Vendedor", queryVendedor.value("nome"));
  report->dataManager()->setReportVariable("EmailVendedor", queryVendedor.value("email"));

  QLocale locale;

  report->dataManager()->setReportVariable("Soma", locale.toString(query.value("subTotalLiq").toDouble(), 'f', 2));
  report->dataManager()->setReportVariable("Desconto",
                                           "R$ " + locale.toString(query.value("descontoReais").toDouble(), 'f', 2) + " (" + locale.toString(query.value("descontoPorc").toDouble(), 'f', 2) + "%)");
  double value = query.value("total").toDouble() - query.value("frete").toDouble();
  report->dataManager()->setReportVariable("Total", locale.toString(value, 'f', 2));
  report->dataManager()->setReportVariable("Frete", locale.toString(query.value("frete").toDouble(), 'f', 2));
  report->dataManager()->setReportVariable("TotalFinal", locale.toString(query.value("total").toDouble(), 'f', 2));
  report->dataManager()->setReportVariable("Observacao", query.value("observacao").toString().replace("\n", " "));

  if (type == Orcamento) {
    report->dataManager()->setReportVariable("Orcamento", id);
    report->dataManager()->setReportVariable("Validade", query.value("validade").toString() + " dias");
  }

  if (type == Venda) {
    report->dataManager()->setReportVariable("Pedido", id);
    report->dataManager()->setReportVariable("Orcamento", query.value("idOrcamento"));

    QLocale locale;

    report->dataManager()->setReportVariable("PrazoEntrega", query.value("prazoEntrega").toString() + " dias");

    QSqlQuery queryPgt1("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                        "' AND tipo LIKE '1%' AND tipo != '1. Comissão' AND tipo != '1. Taxa Cartão' AND status != 'CANCELADO'");

    if (not queryPgt1.exec() or not queryPgt1.first()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
      return;
    }

    const QString pgt1 = queryPgt1.value("tipo").toString() + " - " + queryPgt1.value("COUNT(valor)").toString() + "x de R$ " + locale.toString(queryPgt1.value("valor").toDouble(), 'f', 2) +
                         (queryPgt1.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") + queryPgt1.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " +
                         queryPgt1.value("observacao").toString();

    report->dataManager()->setReportVariable("FormaPagamento1", pgt1);

    QSqlQuery queryPgt2("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                        "' AND tipo LIKE '2%' AND tipo != '2. Comissão' AND tipo != '2. Taxa Cartão' AND status != 'CANCELADO'");

    if (not queryPgt2.exec() or not queryPgt2.first()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
      return;
    }

    const QString pgt2 = queryPgt2.value("valor") == 0
                             ? ""
                             : queryPgt2.value("tipo").toString() + " - " + queryPgt2.value("COUNT(valor)").toString() + "x de R$ " + locale.toString(queryPgt2.value("valor").toDouble(), 'f', 2) +
                                   (queryPgt2.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") + queryPgt2.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " +
                                   queryPgt2.value("observacao").toString();

    report->dataManager()->setReportVariable("FormaPagamento2", pgt2);

    QSqlQuery queryPgt3("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                        "' AND tipo LIKE '3%' AND tipo != '3. Comissão' AND tipo != '3. Taxa Cartão' AND status != 'CANCELADO'");

    if (not queryPgt3.exec() or not queryPgt3.first()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
      return;
    }

    const QString pgt3 = queryPgt3.value("valor") == 0
                             ? ""
                             : queryPgt3.value("tipo").toString() + " - " + queryPgt3.value("COUNT(valor)").toString() + "x de R$ " + locale.toString(queryPgt3.value("valor").toDouble(), 'f', 2) +
                                   (queryPgt3.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") + queryPgt3.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " +
                                   queryPgt3.value("observacao").toString();

    report->dataManager()->setReportVariable("FormaPagamento3", pgt3);

    QSqlQuery queryPgt4("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                        "' AND tipo LIKE '4%' AND tipo != '4. Comissão' AND tipo != '4. Taxa Cartão' AND status != 'CANCELADO'");

    if (not queryPgt4.exec() or not queryPgt4.first()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro buscando pagamentos 4: " + queryPgt4.lastError().text());
      return;
    }

    const QString pgt4 = queryPgt4.value("valor") == 0
                             ? ""
                             : queryPgt4.value("tipo").toString() + " - " + queryPgt4.value("COUNT(valor)").toString() + "x de R$ " + locale.toString(queryPgt4.value("valor").toDouble(), 'f', 2) +
                                   (queryPgt4.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") + queryPgt4.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " +
                                   queryPgt4.value("observacao").toString();

    report->dataManager()->setReportVariable("FormaPagamento4", pgt4);

    QSqlQuery queryPgt5("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                        "' AND tipo LIKE '5%' AND tipo != '5. Comissão' AND tipo != '5. Taxa Cartão' AND status != 'CANCELADO'");

    if (not queryPgt5.exec() or not queryPgt5.first()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro buscando pagamentos 5: " + queryPgt5.lastError().text());
      return;
    }

    const QString pgt5 = queryPgt5.value("valor") == 0
                             ? ""
                             : queryPgt5.value("tipo").toString() + " - " + queryPgt5.value("COUNT(valor)").toString() + "x de R$ " + locale.toString(queryPgt5.value("valor").toDouble(), 'f', 2) +
                                   (queryPgt5.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") + queryPgt5.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " +
                                   queryPgt5.value("observacao").toString();

    report->dataManager()->setReportVariable("FormaPagamento5", pgt5);
  }

  const QString path = UserSession::settings(folder).toString();

  QDir dir(path);

  if (not dir.exists() and not dir.mkdir(path)) {
    QMessageBox::critical(nullptr, "Erro!", "Erro ao criar a pasta escolhida nas configurações!");
    return;
  }

  const QString fileName = path + "/" + id + "-" + queryVendedor.value("nome").toString().split(" ").first() + "-" + queryCliente.value("nome_razao").toString().replace("/", "-") + ".pdf";

  QFile file(fileName);

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(nullptr, "Erro!", "Não foi possível abrir o arquivo para escrita: " + fileName);
    QMessageBox::critical(nullptr, "Erro!", "Erro: " + file.errorString());
    return;
  }

  file.close();

  report->printToPDF(fileName);
  QMessageBox::information(nullptr, "Ok!", "Arquivo salvo como " + fileName);
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

bool Impressao::setQuerys() {
  if (type == Orcamento) {
    query.prepare("SELECT idCliente, idProfissional, idUsuario, idLoja, data, validade, idEnderecoFaturamento, "
                  "idEnderecoEntrega, subTotalLiq, descontoPorc, descontoReais, frete, total, observacao, prazoEntrega "
                  "FROM orcamento WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", id);
  }

  if (type == Venda) {
    query.prepare("SELECT idCliente, idProfissional, idUsuario, idLoja, idOrcamento, data, idEnderecoFaturamento, "
                  "idEnderecoEntrega, subTotalLiq, descontoPorc, descontoReais, frete, total, observacao, prazoEntrega "
                  "FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", id);
  }

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando dados da venda/orçamento: " + query.lastError().text());
    return false;
  }

  queryCliente.prepare("SELECT nome_razao, pfpj, cpf, cnpj, email, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", query.value("idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return false;
  }

  queryEndEnt.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM cliente_has_endereco "
                      "WHERE idEndereco = :idEndereco");
  queryEndEnt.bindValue(":idEndereco", query.value("idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando endereço: " + queryEndEnt.lastError().text());
    return false;
  }

  queryEndFat.prepare("SELECT logradouro, numero, complemento, bairro, cidade, uf, cep FROM cliente_has_endereco "
                      "WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", query.value(type == Venda ? "idEnderecoFaturamento" : "idEnderecoEntrega"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando dados do endereço: " + queryEndFat.lastError().text());
    return false;
  }

  queryProfissional.prepare("SELECT nome_razao, tel, email FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", query.value("idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return false;
  }

  queryVendedor.prepare("SELECT nome, email FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", query.value("idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return false;
  }

  queryLoja.prepare("SELECT descricao, tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", query.value("idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return false;
  }

  queryLojaEnd.prepare("SELECT logradouro, numero, bairro, cidade, uf, cep FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", query.value("idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando loja endereço: " + queryLojaEnd.lastError().text());
    return false;
  }

  return true;
}
