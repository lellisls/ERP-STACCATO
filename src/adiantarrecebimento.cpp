#include "adiantarrecebimento.h"
#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "ui_adiantarrecebimento.h"

AdiantarRecebimento::AdiantarRecebimento(QWidget *parent) : QDialog(parent), ui(new Ui::AdiantarRecebimento) {
  ui->setupUi(this);

  setupTables();

  connect(ui->table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &AdiantarRecebimento::calcularTotais);
}

AdiantarRecebimento::~AdiantarRecebimento() { delete ui; }

void AdiantarRecebimento::calcularTotais() {
  auto list = ui->table->selectionModel()->selectedRows();

  double total = 0;

  for (auto const &item : list) total += model.data(item.row(), "valor").toDouble();
}

void AdiantarRecebimento::setupTables() {
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

  ui->table->setModel(&model);
  ui->table->hideColumn("idPagamento");
  ui->table->hideColumn("idLoja");
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
}

void AdiantarRecebimento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: usar essa tela para recebimentos normais de cartão além dos antecipamentos

// prazo medio = somatorio de cada linha usando -> ((data do pag - data do evento) * valor[valor lanc. - mdr]) / valor
// liquido
// valor bruto = soma das linhas que nao sao mdr
// valor liquido = soma inclusive mdr
// valor presente = valor liquido * (1 - taxa desc total)
// taxa desc total = taxa desc mes / 30 * prazo medio
