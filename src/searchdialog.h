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
    explicit SearchDialog(const QString &title, const QString &table, const QStringList &indexes, const QString &filter,
                          QWidget *parent = 0);
    ~SearchDialog();
    void show();
    void showMaximized();
    void setFilter(const QString &value);
    void setRepresentacao(const QString &value);
    QString getText(const QVariant &value);

    // Factory Methods
    static SearchDialog *cliente(QWidget *parent);
    static SearchDialog *fornecedor(QWidget *parent);
    static SearchDialog *usuario(QWidget *parent);
    static SearchDialog *profissional(QWidget *parent);
    static SearchDialog *enderecoCliente(QWidget *parent);
    static SearchDialog *loja(QWidget *parent);
    static SearchDialog *produto(QWidget *parent);
    static SearchDialog *transportadora(QWidget *parent);
    static SearchDialog *vendedor(QWidget *parent);

  signals:
    void itemSelected(QVariant value);

  private slots:
    void on_lineEditBusca_textChanged(const QString &text);
    void on_pushButtonSelecionar_clicked();
    void on_radioButtonProdAtivos_toggled(const bool &);
    void on_radioButtonProdDesc_toggled(const bool &);
    void on_table_doubleClicked(const QModelIndex &);
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::SearchDialog *ui;
    QDataWidgetMapper mapper;
    QString filter;
    QString primaryKey;
    QString representacao;
    QStringList indexes;
    QStringList textKeys;
    QVector<QPair<QString, QString>> headerData;
    SqlTableModel model;
    // methods
    void hideColumns(const QStringList &columns);
    void montarFiltroAtivoDesc(const bool &ativo);
    void sendUpdateMessage();
    void setHeaderData(const QString &column, const QString &value);
    void setPrimaryKey(const QString &value);
    void setTextKeys(const QStringList &value);
};

#endif // SEARCHDIALOG_H
