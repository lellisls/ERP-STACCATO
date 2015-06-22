#ifndef APAGAORCAMENTO_H
#define APAGAORCAMENTO_H

#include <QDialog>
#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>

namespace Ui {
  class ApagaOrcamento;
}

class ApagaOrcamento : public QDialog {
    Q_OBJECT

  public:
    explicit ApagaOrcamento(QWidget *parent = 0);
    ~ApagaOrcamento();
    void apagar(const int index);

  private slots:
    void on_pushButtonSalvar_clicked();
    void on_pushButtonCancelar_clicked();

  private:
    Ui::ApagaOrcamento *ui;
    QSqlRelationalTableModel modelOrc;
    QDataWidgetMapper mapperOrc;
};

#endif // APAGAORCAMENTO_H
