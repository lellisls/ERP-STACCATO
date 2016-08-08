#include "calendarioentregas.h"
#include "doubledelegate.h"
#include "entregascliente.h"
#include "orcamentoproxymodel.h"
#include "ui_calendarioentregas.h"

CalendarioEntregas::CalendarioEntregas(QWidget *parent) : QWidget(parent), ui(new Ui::CalendarioEntregas) {
  ui->setupUi(this);

  model.setTable("view_venda");
  model.setFilter("statusEntrega = 'ESTOQUE'");

  model.select();

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("idUsuario");
  ui->table->hideColumn("Data");
  ui->table->hideColumn("Total");
  ui->table->hideColumn("statusEntrega");
}

CalendarioEntregas::~CalendarioEntregas() { delete ui; }

void CalendarioEntregas::on_table_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(model.data(index.row(), "Código").toString());
}
