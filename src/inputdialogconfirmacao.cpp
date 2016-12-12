#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialogconfirmacao.h"
#include "orcamento.h"
#include "ui_inputdialogconfirmacao.h"

InputDialogConfirmacao::InputDialogConfirmacao(const Type &type, QWidget *parent)
    : QDialog(parent), type(type), ui(new Ui::InputDialogConfirmacao) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == Recebimento) {
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data do recebimento:");

    ui->labelEntregou->hide();
    ui->lineEditEntregou->hide();
  }

  if (type == Entrega) {
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data entrega:");

    ui->labelRecebeu->hide();
    ui->lineEditRecebeu->hide();
  }

  if (type == Representacao) {
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->tableLogistica->hide();

    ui->labelRecebeu->hide();
    ui->lineEditRecebeu->hide();

    ui->frameQuebrado->hide();
  }

  adjustSize();

  show();
}

InputDialogConfirmacao::~InputDialogConfirmacao() { delete ui; }

QDateTime InputDialogConfirmacao::getDate() const { return ui->dateEditEvento->dateTime(); }

QDateTime InputDialogConfirmacao::getNextDate() const { return ui->dateEditProximo->dateTime(); }

QString InputDialogConfirmacao::getRecebeu() const { return ui->lineEditRecebeu->text(); }

QString InputDialogConfirmacao::getEntregou() const { return ui->lineEditEntregou->text(); }

bool InputDialogConfirmacao::cadastrar() {
  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela: " + model.lastError().text());
    return false;
  }

  return true;
}

void InputDialogConfirmacao::on_pushButtonSalvar_clicked() {
  if (type == Recebimento and ui->lineEditRecebeu->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher quem recebeu!");
    return;
  }

  if (type == Entrega and ui->lineEditEntregou->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher quem entregou!");
    return;
  }

  if (type != Representacao and not cadastrar()) return;

  QDialog::accept();
  close();
}

