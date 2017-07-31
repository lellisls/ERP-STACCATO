#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "anteciparrecebimento.h"
#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "reaisdelegate.h"
#include "ui_anteciparrecebimento.h"

AnteciparRecebimento::AnteciparRecebimento(QWidget *parent) : QDialog(parent), ui(new Ui::AnteciparRecebimento) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  QSqlQuery query;
  query.exec("SELECT DISTINCT(SUBSTRING(tipo FROM 4)) AS tipo FROM view_conta_receber");

  ui->comboBox->addItem("");

  while (query.next()) ui->comboBox->addItem(query.value("tipo").toString());

  ui->dateEditEvento->setDate(QDate::currentDate());

  connect(ui->doubleSpinBoxDescMes, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &AnteciparRecebimento::calcularTotais);
  connect(ui->dateEditEvento, &QDateEdit::dateChanged, this, &AnteciparRecebimento::calcularTotais);
  connect(ui->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AnteciparRecebimento::calcularTotais);
}

AnteciparRecebimento::~AnteciparRecebimento() { delete ui; }

void AnteciparRecebimento::calcularTotais() {
  // TODO: 1ao selecionar parcelas de cartao somar a taxa tambem
  if (isBlockedPresente) return;

  const auto list = ui->table->selectionModel()->selectedRows();

  double bruto = 0;
  double liquido = 0;
  int prazoMedio = 0;

  for (auto const &item : list) {
    const QString tipo = model.data(item.row(), "tipo").toString();
    const double valor = model.data(item.row(), "valor").toDouble();
    const QDate dataPagamento = model.data(item.row(), "dataPagamento").toDate();

    if (not tipo.contains("Taxa Cartão")) bruto += valor;

    liquido += model.data(item.row(), "valor").toDouble();

    // valor aqui deve considerar a taxa cartao
    const double prazo = ui->dateEditEvento->date().daysTo(dataPagamento) * valor;

    prazoMedio += prazo;
  }

  // be careful about division by 0
  prazoMedio /= liquido;

  ui->doubleSpinBoxDescTotal->setValue(ui->doubleSpinBoxDescMes->value() / 30 * prazoMedio);
  ui->spinBoxPrazoMedio->setValue(prazoMedio);
  ui->doubleSpinBoxValorBruto->setValue(bruto);
  ui->doubleSpinBoxValorLiquido->setValue(liquido);

  isBlockedMes = true;
  ui->doubleSpinBoxValorPresente->setValue(liquido * (1 - ui->doubleSpinBoxDescTotal->value() / 100));
  isBlockedMes = false;
}

void AnteciparRecebimento::setupTables() {
  model.setTable("view_conta_receber");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("dataEmissao", "Data Emissão");
  model.setHeaderData("idVenda", "Código");
  model.setHeaderData("valor", "R$");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("parcela", "Parcela");
  model.setHeaderData("dataPagamento", "Data Pag.");
  model.setHeaderData("observacao", "Obs.");
  model.setHeaderData("status", "Status");
  model.setHeaderData("representacao", "Representação");
  model.setHeaderData("dataRealizado", "Data Realizado");
  model.setHeaderData("valorReal", "Valor Real");
  model.setHeaderData("tipoReal", "Tipo Real");
  model.setHeaderData("parcelaReal", "Parcela Real");
  model.setHeaderData("contaDestino", "Conta Destino");
  model.setHeaderData("tipoDet", "Tipo Det");
  model.setHeaderData("centroCusto", "Centro Custo");
  model.setHeaderData("grupo", "Grupo");
  model.setHeaderData("subGrupo", "SubGrupo");
  model.setHeaderData("contraParte", "Contraparte");
  model.setHeaderData("statusFinanceiro", "Financeiro");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return;
  }

  ui->table->setModel(&model);
  ui->table->hideColumn("representacao");
  ui->table->hideColumn("idPagamento");
  ui->table->hideColumn("idLoja");
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(this));
}

