#ifndef REGISTERADDRESSDIALOG_H
#define REGISTERADDRESSDIALOG_H

#include "registerdialog.h"

class RegisterAddressDialog : public RegisterDialog {

  public:
    explicit RegisterAddressDialog(QString table, QString primaryKey, QWidget *parent);
    bool viewRegister(const QModelIndex index);

  public slots:
    bool viewRegisterById(const QVariant id);

  protected:
    // attributes
    QDataWidgetMapper mapperEnd;
    QSqlTableModel modelEnd;
    // methods
    bool newRegister();
    int getCodigoUF(const QString uf) const;
    bool save(const bool isUpdate = false);
    bool setDataEnd(int row, const QString &key, QVariant value);
};

#endif // REGISTERADDRESSDIALOG_H
