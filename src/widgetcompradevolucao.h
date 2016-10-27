#ifndef WIDGETCOMPRADEVOLUCAO_H
#define WIDGETCOMPRADEVOLUCAO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraDevolucao;
}

class WidgetCompraDevolucao : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraDevolucao(QWidget *parent = 0);
  ~WidgetCompraDevolucao();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_pushButtonDevolucaoFornecedor_clicked();
  void on_pushButtonRetornarEstoque_clicked();
  void on_radioButtonFiltroPendente_toggled(bool checked);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  Ui::WidgetCompraDevolucao *ui;
  // methods
  void setupTables();
};

#endif // WIDGETCOMPRADEVOLUCAO_H
