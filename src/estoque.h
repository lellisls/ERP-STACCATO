#ifndef ESTOQUE_H
#define ESTOQUE_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
  class Estoque;
}

class Estoque : public QDialog {
    Q_OBJECT

  public:
    explicit Estoque(QWidget *parent = 0);
    ~Estoque();
    void viewRegisterById(const QVariant id);

  private slots:
    void on_tableEstoque_activated(const QModelIndex &index);

  private:
    Ui::Estoque *ui;
    QSqlTableModel modelEstoque;
};

#endif // ESTOQUE_H