void AnteciparRecebimento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void AnteciparRecebimento::on_comboBox_currentTextChanged(const QString &text) {
  model.setFilter("tipo LIKE '%" + text + "%' AND status = 'PENDENTE' AND dataPagamento > NOW()");

  if (text == "Cartão de crédito") {
    model.setFilter("(tipo LIKE '%Cartão de crédito%' OR tipo LIKE '%Taxa Cartão%') AND status = 'PENDENTE' AND "
                    "dataPagamento > NOW()");
  }

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return;
  }
}

void AnteciparRecebimento::on_doubleSpinBoxValorPresente_valueChanged(double) {
  if (isBlockedMes) return;

  const double presente = ui->doubleSpinBoxValorPresente->value();
  const double liquido = ui->doubleSpinBoxValorLiquido->value();
  const double descTotal = 1 - (presente / liquido);
  const int prazoMedio = ui->spinBoxPrazoMedio->value();

  ui->doubleSpinBoxDescTotal->setValue(descTotal * 100);

  isBlockedPresente = true;
  const double valor = descTotal / prazoMedio * 30 * 100;
  ui->doubleSpinBoxDescMes->setValue(valor != valor ? 0 : valor); // if nan set zero
  //qDebug() << "comparison: " << (valor != valor);
  //qDebug() << "isnan: " << qIsNaN(valor);
  isBlockedPresente = false;
}

// NOTE: usar essa tela para recebimentos normais de cartão além dos antecipamentos

// TODO: 0implementar antecipacao

// O Cálculo do prazo médio de vencimento das duplicatas é feito da seguinte forma:

// Com a soma dos valores das parcelas, obtém-se o primeiro valor(A).
// O segundo passo, é a totalização, da multiplicação do número de dias de cada parcela, por seu respectivo valor (B).
// Para obter o prazo médio, é só dividir os valores (B) por (A).

// Ex. 3 parcelas de R$100,00 -> vencimento em 10/20/30 dias

// A = 100,00 + 100,00 + 100,00.
// A = 300,00.

// B = (10 * 100,00) + (20 * 100,00) + (30 * 100,00).
// B = 6.000,00

// Cálculo do prazo médio = 6.000,00 / 300,00
// PRAZO MÉDIO = 20.

// datas / valor liquido

// prazo medio = somatorio de cada linha usando -> (dias parcela * valor) / valor liquido
// ((data do pag - data do evento) * valor[valor lanc. - mdr]) / valor liquido
// valor bruto = soma das linhas que nao sao mdr
// valor liquido = soma inclusive mdr
// taxa desc total = taxa desc mes / 30 * prazo medio
// valor presente = valor liquido * (1 - taxa desc total)

// MDR 2%
// Taxa Antec. 5%

// valor bruto = R$ 100
// valor liq. = R$ 98
// taxa desc = (5% / 30) * (quant. dias antecipados)
//     taxa desc = (5% / 30) * 60d
//     taxa desc = 5% * 2 = 10%
// valor presente = R$ 98 - (R$ 98 * 10%) = R$ 88,2

// TODO: 1para recebiveis diferentes de cartao calcular IOF

void AnteciparRecebimento::on_pushButtonGerar_clicked() {
  // TODO: 1para gerar a antecipacao: dar baixa nas linhas selecionadas (colocar RECEBIDO e marcar em qual data) e criar
  // uma unica linha dizendo quanto foi pago de juros

  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  // this is a view, edit the original table

  //  for (auto const &item : list) {
  //    model.setData(item.row(), "status", "RECEBIDO");
  //    model.setData(item.row(), "dataRealizado", ui->dateEditEvento->date());
  //    model.setData(item.row(), "observacao", "Antecipação");
  //  }

  const int newRow = model.rowCount();
  model.insertRow(newRow);

  model.setData(newRow, "status", "PAGO");
  model.setData(newRow, "valor", ui->doubleSpinBoxValorLiquido->value() - ui->doubleSpinBoxValorPresente->value());
  model.setData(newRow, "observacao", "Juros da antecipação");
  model.setData(newRow, "tipo", "JUROS ANTECIPAÇÃO");
}
