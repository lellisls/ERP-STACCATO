#include <QDebug>
#include <QMessageBox>

#include "ui_vendasmes.h"
#include "vendasmes.h"

VendasMes::VendasMes(const int usuario, QWidget *parent) : QWidget(parent), ui(new Ui::VendasMes) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(usuario);
}

VendasMes::~VendasMes() { delete ui; }

void VendasMes::setupTables(const int usuario) {
  model.setTable("view_vendas_mes");
  model.setFilter("idUsuario = " + QString::number(usuario));

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela!");

  ui->table->setModel(&model);
}

void VendasMes::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: excluir cancelados/devolucao do calculo
