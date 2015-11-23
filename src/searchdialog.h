#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QDataWidgetMapper>

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
    QString getText(const QVariant &index);

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
    void on_tableBusca_doubleClicked(const QModelIndex &);
    void on_pushButtonSelecionar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_tableBusca_entered(const QModelIndex &);
    void on_radioButtonProdAtivos_toggled(const bool &);
    void on_radioButtonProdDesc_toggled(const bool &);

  private:
    // attributes
    Ui::SearchDialog *ui;
    QDataWidgetMapper mapper;
    SqlTableModel model;
    QStringList indexes;
    QVariant selectedId;
    QString filter;
    QString primaryKey;
    QString representacao;
    QStringList textKeys;
    QVector<QPair<QString, QString>> headerData;
    // methods
    QString getFilter() const;
    void hideColumns(const QStringList &columns);
    QString getPrimaryKey() const;
    void setPrimaryKey(const QString &value);
    QStringList getTextKeys() const;
    void setTextKeys(const QStringList &value);
    void setHeaderData(const QString &column, const QString &value);
    void sendUpdateMessage();
    void montarFiltroAtivoDesc(const bool &ativo);
};

#endif // SEARCHDIALOG_H
