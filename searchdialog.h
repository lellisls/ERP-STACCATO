#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QSqlQuery>
//#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>

namespace Ui {
  class SearchDialog;
}

class SearchDialog : public QDialog {
    Q_OBJECT

  public:
    explicit SearchDialog(QString title, QString table, QStringList indexes, QString filter,
                          QWidget *parent = 0);
    ~SearchDialog();
    QString getFilter() const;
    void setFilter(const QString &value);
    void hideColumns(QStringList columns);
    QString getPrimaryKey() const;
    void setPrimaryKey(const QString &value);
    QStringList getTextKeys() const;
    void setTextKeys(const QStringList &value);
    QString getText(QVariant idx);
    void setHeaderData(QVector<QPair<QString, QString>> headerData);

    // Factory Methods
    static SearchDialog *cliente(QWidget *parent);
    static SearchDialog *loja(QWidget *parent);
    static SearchDialog *produto(QWidget *parent);
    static SearchDialog *fornecedor(QWidget *parent);
    static SearchDialog *transportadora(QWidget *parent);
    static SearchDialog *usuario(QWidget *parent);
    static SearchDialog *profissional(QWidget *parent);
    static SearchDialog *endereco(QWidget *parent);
    static SearchDialog *vendedor(QWidget *parent);
    void sendUpdateMessage();
    void show();
    void showMaximized();

  signals:
    void itemSelected(QVariant value, QString text);

  private slots:
    void on_lineEditBusca_textChanged(const QString &text);
    void on_tableBusca_doubleClicked(const QModelIndex &index);
    void on_pushButtonSelecionar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_radioButtonProdAtivos_clicked();
    void on_radioButtonProdDesc_clicked();

  private:
    Ui::SearchDialog *ui;
    QDataWidgetMapper mapper;
    QSqlRelationalTableModel model;
    QStringList indexes;
    QVariant selectedId;
    QString filter;
    QString primaryKey;
    QStringList textKeys;
    QVector<QPair<QString, QString>> headerData;
};

#endif // SEARCHDIALOG_H
