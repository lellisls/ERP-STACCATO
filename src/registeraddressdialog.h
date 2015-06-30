#ifndef REGISTERADDRESSDIALOG_H
#define REGISTERADDRESSDIALOG_H

#include "registerdialog.h"

class RegisterAddressDialog : public RegisterDialog
{
public:
    explicit RegisterAddressDialog(QString table, QString primaryKey, QWidget *parent);
};

#endif // REGISTERADDRESSDIALOG_H
