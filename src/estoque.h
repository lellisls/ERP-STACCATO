#ifndef ESTOQUE_H
#define ESTOQUE_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class Estoque;
}

class Estoque : public QDialog {
  Q_OBJECT

public:
  explicit Estoque(QWidget *parent = 0);
  ~Estoque();
  bool criarConsumo(const int &idVendaProduto);
  void viewRegisterById(const QString &idEstoque);

private slots:
  void on_pushButtonExibirNfe_clicked();
  void on_tableConsumo_entered(const QModelIndex &);
  void on_tableEstoque_activated(const QModelIndex &);
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  SqlTableModel modelConsumo;
  Ui::Estoque *ui;
  // methods
  void calcularRestante();
  void exibirNota();
  void setupTables();
};

#endif // ESTOQUE_H
