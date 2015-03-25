#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include "registerdialog.h"

#include <QDialog>

namespace Ui {
  class CadastrarNFE;
}

class CadastrarNFE : public RegisterDialog {
  Q_OBJECT

public:
  explicit CadastrarNFE(QWidget *parent = 0);
  ~CadastrarNFE();

  void gerarNFe(QString idVenda, QList<int> items );

  void setItemData(int row, const QString & key, const QVariant &value);
  private slots:
  void on_pushButtonGerarNFE_clicked();

  void on_pushButtonCancelar_clicked();

private:
  Ui::CadastrarNFE *ui;
  QSqlRelationalTableModel modelItem;

  // RegisterDialog interface
protected:
  bool verifyFields(int row);
  bool savingProcedures(int row);
  void clearFields();
  void setupMapper();
  void registerMode();
  void updateMode();
};

#endif // CADASTRARNFE_H
