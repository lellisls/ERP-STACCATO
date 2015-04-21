#ifndef IMPORTATESTE2_H
#define IMPORTATESTE2_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QSqlRelationalTableModel>

class ImportaTeste2 : public QDialog
{
    Q_OBJECT

public:
    explicit ImportaTeste2(QWidget *parent = 0);
    ~ImportaTeste2();
    int buscarCadastrarFornecedor(QString fornecedor);

private:
    QSqlRelationalTableModel model;
    QProgressDialog *progressDialog;
};

#endif // IMPORTATESTE2_H
