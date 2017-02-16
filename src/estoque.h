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
  bool criarConsumo(const int idVendaProduto, double quant = 0);
  bool viewRegisterById(const QString &idEstoque, bool showWindow = true);

private slots:
  void on_pushButtonExibirNfe_clicked();
  void on_tableConsumo_entered(const QModelIndex &);
  void on_tableEstoque_activated(const QModelIndex &);
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  SqlTableModel modelConsumo;
  SqlTableModel modelViewConsumo;
  Ui::Estoque *ui;

  enum FieldColors {
    White = 0,     // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
    Cyan = 5       // Devolução
  };

  // methods
  void calcularRestante();
  void exibirNota();
  void setupTables();
};

#endif // ESTOQUE_H
