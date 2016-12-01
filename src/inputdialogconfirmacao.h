#ifndef INPUTDIALOGCONFIRMACAO_H
#define INPUTDIALOGCONFIRMACAO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class InputDialogConfirmacao;
}

class InputDialogConfirmacao : public QDialog {
  Q_OBJECT

public:
  enum Type { Recebimento, Entrega } type;

  explicit InputDialogConfirmacao(const Type &type, QWidget *parent = 0);
  ~InputDialogConfirmacao();
  QDateTime getDate() const;
  QDateTime getNextDate() const;
  QString getRecebeu() const;
  QString getEntregou() const;
  bool setFilter(const QStringList &ids);
  bool setFilter(const QString &id);

private slots:
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_pushButtonQuebradoFaltando_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  SqlTableModel model;
  Ui::InputDialogConfirmacao *ui;
  // methods
  bool cadastrar();
  void setupTables();
};

#endif // INPUTDIALOGCONFIRMACAO_H
