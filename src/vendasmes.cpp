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
  ui->table->hideColumn("idUsuario");
}
