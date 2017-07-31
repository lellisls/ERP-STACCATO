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
  enum Type { Recebimento, Entrega, Representacao };

  explicit InputDialogConfirmacao(const Type &type, QWidget *parent = 0);
  ~InputDialogConfirmacao();
  QDateTime getDate() const;
  QDateTime getNextDate() const;
  QString getRecebeu() const;
  QString getEntregou() const;
  bool setFilter(const QStringList &ids);
  bool setFilter(const QString &id, const QString &idEvento);

private slots:
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_pushButtonQuebradoFaltando_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  const Type type;
  QString error;
  SqlTableModel model; // separate this into 2 models? one for each table
  SqlTableModel modelCliente;
  SqlTableModel modelVenda;
  Ui::InputDialogConfirmacao *ui;
  // temp
  int caixasDefeito;
  double unCaixa;
  //

  // methods
  bool cadastrar();
  void setupTables();
  bool gerarCreditoCliente();
  bool criarReposicaoCliente();
  bool quebraEntrega(const int row);
  bool quebraRecebimento(const int row);
  bool processarQuebra(const int row);
  bool quebrarLinha(const int row, const int caixas);
  bool criarConsumo(const int row);
  bool desfazerConsumo(const int idEstoque);
};

#endif // INPUTDIALOGCONFIRMACAO_H
