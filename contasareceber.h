#ifndef CONTASARECEBER_H
#define CONTASARECEBER_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
  class ContasAReceber;
}

class ContasAReceber : public QDialog {
    Q_OBJECT

  public:
    explicit ContasAReceber(QWidget *parent = 0);
    ~ContasAReceber();
    void viewConta(QString idVenda);

  private slots:
    void on_checkBox_toggled(bool checked);
    void on_pushButtonSalvar_clicked();
    void on_pushButtonCancelar_clicked();

  private:
    // attributes
    Ui::ContasAReceber *ui;
    QSqlTableModel modelContas, modelItensConta;
    QString idVenda;
};

#endif // CONTASARECEBER_H
