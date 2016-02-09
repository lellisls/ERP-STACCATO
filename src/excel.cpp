#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>
#include <QSqlError>

#include "excel.h"
#include "usersession.h"
#include "xlsxdocument.h"

Excel::Excel(QString id, QWidget *parent) : QObject(parent), id(id), parent(parent) {
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

void Excel::verificaTipo() {
  QSqlQuery query;
  query.prepare("SELECT idOrcamento FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", id);

  if (not query.exec()) {
    QMessageBox::critical(parent, "Erro!", "Erro verificando se id é Orçamento!");
    return;
  }

  type = query.first() ? Orcamento : Venda;
}

void Excel::gerarExcel() {
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

  if (modelItem.rowCount() > 34) {
    QMessageBox::critical(parent, "Erro!", "Mais itens do que cabe no modelo!");
    return;
  }

  QString arquivoModelo = modelItem.rowCount() <= 17 ? "modelo.xlsx" : "modelo2.xlsx";
  bool maior = modelItem.rowCount() > 17 ? true : false;

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(parent, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  QFile file(path + "/" + id + ".xlsx");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(parent, "Erro!", "Arquivo bloqueado! Por favor feche o arquivo.");
    return;
  }

  file.close();

  setQuerys();

  QLocale locale;

  QXlsx::Document xlsx(arquivoModelo);

  QString endLoja = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() +
                    " - " + queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() +
                    " - " + queryLojaEnd.value("uf").toString() + " - CEP: " + queryLojaEnd.value("cep").toString() +
                    "\n" + queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();

  QString endEntrega = queryEndEnt.value("logradouro").toString().isEmpty()
                       ? "Não há/Retira"
                       : queryEndEnt.value("logradouro").toString() + " " + queryEndEnt.value("numero").toString() +
                         " - " + queryEndEnt.value("bairro").toString() + ", " +
                         queryEndEnt.value("cidade").toString();

  QString endFat = queryEndFat.value("logradouro").toString().isEmpty()
                   ? "Não há/Retira"
                   : queryEndFat.value("logradouro").toString() + " " + queryEndFat.value("numero").toString() +
                     " - " + queryEndFat.value("bairro").toString() + ", " +
                     queryEndFat.value("cidade").toString();

  xlsx.write("A5", endLoja);
  if (type == Venda) xlsx.write("C2", "Pedido:");
  xlsx.write("D2", id);
  xlsx.write("D3", queryCliente.value("nome_razao"));
  xlsx.write("D4", queryCliente.value("email"));
  xlsx.write("D5", endEntrega);
  xlsx.write("D6", endFat);
  xlsx.write("D7", queryProfissional.value("nome_razao"));
  xlsx.write("D8", queryVendedor.value("nome"));
  xlsx.write("F8", queryVendedor.value("email"));
  xlsx.write("M2", query.value("data").toDateTime().toString("dd/MM/yyyy hh:mm"));
  xlsx.write("M3", queryCliente.value(queryCliente.value("pfpj") == "PF" ? "cpf" : "cnpj"));
  xlsx.write("M4", queryCliente.value("tel"));
  xlsx.write("J4", queryCliente.value("telCel"));
  xlsx.write("M5", queryEndFat.value("cep"));
  xlsx.write("M6", queryEndEnt.value("cep"));
  xlsx.write("H7", queryProfissional.value("tel"));
  xlsx.write("K7", queryProfissional.value("email"));

  double subLiq = query.value("subTotalLiq").toDouble();
  double subBru = query.value("subTotalBru").toDouble();
  double desconto = query.value("descontoPorc").toDouble();
  xlsx.write(maior ? "N46" : "N29", subLiq > subBru ? "R$ " + locale.toString(subLiq, 'f', 2)
                                                    : "R$ " + locale.toString(subBru, 'f', 2) + " (R$ " +
                                                      locale.toString(subLiq, 'f', 2) + ")");          // soma
  xlsx.write(maior ? "N47" : "N30", locale.toString(desconto, 'f', 2) + "%");                              // desconto
  xlsx.write(maior ? "N48" : "N31", "R$ " + locale.toString(subLiq - (desconto / 100. * subLiq), 'f', 2)); // total
  xlsx.write(maior ? "N49" : "N32", "R$ " + locale.toString(query.value("frete").toDouble(), 'f', 2));     // frete
  xlsx.write(maior ? "N50" : "N33", "R$ " + locale.toString(query.value("total").toDouble(), 'f', 2)); // total final
  xlsx.write(maior ? "B46" : "B29", query.value("prazoEntrega").toString() + " dias");

  QSqlQuery queryPgt1(
        "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
        "' AND tipo LIKE '1%'");

  if (not queryPgt1.exec() or not queryPgt1.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
    return;
  }

  QString pgt1 = queryPgt1.value("tipo").toString() + " - " + queryPgt1.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt1.value("valor").toDouble(), 'f', 2) +
                 (queryPgt1.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt1.value("dataEmissao").toDate().toString("dd-MM-yyyy");

  if (queryPgt1.value("valor") == 0) pgt1 = "";

  QSqlQuery queryPgt2(
        "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
        "' AND tipo LIKE '2%'");

  if (not queryPgt2.exec() or not queryPgt2.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
    return;
  }

  QString pgt2 = queryPgt2.value("tipo").toString() + " - " + queryPgt2.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt2.value("valor").toDouble(), 'f', 2) +
                 (queryPgt2.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt2.value("dataEmissao").toDate().toString("dd-MM-yyyy");

  if (queryPgt2.value("valor") == 0) pgt2 = "";

  QSqlQuery queryPgt3(
        "SELECT tipo, COUNT(valor), valor, dataEmissao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
        "' AND tipo LIKE '3%'");

  if (not queryPgt3.exec() or not queryPgt3.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
    return;
  }

  QString pgt3 = queryPgt3.value("tipo").toString() + " - " + queryPgt3.value("COUNT(valor)").toString() + "x de R$ " +
                 locale.toString(queryPgt3.value("valor").toDouble(), 'f', 2) +
                 (queryPgt3.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                 queryPgt3.value("dataEmissao").toDate().toString("dd-MM-yyyy");

  if (queryPgt3.value("valor") == 0) pgt3 = "";

  xlsx.write(maior ? "B47" : "B30", pgt1);
  xlsx.write(maior ? "B48" : "B31", pgt2);
  xlsx.write(maior ? "B49" : "B32", pgt3);

  xlsx.write(maior ? "B50" : "B33", query.value("observacao").toString().replace("\n", " "));

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    xlsx.write("A" + QString::number(12 + row), modelItem.data(row, "fornecedor"));
    xlsx.write("B" + QString::number(12 + row), modelItem.data(row, "codComercial"));
    QString formComercial = modelItem.data(row, "formComercial").toString();
    xlsx.write("C" + QString::number(12 + row),
               modelItem.data(row, "produto").toString() + (formComercial.isEmpty() ? "" : " (" + formComercial + ")"));
    xlsx.write("H" + QString::number(12 + row), modelItem.data(row, "obs"));

    double prcUn = modelItem.data(row, "prcUnitario").toDouble();
    double desc = prcUn * modelItem.data(row, "desconto").toDouble() / 100.;
    double porc = modelItem.data(row, "desconto").toDouble();
    QString preco = "R$ " + locale.toString(prcUn, 'f', 2);
    QString precoDesc =
        desc > 0.01 ? " (-" + locale.toString(porc, 'f', 2) + "% R$ " + locale.toString(prcUn - desc, 'f', 2) + ")"
                    : "";
    QString precoDescNeg = "R$ " + locale.toString((porc / -100. + 1) * prcUn, 'f', 2);
    xlsx.write("K" + QString::number(12 + row), porc < 0 ? precoDescNeg : preco + precoDesc);
    xlsx.write("L" + QString::number(12 + row), modelItem.data(row, "quant"));
    xlsx.write("M" + QString::number(12 + row), modelItem.data(row, "un"));
    QString total = "R$ " + locale.toString(modelItem.data(row, "parcial").toDouble(), 'f', 2);
    QString totalDesc =
        desc > 0.01 ? " (R$ " + locale.toString(modelItem.data(row, "parcialDesc").toDouble(), 'f', 2) + ")" : "";
    QString totalDescNeg = "R$ " + locale.toString(modelItem.data(row, "parcialDesc").toDouble(), 'f', 2);
    xlsx.write("N" + QString::number(12 + row), porc < 0 ? totalDescNeg : total + totalDesc);

    if (desc > 0.01) xlsx.setColumnWidth(11, 28);
  }

  if (not xlsx.saveAs(path + "/" + id + ".xlsx")) {
    QMessageBox::critical(parent, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return;
  }

  QMessageBox::information(parent, "Ok!", "Arquivo salvo como " + path + "/" + id + ".xlsx");
  QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + id + ".xlsx"));
}

