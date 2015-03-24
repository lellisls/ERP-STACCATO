#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDialog>

namespace Ui {
  class CadastrarNFE;
}

class CadastrarNFE : public QDialog {
  Q_OBJECT

public:
  explicit CadastrarNFE(QWidget *parent = 0);
  ~CadastrarNFE();

private:
  Ui::CadastrarNFE *ui;
};

#endif // CADASTRARNFE_H
