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
    inline bool setDataEnd(int row, const QString &key, QVariant value) {
      if (row == -1) {
        qDebug() << "Something wrong on the row!";
        return false;
      }

      if (modelEnd.fieldIndex(key) == -1) {
        qDebug() << objectName() << " : Key '" << key << "' not found on table '" << modelEnd.tableName() << "'";
        return false;
      }

      if (not value.isNull()) {
        if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex(key)), value)) {
          qDebug() << "row: " << row;
          qDebug() << "column: " << modelEnd.fieldIndex(key);
          qDebug() << "key: " << key;
          qDebug() << "index: " << modelEnd.index(row, modelEnd.fieldIndex(key)).isValid();
          return false;
        }
      }

      return true;
    }
};

#endif // REGISTERADDRESSDIALOG_H
