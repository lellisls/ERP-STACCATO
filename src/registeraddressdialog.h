#ifndef REGISTERADDRESSDIALOG_H
#define REGISTERADDRESSDIALOG_H

#include "registerdialog.h"

class RegisterAddressDialog : public RegisterDialog {

  public:
    explicit RegisterAddressDialog(const QString &table, const QString &primaryKey, QWidget *parent);

  public slots:
    bool viewRegisterById(const QVariant &id);

  protected:
    // attributes
    QDataWidgetMapper mapperEnd;
    SqlTableModel modelEnd;
    // methods
    bool newRegister();
    int getCodigoUF(QString uf) const;
    bool save(const bool &isUpdate = false);
    void setDataEnd(const QString &key, const QVariant &value);
};

#endif // REGISTERADDRESSDIALOG_H
