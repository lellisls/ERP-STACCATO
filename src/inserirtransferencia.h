#ifndef INSERIRTRANSFERENCIA_H
#define INSERIRTRANSFERENCIA_H

#include "sqltablemodel.h"

#include <QDialog>

namespace Ui {
class InserirTransferencia;
}

class InserirTransferencia : public QDialog {
  Q_OBJECT

public:
  explicit InserirTransferencia(QWidget *parent = 0);
  ~InserirTransferencia();

private slots:
  void on_pushButtonSalvar_clicked();
  void on_pushButtonCancelar_clicked();

private:
  QString error;
  SqlTableModel modelDe;
  SqlTableModel modelPara;
  Ui::InserirTransferencia *ui;
  bool verifyFields();
  void setupTables();
  bool cadastrar();
};

#endif // INSERIRTRANSFERENCIA_H