void InputDialogConfirmacao::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialogConfirmacao::setupTables() {
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (type == Recebimento) {
    model.setTable("estoque");
    model.setHeaderData("status", "Status");
    model.setHeaderData("local", "Local");
    model.setHeaderData("fornecedor", "Fornecedor");
    model.setHeaderData("descricao", "Produto");
    model.setHeaderData("quant", "Quant.");
    model.setHeaderData("un", "Un.");
    model.setHeaderData("caixas", "Cx.");
    model.setHeaderData("codComercial", "Cód. Com.");
  }

  if (type == Entrega) {
    model.setTable("veiculo_has_produto");
    model.setHeaderData("idVenda", "Venda");
    model.setHeaderData("status", "Status");
    model.setHeaderData("fornecedor", "Fornecedor");
    model.setHeaderData("produto", "Produto");
    model.setHeaderData("obs", "Obs.");
    model.setHeaderData("caixas", "Cx.");
    model.setHeaderData("quant", "Quant.");
    model.setHeaderData("un", "Un.");
    model.setHeaderData("unCaixa", "Un./Cx.");
    model.setHeaderData("codComercial", "Cód. Com.");
    model.setHeaderData("formComercial", "Form. Com.");
  }

  if (type != Representacao) {
    if (not model.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
      return;
    }

    ui->tableLogistica->setModel(&model);
  }

  if (type == Recebimento) {
    ui->tableLogistica->hideColumn("idEstoque");
    ui->tableLogistica->hideColumn("recebidoPor");
    ui->tableLogistica->hideColumn("idProduto");
    ui->tableLogistica->hideColumn("quantUpd");
    ui->tableLogistica->hideColumn("codBarras");
    ui->tableLogistica->hideColumn("ncm");
    ui->tableLogistica->hideColumn("cfop");
    ui->tableLogistica->hideColumn("valor");
    ui->tableLogistica->hideColumn("valorUnid");
    ui->tableLogistica->hideColumn("codBarrasTrib");
    ui->tableLogistica->hideColumn("unTrib");
    ui->tableLogistica->hideColumn("quantTrib");
    ui->tableLogistica->hideColumn("valorTrib");
    ui->tableLogistica->hideColumn("desconto");
    ui->tableLogistica->hideColumn("compoeTotal");
    ui->tableLogistica->hideColumn("numeroPedido");
    ui->tableLogistica->hideColumn("itemPedido");
    ui->tableLogistica->hideColumn("tipoICMS");
    ui->tableLogistica->hideColumn("orig");
    ui->tableLogistica->hideColumn("cstICMS");
    ui->tableLogistica->hideColumn("modBC");
    ui->tableLogistica->hideColumn("vBC");
    ui->tableLogistica->hideColumn("pICMS");
    ui->tableLogistica->hideColumn("vICMS");
    ui->tableLogistica->hideColumn("modBCST");
    ui->tableLogistica->hideColumn("pMVAST");
    ui->tableLogistica->hideColumn("vBCST");
    ui->tableLogistica->hideColumn("pICMSST");
    ui->tableLogistica->hideColumn("vICMSST");
    ui->tableLogistica->hideColumn("cEnq");
    ui->tableLogistica->hideColumn("cstIPI");
    ui->tableLogistica->hideColumn("cstPIS");
    ui->tableLogistica->hideColumn("vBCPIS");
    ui->tableLogistica->hideColumn("pPIS");
    ui->tableLogistica->hideColumn("vPIS");
    ui->tableLogistica->hideColumn("cstCOFINS");
    ui->tableLogistica->hideColumn("vBCCOFINS");
    ui->tableLogistica->hideColumn("pCOFINS");
    ui->tableLogistica->hideColumn("vCOFINS");
  }

  if (type == Entrega) {
    ui->tableLogistica->hideColumn("id");
    ui->tableLogistica->hideColumn("data");
    ui->tableLogistica->hideColumn("idEvento");
    ui->tableLogistica->hideColumn("idVeiculo");
    ui->tableLogistica->hideColumn("idEstoque");
    ui->tableLogistica->hideColumn("idVendaProduto");
    ui->tableLogistica->hideColumn("idCompra");
    ui->tableLogistica->hideColumn("idNfeSaida");
    ui->tableLogistica->hideColumn("idLoja");
    ui->tableLogistica->hideColumn("idProduto");
  }
}

bool InputDialogConfirmacao::setFilter(const QString &id) { // entrega
  if (id.isEmpty()) {
    model.setFilter("0");
    QMessageBox::critical(this, "Erro!", "IdsCompra vazio!");
    return false;
  }

  const QString filter = "idVenda = '" + id + "'";

  if (filter.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Filtro vazio!");
    return false;
  }

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->tableLogistica->resizeColumnsToContents();

  QMessageBox::information(this, "Aviso!", "Se for o caso, marcar os itens com problema.");

  return true;
}

bool InputDialogConfirmacao::setFilter(const QStringList &ids) { // recebimento
  if (ids.isEmpty()) {
    model.setFilter("0");
    QMessageBox::critical(this, "Erro!", "IdsCompra vazio!");
    return false;
  }

  const QString filter = "idEstoque = " + ids.join(" OR idEstoque = ");

  if (filter.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Filtro vazio!");
    return false;
  }

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->tableLogistica->resizeColumnsToContents();

  QMessageBox::information(this, "Aviso!", "Se for o caso, marcar os itens com problema.");

  return true;
}