void Excel::setQuerys() {
  if (type == Orcamento) {
    query.prepare("SELECT data, subTotalLiq, subTotalBru, descontoPorc, frete, total, prazoEntrega, observacao FROM "
                  "orcamento WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", model.data(0, "idOrcamento"));
  }

  if (type == Venda) {
    query.prepare("SELECT data, subTotalLiq, subTotalBru, descontoPorc, frete, total, prazoEntrega, observacao FROM "
                  "venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", model.data(0, "idVenda"));
  }

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados da venda: " + query.lastError().text());
    return;
  }

  queryCliente.prepare(
        "SELECT nome_razao, email, cpf, cnpj, pfpj, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", model.data(0, "idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return;
  }

  queryEndEnt.prepare(
        "SELECT logradouro, numero, bairro, cidade, cep FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndEnt.bindValue(":idEndereco", model.data(0, "idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(parent, "Erro!",
                          "Erro buscando dados do endereço entrega: " + queryEndEnt.lastError().text());
    return;
  }

  queryEndFat.prepare(
        "SELECT logradouro, numero, bairro, cidade, cep FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", model.data(0, "idEnderecoFaturamento"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados do endereço: " + queryEndFat.lastError().text());
    return;
  }

  queryProfissional.prepare("SELECT nome_razao, tel, email FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", model.data(0, "idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return;
  }

  queryVendedor.prepare("SELECT nome, email FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", model.data(0, "idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return;
  }

  queryLoja.prepare("SELECT tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", model.data(0, "idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return;
  }

  queryLojaEnd.prepare(
        "SELECT logradouro, numero, bairro, cidade, cep, uf FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", model.data(0, "idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando endereço loja: " + queryLojaEnd.lastError().text());
    return;
  }
}
