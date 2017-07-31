#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDataWidgetMapper>
#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog {
  Q_OBJECT

public:
  explicit SearchDialog(const QString &title, const QString &table, const QStringList &indexes, const QString &filter, bool permitirDescontinuados, QWidget *parent = 0);
  ~SearchDialog();
  void show();
  void showMaximized();
  void setFilter(const QString &value);
  QString getFilter() const;
  void setRepresentacao(const QString &value);
  QString getText(const QVariant &value);
  void setFornecedorRep(const QString &value);

  // Factory Methods
  static SearchDialog *cliente(QWidget *parent);
  static SearchDialog *conta(QWidget *parent);
  static SearchDialog *enderecoCliente(QWidget *parent);
  static SearchDialog *fornecedor(QWidget *parent);
  static SearchDialog *loja(QWidget *parent);
  static SearchDialog *produto(bool permitirDescontinuados, QWidget *parent);
  static SearchDialog *profissional(QWidget *parent);
  static SearchDialog *transportadora(QWidget *parent);
  static SearchDialog *usuario(QWidget *parent);
  static SearchDialog *veiculo(QWidget *parent);
  static SearchDialog *vendedor(QWidget *parent);

signals:
  void itemSelected(const QVariant &value);

private slots:
  void on_lineEditBusca_textChanged(const QString &);
  void on_pushButtonSelecionar_clicked();
  void on_radioButtonProdAtivos_toggled(const bool);
  void on_radioButtonProdDesc_toggled(const bool);
  void on_table_doubleClicked(const QModelIndex &);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  const QDataWidgetMapper mapper;
  const QStringList indexes;
  const QVector<QPair<QString, QString>> headerData;
  bool permitirDescontinuados;
  QString filter;
  QString fornecedorRep;
  QString primaryKey;
  QString representacao;
  QStringList textKeys;
  SqlTableModel model;
  Ui::SearchDialog *ui;
  // methods
  void hideColumns(const QStringList &columns);
  void sendUpdateMessage();
  void setHeaderData(const QString &column, const QString &value);
  void setPrimaryKey(const QString &value);
  void setTextKeys(const QStringList &value);
  void setupTables(const QString &table, const QString &filter);
};

#endif // SEARCHDIALOG_H