void InputDialogConfirmacao::on_pushButtonQuebradoFaltando_clicked() { // TODO: 2refactor this function
  const auto list = ui->tableLogistica->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  const int row = list.first().row();

  if (type == Recebimento) {
    const QString produto = model.data(row, "descricao").toString();
    const int idEstoque = model.data(row, "idEstoque").toInt();
    const double caixasEstoque = model.data(row, "caixas").toDouble(); // 4

    QInputDialog input;
    const int caixasDefeito = input.getInt(this, produto, "Caixas: ", 0, 0, caixasEstoque); // 1

    if (caixasDefeito == 0) return;

    QSqlQuery query;
    query.prepare("SELECT unCaixa FROM venda_has_produto WHERE idProduto = :idProduto");
    query.bindValue(":idProduto", model.data(row, "idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando consumo estoque: " + query.lastError().text());
      return;
    }

    const double unCaixa = query.value("unCaixa").toDouble();

    query.prepare("SELECT e.caixas - SUM(ehc.caixas) AS sobra FROM estoque_has_consumo ehc LEFT JOIN estoque e ON "
                  "ehc.idEstoque = e.idEstoque WHERE ehc.idEstoque = :idEstoque");
    query.bindValue(":idEstoque", idEstoque);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando sobra estoque: " + query.lastError().text());
      return;
    }

    const double sobra = query.first() ? query.value("sobra").toInt() : 0;
    qDebug() << "sobra: " << sobra;
    qDebug() << "caixasDefeito: " << caixasDefeito;

    query.prepare(
        "SELECT CAST((`v`.`data` + INTERVAL `v`.`prazoEntrega` DAY) AS DATE) AS `prazoEntrega`, ehc.* FROM "
        "estoque_has_consumo ehc LEFT JOIN venda_has_produto vp ON ehc.idVendaProduto = vp.idVendaProduto "
        "LEFT JOIN venda v ON vp.idVenda = v.idVenda WHERE ehc.idEstoque = :idEstoque ORDER BY prazoEntrega DESC");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":caixas", caixasDefeito);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando consumo estoque: " + query.lastError().text());
      return;
    }

    // estoque 10
    // consumo 1
    // consumo 4
    // consumo 3
    // sobra   2

    // defeito 3

    // cria consumo quebrado 3
    // abate consumo 1, resta 0 (faltam 2 quebrados)
    // abate consumo 4, resta 2 (falta 0 quebrados)

    SqlTableModel modelConsumo;
    modelConsumo.setTable("estoque_has_consumo");
    modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

    modelConsumo.setFilter("idEstoque = " + model.data(row, "idEstoque").toString());

    if (not modelConsumo.select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela consumo: " + modelConsumo.lastError().text());
      return;
    }

    // criar consumo 'quebrado'

    int rowConsumo = modelConsumo.rowCount();
    modelConsumo.insertRow(rowConsumo);

    for (int col = 0; col < modelConsumo.columnCount(); ++col) {
      // skip cols
      if (modelConsumo.fieldIndex("idConsumo") == col) continue;
      if (modelConsumo.fieldIndex("idVendaProduto") == col) continue;
      if (modelConsumo.fieldIndex("created") == col) continue;
      if (modelConsumo.fieldIndex("lastUpdated") == col) continue;

      if (not modelConsumo.setData(rowConsumo, col, modelConsumo.data(0, col))) {
        QMessageBox::critical(this, "Erro!", "Erro copiando no consumo: " + modelConsumo.lastError().text());
        return;
      }
    }

    if (not modelConsumo.setData(rowConsumo, "status", "QUEBRADO")) return;
    qDebug() << "quant quebrado: " << caixasDefeito * unCaixa * -1;
    qDebug() << "caixas quebrado: " << caixasDefeito;
    if (not modelConsumo.setData(rowConsumo, "quant", caixasDefeito * unCaixa * -1)) return;
    if (not modelConsumo.setData(rowConsumo, "caixas", caixasDefeito)) return;

    if (not modelConsumo.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando consumo: " + modelConsumo.lastError().text());
      return;
    }

    //

    if (caixasDefeito > sobra) {
      double caixasDefeitoRestante = caixasDefeito;

      while (query.next()) {
        // se algum consumo for maior que a quantidade quebrada, dividir em duas linhas

        const double caixasConsumo = query.value("caixas").toDouble(); // 2
        qDebug() << "caixasConsumo: " << caixasConsumo;
        qDebug() << "defeitoRestante: " << caixasDefeitoRestante;

        //        if (caixasDefeitoRestante > caixasConsumo) {
        // tratar
        // ajustar quantidade (quantOriginal - quantQuebrada)
        // ajustar quando (quantOriginal - quantQuebrada) < 0

        const double newQuant = caixasConsumo > caixasDefeitoRestante ? caixasConsumo - caixasDefeitoRestante : 0;

        QSqlQuery query2;
        query2.prepare("UPDATE estoque_has_consumo SET quant = :quant, caixas = :caixas WHERE idConsumo = :idConsumo");
        query2.bindValue(":quant", newQuant * unCaixa * -1);
        qDebug() << "quant: " << newQuant * unCaixa * -1;
        query2.bindValue(":caixas", newQuant);
        qDebug() << "caixas: " << newQuant;
        query2.bindValue(":idConsumo", query.value("idConsumo"));

        if (not query2.exec()) {
          QMessageBox::critical(this, "Erro!", "Erro atualizando consumo estoque: " + query2.lastError().text());
          return;
        }

        // quebrar linha venda_produto em 2

        SqlTableModel modelVenda;
        modelVenda.setTable("venda_has_produto");
        modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);

        modelVenda.setFilter("idVendaProduto = " + query.value("idVendaProduto").toString());

        if (not modelVenda.select()) {
          QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda produto: " + modelVenda.lastError().text());
          return;
        }

        const int newRow = modelVenda.rowCount();
        modelVenda.insertRow(newRow);

        // copiar linha com quantidade quebrada
        for (int col = 0; col < modelVenda.columnCount(); ++col) {
          if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
          if (modelVenda.fieldIndex("idCompra") == col) continue;
          if (modelVenda.fieldIndex("idNfeSaida") == col) continue;
          if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
          if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
          if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
          if (modelVenda.fieldIndex("dataPrevCompra") == col) continue;
          if (modelVenda.fieldIndex("dataRealCompra") == col) continue;
          if (modelVenda.fieldIndex("dataPrevConf") == col) continue;
          if (modelVenda.fieldIndex("dataRealConf") == col) continue;
          if (modelVenda.fieldIndex("dataPrevFat") == col) continue;
          if (modelVenda.fieldIndex("dataRealFat") == col) continue;
          if (modelVenda.fieldIndex("dataPrevColeta") == col) continue;
          if (modelVenda.fieldIndex("dataRealColeta") == col) continue;
          if (modelVenda.fieldIndex("dataPrevReceb") == col) continue;
          if (modelVenda.fieldIndex("dataRealReceb") == col) continue;
          if (modelVenda.fieldIndex("dataPrevEnt") == col) continue;
          if (modelVenda.fieldIndex("dataRealEnt") == col) continue;

          if (not modelVenda.setData(newRow, col, modelVenda.data(0, col))) return;
        }

        const double prcUnitario = modelVenda.data(0, "prcUnitario").toDouble();
        const double desconto = modelVenda.data(0, "desconto").toDouble() / 100.;
        const double descGlobal = modelVenda.data(0, "descGlobal").toDouble() / 100.;

        const double quantCopia = (caixasConsumo - newQuant) * unCaixa;
        const double parcialCopia = prcUnitario * quantCopia;
        const double parcialDescCopia = parcialCopia + (parcialCopia * desconto);
        const double totalCopia = parcialDescCopia + (parcialDescCopia * descGlobal);

        qDebug() << "quantRepo: " << quantCopia;
        qDebug() << "caixasRepo: " << caixasConsumo - newQuant;
        qDebug() << "parcialRepo: " << parcialCopia;
        qDebug() << "parcialDescRepo: " << parcialDescCopia;
        qDebug() << "totalRepo: " << totalCopia;
        if (not modelVenda.setData(newRow, "quant", quantCopia)) return;
        if (not modelVenda.setData(newRow, "caixas", caixasConsumo - newQuant)) return;
        if (not modelVenda.setData(newRow, "parcial", parcialCopia)) return;
        if (not modelVenda.setData(newRow, "parcialDesc", parcialDescCopia)) return;
        if (not modelVenda.setData(newRow, "total", totalCopia)) return;
        if (not modelVenda.setData(newRow, "status", "QUEBRA")) return;
        if (not modelVenda.setData(newRow, "obs", "REPOSIÇÂO")) return;

        // ajustar a linha original e recalcular valores

        const double quantOriginal = newQuant;
        const double parcial = prcUnitario * quantOriginal * unCaixa;
        const double parcialDesc = parcial + (parcial * desconto);
        const double total = parcialDesc + (parcialDesc * descGlobal);

        qDebug() << "quantOri: " << quantOriginal * unCaixa;
        qDebug() << "caixasOri: " << quantOriginal;
        qDebug() << "parcialOri: " << parcial;
        qDebug() << "parcialDescOri: " << parcialDesc;
        qDebug() << "totalOri: " << total;
        if (not modelVenda.setData(0, "quant", quantOriginal * unCaixa)) return;
        if (not modelVenda.setData(0, "caixas", quantOriginal)) return;
        if (not modelVenda.setData(0, "parcial", parcial)) return;
        if (not modelVenda.setData(0, "parcialDesc", parcialDesc)) return;
        if (not modelVenda.setData(0, "total", total)) return;

        if (not modelVenda.submitAll()) {
          QMessageBox::critical(this, "Erro!", "Erro atualizando venda_produto: " + modelVenda.lastError().text());
          return;
        }
        //        qDebug() << "submit: " << modelVenda.submitAll();
        //        qDebug() << "error: " << modelVenda.lastError();

        caixasDefeitoRestante -= caixasConsumo;
        qDebug() << "defeito - consumo: " << caixasDefeitoRestante;
        //        }
      }

      if (caixasDefeitoRestante > 0) {
        // criar orcamento reposicao
        // colocar qual o estoque na observacao do produto

        QMessageBox msgBox(QMessageBox::Question, "Atenção!",
                           "Restaram " + QString::number(caixasDefeitoRestante) +
                               " caixas quebradas que não foram vendidas. Deseja fazer um pedido de reposição?",
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setButtonText(QMessageBox::Yes, "Sim");
        msgBox.setButtonText(QMessageBox::No, "Não");

        if (msgBox.exec() == QMessageBox::Yes) {
          Orcamento *orcamento = new Orcamento(this);
          orcamento->exec();
        }
      }
    }

    // 10 peças, 5 consumo, 3 quebradas
    // verificar se quant quebrada < quant consumo manter consumo senao ajustar

    // 1. criar linha consumo 'QUEBRADO'
    // 2. na venda subtrair do produto a quant quebrada, recalcular valores
    // 3. copiar linha com quant quebrada e status 'pendente', obs 'reposicao' (para ir para tela de pendentes_compra)
    // -
    // recalcular valores
  }

  if (type == Entrega) {
    // perguntar quantidade com problema
    // pegar o idVendaProduto e dividir essa linha em duas, uma com a quantidade com problema (status PENDENTE) e a
    // outra com a quant. que sobrou (status ENTREGUE)

    const QString produto = model.data(row, "produto").toString();
    const double caixas = model.data(row, "caixas").toDouble();
    const double unCaixa = model.data(row, "unCaixa").toDouble();

    QInputDialog input;
    const int caixasDefeito = input.getInt(this, produto, "Caixas: ", 0, 0, caixas);

    if (caixasDefeito == 0) return;

    // diminuir quantidade da linha selecionada

    if (not model.setData(row, "caixas", caixas - caixasDefeito)) return;
    if (not model.setData(row, "quant", (caixas - caixasDefeito) * unCaixa)) return;

    // copiar linha com quantDefeito

    if (not model.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo veiculo: " + model.lastError().text());
    }

    // quebrar linha venda_produto em 2

    SqlTableModel modelVenda;
    modelVenda.setTable("venda_has_produto");
    modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);

    modelVenda.setFilter("idVendaProduto = " + model.data(row, "idVendaProduto").toString());

    if (not modelVenda.select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto venda: " + modelVenda.lastError().text());
      return;
    }

    const int newRow = modelVenda.rowCount();
    modelVenda.insertRow(newRow);

    // copiar linha com quantidade quebrada
    for (int col = 0; col < modelVenda.columnCount(); ++col) {
      if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
      if (modelVenda.fieldIndex("idCompra") == col) continue;
      if (modelVenda.fieldIndex("idNfeSaida") == col) continue;
      if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
      if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
      if (modelVenda.fieldIndex("idVendaProduto") == col) continue;
      if (modelVenda.fieldIndex("dataPrevCompra") == col) continue;
      if (modelVenda.fieldIndex("dataRealCompra") == col) continue;
      if (modelVenda.fieldIndex("dataPrevConf") == col) continue;
      if (modelVenda.fieldIndex("dataRealConf") == col) continue;
      if (modelVenda.fieldIndex("dataPrevFat") == col) continue;
      if (modelVenda.fieldIndex("dataRealFat") == col) continue;
      if (modelVenda.fieldIndex("dataPrevColeta") == col) continue;
      if (modelVenda.fieldIndex("dataRealColeta") == col) continue;
      if (modelVenda.fieldIndex("dataPrevReceb") == col) continue;
      if (modelVenda.fieldIndex("dataRealReceb") == col) continue;
      if (modelVenda.fieldIndex("dataPrevEnt") == col) continue;
      if (modelVenda.fieldIndex("dataRealEnt") == col) continue;

      if (not modelVenda.setData(newRow, col, modelVenda.data(0, col))) return;
    }

    const double prcUnitario = modelVenda.data(0, "prcUnitario").toDouble();
    const double desconto = modelVenda.data(0, "desconto").toDouble() / 100.;
    const double descGlobal = modelVenda.data(0, "descGlobal").toDouble() / 100.;

    const double quantCopia = caixasDefeito * unCaixa;
    const double parcialCopia = prcUnitario * quantCopia;
    const double parcialDescCopia = parcialCopia - (parcialCopia * desconto);
    const double totalCopia = parcialDescCopia - (parcialDescCopia * descGlobal);

    if (not modelVenda.setData(newRow, "quant", caixasDefeito * unCaixa)) return;
    if (not modelVenda.setData(newRow, "caixas", caixasDefeito)) return;
    if (not modelVenda.setData(newRow, "parcial", parcialCopia)) return;
    if (not modelVenda.setData(newRow, "parcialDesc", parcialDescCopia)) return;
    if (not modelVenda.setData(newRow, "total", totalCopia)) return;
    if (not modelVenda.setData(newRow, "status", "QUEBRA")) return;
    if (not modelVenda.setData(newRow, "obs", "REPOSIÇÂO")) return;

    // ajustar a linha original e recalcular valores

    const double quantOriginal = modelVenda.data(0, "caixas").toInt() - caixasDefeito;
    const double parcial = prcUnitario * quantOriginal * unCaixa;
    const double parcialDesc = parcial - (parcial * desconto);
    const double total = parcialDesc - (parcialDesc * descGlobal);

    qDebug() << "quantOri: " << quantOriginal * unCaixa;
    qDebug() << "caixasOri: " << quantOriginal;
    qDebug() << "parcialOri: " << parcial;
    qDebug() << "parcialDescOri: " << parcialDesc;
    qDebug() << "totalOri: " << total;
    if (not modelVenda.setData(0, "quant", quantOriginal * unCaixa)) return;
    if (not modelVenda.setData(0, "caixas", quantOriginal)) return;
    if (not modelVenda.setData(0, "parcial", parcial)) return;
    if (not modelVenda.setData(0, "parcialDesc", parcialDesc)) return;
    if (not modelVenda.setData(0, "total", total)) return;

    if (not modelVenda.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando produto venda: " + modelVenda.lastError().text());
      return;
    }
  }

  QMessageBox::information(this, "Aviso!", "Operação realizada com sucesso!");
}

// TODO: 2em vez de criar reposicao automatico perguntar para usuario se deseja criar reposicao, ou gerar credito etc
