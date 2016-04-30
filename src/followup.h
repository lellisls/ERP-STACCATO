#ifndef FOLLOWUP_H
#define FOLLOWUP_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class FollowUp;
}

class FollowUp : public QDialog
{
    Q_OBJECT

public:
    explicit FollowUp(QString idOrcamento, QWidget *parent = 0);
    ~FollowUp();

private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

private:
    // attributes
    Ui::FollowUp *ui;
    int row;
    QString idOrcamento;
    SqlTableModel model;
    // methods
    void setupTables();
    bool savingProcedures();
    bool verifyFields();
};

#endif // FOLLOWUP_H
