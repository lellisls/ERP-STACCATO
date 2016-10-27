#include <QMessageBox>
#include <QSqlError>

#include "calendarioentregas.h"
#include "doubledelegate.h"
#include "entregascliente.h"
#include "ui_calendarioentregas.h"

CalendarioEntregas::CalendarioEntregas(QWidget *parent) : QWidget(parent), ui(new Ui::CalendarioEntregas) {
  ui->setupUi(this);
}

CalendarioEntregas::~CalendarioEntregas() { delete ui; }

void CalendarioEntregas::on_table_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(model.data(index.row(), "idVenda").toString());
}

bool CalendarioEntregas::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela de entregas: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void CalendarioEntregas::setupTables() {
  model.setTable("view_entrega");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Cx.");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("local", "Local");
  model.setHeaderData("dataPrevEnt", "Agendado");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela entregas: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  //  ui->table->hideColumn("idEstoque");
}
