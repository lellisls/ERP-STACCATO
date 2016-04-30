#ifndef CADASTROPRODUTO_H
#define CADASTROPRODUTO_H

#include "registerdialog.h"

namespace Ui {
class CadastroProduto;
}

class CadastroProduto : public RegisterDialog {
  Q_OBJECT

public:
  explicit CadastroProduto(QWidget *parent = 0);
  ~CadastroProduto();

private slots:
  void on_pushButtonAtualizar_clicked();
  void on_pushButtonCadastrar_clicked();
  void on_pushButtonNovoCad_clicked();
  void on_pushButtonRemover_clicked();
  void on_doubleSpinBoxVenda_valueChanged(const double &);
  void on_doubleSpinBoxCusto_valueChanged(const double &);

private:
  // attributes
  Ui::CadastroProduto *ui;
  // methods
  bool verifyFields(const bool &isUpdate);
  virtual bool save() override;
  virtual bool savingProcedures() override;
  virtual bool verifyFields() override;
  virtual void clearFields() override;
  virtual void registerMode() override;
  virtual void setupMapper() override;
  virtual void updateMode() override;
  void calcularMarkup();
};

#endif // CADASTROPRODUTO_H
