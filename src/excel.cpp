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

Excel::Excel(const QString &id, QWidget *parent) : id(id), parent(parent) { verificaTipo(); }

void Excel::verificaTipo() {
  QSqlQuery query;
  query.prepare("SELECT idOrcamento FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", id);

  if (not query.exec()) {
    QMessageBox::critical(parent, "Erro!", "Erro verificando se id é Orçamento!");
    return;
  }

  tipo = query.first() ? Tipo::Orcamento : Tipo::Venda;
}

bool Excel::gerarExcel(const int oc, const bool isRepresentacao, const QString &representacao) {
  const QString folder = tipo == Tipo::Orcamento ? "User/OrcamentosFolder" : "User/VendasFolder";

  if (UserSession::settings(folder).toString().isEmpty()) {
    QMessageBox::critical(parent, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    UserSession::setSettings(folder, QFileDialog::getExistingDirectory(parent, "Pasta PDF/Excel"));

    if (UserSession::settings(folder).toString().isEmpty()) return false;
  }

  const QString path = UserSession::settings(folder).toString();

  QDir dir(path);

  if (not dir.exists() and not dir.mkdir(path)) {
    QMessageBox::critical(parent, "Erro!", "Erro ao criar a pasta escolhida nas configurações!");
    return false;
  }

  const QString arquivoModelo = "modelo pedido.xlsx";

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(parent, "Erro!", "Não encontrou o modelo do Excel!");
    return false;
  }

  if (not setQuerys()) {
    QMessageBox::critical(parent, "Erro!", "Processo interrompido, ocorreu algum erro!");
    return false;
  }

  fileName = isRepresentacao ? path + "/" + representacao + ".xlsx"
                             : path + "/" + id + "-" + queryVendedor.value("nome").toString().split(" ").first() + "-" + queryCliente.value("nome_razao").toString().replace("/", "-") + ".xlsx";

  QFile file(fileName);

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(parent, "Erro!", "Não foi possível abrir o arquivo para escrita:\n" + fileName);
    QMessageBox::critical(parent, "Erro!", "Erro: " + file.errorString());
    return false;
  }

  file.close();

  QLocale locale;

  QXlsx::Document xlsx(arquivoModelo);

  const QString endLoja = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + " " + queryLojaEnd.value("complemento").toString() + " - " +
                          queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " + queryLojaEnd.value("uf").toString() + " - CEP: " +
                          queryLojaEnd.value("cep").toString() + "\n" + queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();

  const QString endEntrega = queryEndEnt.value("logradouro").toString().isEmpty()
                                 ? "Não há/Retira"
                                 : queryEndEnt.value("logradouro").toString() + " " + queryEndEnt.value("numero").toString() + " " + queryEndEnt.value("complemento").toString() + " - " +
                                       queryEndEnt.value("bairro").toString() + ", " + queryEndEnt.value("cidade").toString();

  const QString endFat = queryEndFat.value("logradouro").toString().isEmpty()
                             ? "Não há/Retira"
                             : queryEndFat.value("logradouro").toString() + " " + queryEndFat.value("numero").toString() + " " + queryEndFat.value("complemento").toString() + " - " +
                                   queryEndFat.value("bairro").toString() + ", " + queryEndFat.value("cidade").toString();

  xlsx.write("A5", endLoja);

  if (tipo == Tipo::Venda) {
    if (isRepresentacao) {
      xlsx.write("C2", "Pedido:");
      xlsx.write("D2", "OC " + QString::number(oc) + " " + id);
      QXlsx::Format format;
      format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
      format.setFontBold(true);
      format.setTopBorderStyle(QXlsx::Format::BorderMedium);
      format.setBottomBorderStyle(QXlsx::Format::BorderThin);
      xlsx.mergeCells(QXlsx::CellRange("D2:K2"), format);
    } else {
      xlsx.write("C2", "Pedido:");
      xlsx.write("D2", id);
      xlsx.write("H2", "Orçamento:");
      xlsx.write("I2", query.value("idOrcamento").toString());
    }
  }

  if (tipo == Tipo::Orcamento) xlsx.write("D2", id);

  xlsx.write("D3", queryCliente.value("nome_razao"));
  xlsx.write("D4", queryCliente.value("email"));
  xlsx.write("D5", endFat);
  xlsx.write("D6", endEntrega);
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

  const double subLiq = query.value("subTotalLiq").toDouble();
  const double subBru = query.value("subTotalBru").toDouble();
  const double desconto = query.value("descontoPorc").toDouble();
  // TODO: no lugar de calcular valores, usar os do BD
  xlsx.write("N113", subLiq > subBru ? "R$ " + locale.toString(subLiq, 'f', 2) : "R$ " + locale.toString(subBru, 'f', 2) + " (R$ " + locale.toString(subLiq, 'f', 2) + ")"); // soma
  xlsx.write("N114", locale.toString(desconto, 'f', 2) + "%");                                                                                                               // desconto
  xlsx.write("N115", "R$ " + locale.toString(subLiq - (desconto / 100. * subLiq), 'f', 2));                                                                                  // total
  xlsx.write("N116", "R$ " + locale.toString(query.value("frete").toDouble(), 'f', 2));                                                                                      // frete
  xlsx.write("N117", "R$ " + locale.toString(query.value("total").toDouble(), 'f', 2));                                                                                      // total final
  xlsx.write("B113", query.value("prazoEntrega").toString() + " dias");

  QSqlQuery queryPgt1("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                      "' AND tipo LIKE '1%' AND tipo != '1. Comissão' AND tipo != '1. Taxa Cartão' AND status != 'CANCELADO'");

  if (not queryPgt1.exec() or not queryPgt1.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
    return false;
  }

  const QString pgt1 = queryPgt1.value("valor") == 0 ? ""
                                                     : queryPgt1.value("tipo").toString() + " - " + queryPgt1.value("COUNT(valor)").toString() + "x de R$ " +
                                                           locale.toString(queryPgt1.value("valor").toDouble(), 'f', 2) + (queryPgt1.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                                                           queryPgt1.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " + queryPgt1.value("observacao").toString();

  QSqlQuery queryPgt2("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                      "' AND tipo LIKE '2%' AND tipo != '2. Comissão' AND tipo != '2. Taxa Cartão' AND status != 'CANCELADO'");

  if (not queryPgt2.exec() or not queryPgt2.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
    return false;
  }

  const QString pgt2 = queryPgt2.value("valor") == 0 ? ""
                                                     : queryPgt2.value("tipo").toString() + " - " + queryPgt2.value("COUNT(valor)").toString() + "x de R$ " +
                                                           locale.toString(queryPgt2.value("valor").toDouble(), 'f', 2) + (queryPgt2.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                                                           queryPgt2.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " + queryPgt2.value("observacao").toString();

  QSqlQuery queryPgt3("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                      "' AND tipo LIKE '3%' AND tipo != '3. Comissão' AND tipo != '3. Taxa Cartão' AND status != 'CANCELADO'");

  if (not queryPgt3.exec() or not queryPgt3.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
    return false;
  }

  const QString pgt3 = queryPgt3.value("valor") == 0 ? ""
                                                     : queryPgt3.value("tipo").toString() + " - " + queryPgt3.value("COUNT(valor)").toString() + "x de R$ " +
                                                           locale.toString(queryPgt3.value("valor").toDouble(), 'f', 2) + (queryPgt3.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                                                           queryPgt3.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " + queryPgt3.value("observacao").toString();

  // 4

  QSqlQuery queryPgt4("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                      "' AND tipo LIKE '4%' AND tipo != '4. Comissão' AND tipo != '4. Taxa Cartão' AND status != 'CANCELADO'");

  if (not queryPgt4.exec() or not queryPgt4.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 4: " + queryPgt4.lastError().text());
    return false;
  }

  const QString pgt4 = queryPgt4.value("valor") == 0 ? ""
                                                     : queryPgt4.value("tipo").toString() + " - " + queryPgt4.value("COUNT(valor)").toString() + "x de R$ " +
                                                           locale.toString(queryPgt4.value("valor").toDouble(), 'f', 2) + (queryPgt4.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                                                           queryPgt4.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " + queryPgt4.value("observacao").toString();

  // 5

  QSqlQuery queryPgt5("SELECT tipo, COUNT(valor), valor, dataPagamento, observacao FROM conta_a_receber_has_pagamento WHERE idVenda = '" + id +
                      "' AND tipo LIKE '5%' AND tipo != '5. Comissão' AND tipo != '5. Taxa Cartão' AND status != 'CANCELADO'");

  if (not queryPgt5.exec() or not queryPgt5.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando pagamentos 5: " + queryPgt5.lastError().text());
    return false;
  }

  const QString pgt5 = queryPgt5.value("valor") == 0 ? ""
                                                     : queryPgt5.value("tipo").toString() + " - " + queryPgt5.value("COUNT(valor)").toString() + "x de R$ " +
                                                           locale.toString(queryPgt5.value("valor").toDouble(), 'f', 2) + (queryPgt5.value("COUNT(valor)") == 1 ? " - pag. em: " : " - 1° pag. em: ") +
                                                           queryPgt5.value("dataPagamento").toDate().toString("dd-MM-yyyy") + " - " + queryPgt5.value("observacao").toString();

  xlsx.write("B114", pgt1);
  xlsx.write("B115", pgt2);
  xlsx.write("B116", pgt3);
  xlsx.write("B117", pgt4);
  xlsx.write("B118", pgt5);

  xlsx.write("B119", query.value("observacao").toString().replace("\n", " "));

  // TODO: refator this to start at 12
  int row = 0;
  queryProduto.first();

  do {
    QSqlQuery query;
    query.prepare("SELECT ui FROM produto WHERE idProduto = :idProduto");
    query.bindValue(":idProduto", queryProduto.value("idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(parent, "Erro!", "Erro buscando dados do produto: " + query.lastError().text());
      return false;
    }

    const QString loes = query.value("ui").toString().contains("- L") ? " LOES" : "";

    xlsx.write("A" + QString::number(12 + row), queryProduto.value("fornecedor").toString() + loes);
    xlsx.write("B" + QString::number(12 + row), queryProduto.value("codComercial").toString());
    const QString formComercial = queryProduto.value("formComercial").toString();
    xlsx.write("C" + QString::number(12 + row), queryProduto.value("produto").toString() + (formComercial.isEmpty() ? "" : " (" + formComercial + ")") + (loes.isEmpty() ? "" : " -" + loes));
    xlsx.write("H" + QString::number(12 + row), queryProduto.value("obs").toString());

    const double prcUn = queryProduto.value("prcUnitario").toDouble();
    const double desc = prcUn * queryProduto.value("desconto").toDouble() / 100.;
    const double porc = queryProduto.value("desconto").toDouble();

    const QString preco = "R$ " + locale.toString(prcUn, 'f', 2);
    const QString precoDesc = desc > 0.01 ? " (-" + locale.toString(porc, 'f', 2) + "% R$ " + locale.toString(prcUn - desc, 'f', 2) + ")" : "";
    const QString precoDescNeg = "R$ " + locale.toString((porc / -100. + 1) * prcUn, 'f', 2);
    xlsx.write("K" + QString::number(12 + row), porc < 0 ? precoDescNeg : preco + precoDesc);
    xlsx.write("L" + QString::number(12 + row), queryProduto.value("quant").toDouble());
    xlsx.write("M" + QString::number(12 + row), queryProduto.value("un").toString());
    const QString total = "R$ " + locale.toString(queryProduto.value("parcial").toDouble(), 'f', 2);
    const QString totalDesc = desc > 0.01 ? " (R$ " + locale.toString(queryProduto.value("parcialDesc").toDouble(), 'f', 2) + ")" : "";
    const QString totalDescNeg = "R$ " + locale.toString(queryProduto.value("parcialDesc").toDouble(), 'f', 2);
    xlsx.write("N" + QString::number(12 + row), porc < 0 ? totalDescNeg : total + totalDesc);

    if (desc > 0.01) xlsx.setColumnWidth(11, 28);

    ++row;
  } while (queryProduto.next());

  for (int row = queryProduto.size() + 12; row < 111; ++row) xlsx.setRowHeight(row, 0);

  if (not xlsx.saveAs(fileName)) {
    QMessageBox::critical(parent, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return false;
  }

  QMessageBox::information(parent, "Ok!", "Arquivo salvo como " + fileName);
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));

  return true;
}

QString Excel::getFileName() const { return fileName; }

bool Excel::setQuerys() {
  if (tipo == Tipo::Orcamento) {
    query.prepare("SELECT idLoja, idUsuario, idProfissional, idEnderecoEntrega, idCliente, data, subTotalLiq, subTotalBru, descontoPorc, frete, total, prazoEntrega, observacao FROM orcamento WHERE "
                  "idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", id);

    queryProduto.prepare("SELECT idProduto, fornecedor, codComercial, formComercial, produto, obs, prcUnitario, "
                         "desconto, quant, un, parcial, parcialDesc FROM orcamento_has_produto WHERE idOrcamento = :idOrcamento");
    queryProduto.bindValue(":idOrcamento", id);
  }

  if (tipo == Tipo::Venda) {
    query.prepare("SELECT idLoja, idUsuario, idProfissional, idEnderecoFaturamento, idEnderecoEntrega, idCliente, idOrcamento, data, subTotalLiq, subTotalBru, descontoPorc, frete, total, "
                  "prazoEntrega, observacao FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", id);

    queryProduto.prepare("SELECT idProduto, fornecedor, codComercial, formComercial, produto, obs, prcUnitario, "
                         "desconto, quant, un, parcial, parcialDesc FROM venda_has_produto WHERE idVenda = :idVenda");
    queryProduto.bindValue(":idVenda", id);
  }

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados da venda/orçamento: " + query.lastError().text());
    return false;
  }

  if (not queryProduto.exec() or not queryProduto.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados dos produtos: " + query.lastError().text());
    return false;
  }

  queryCliente.prepare("SELECT nome_razao, email, cpf, cnpj, pfpj, tel, telCel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", query.value("idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return false;
  }

  queryEndEnt.prepare("SELECT logradouro, numero, complemento, bairro, cidade, cep FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndEnt.bindValue(":idEndereco", query.value("idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados do endereço entrega: " + queryEndEnt.lastError().text());
    return false;
  }

  queryEndFat.prepare("SELECT logradouro, numero, complemento, bairro, cidade, cep FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", query.value(tipo == Tipo::Venda ? "idEnderecoFaturamento" : "idEnderecoEntrega"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando dados do endereço: " + queryEndFat.lastError().text());
    return false;
  }

  queryProfissional.prepare("SELECT nome_razao, tel, email FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", query.value("idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return false;
  }

  queryVendedor.prepare("SELECT nome, email FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", query.value("idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return false;
  }

  queryLoja.prepare("SELECT tel, tel2 FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", query.value("idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return false;
  }

  queryLojaEnd.prepare("SELECT logradouro, numero, complemento, bairro, cidade, cep, uf FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", query.value("idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(parent, "Erro!", "Erro buscando endereço loja: " + queryLojaEnd.lastError().text());
    return false;
  }

  return true;
}
