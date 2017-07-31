#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QLineEdit>

#include "sqltablemodel.h"

class RegisterDialog : public QDialog {
  Q_OBJECT

public:
  explicit RegisterDialog(const QString &table, const QString &primaryKey, QWidget *parent);
  virtual bool viewRegister();
  static QVariant getLastInsertId();

public slots:
  virtual bool viewRegisterById(const QVariant &id);
  void marcarDirty();
  void saveSlot();
  void show();

signals:
  void registerUpdated(const QVariant &idCliente, const QString &text);

protected:
  // attributes
  bool isDirty = false;
  bool isUpdate = false;
  int currentRow = -1;
  int currentRowEnd = -1;
  QDataWidgetMapper mapper;
  QString primaryId;
  QString primaryKey;
  QStringList textKeys;
  SqlTableModel model;
  // methods
  bool confirmationMessage();
  bool setData(const QString &key, const QVariant &value);
  bool validaCNPJ(const QString &text);
  bool validaCPF(const QString &text);
  bool verifyFields(const QList<QLineEdit *> &list);
  QString requiredStyle();
  QStringList getTextKeys() const;
  QVariant data(const int row, const QString &key);
  QVariant data(const QString &key);
  virtual bool cadastrar() = 0;
  virtual bool newRegister();
  virtual bool save() = 0;
  virtual bool savingProcedures() = 0;
  virtual bool verifyFields() = 0;
  virtual bool verifyRequiredField(QLineEdit *line, const bool silent = false);
  virtual void clearFields() = 0;
  virtual void registerMode() = 0;
  virtual void setupMapper() = 0;
  virtual void successMessage() = 0;
  virtual void updateMode() = 0;
  void addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName = QByteArray());
  void closeEvent(QCloseEvent *event) override;
  void errorMessage();
  void keyPressEvent(QKeyEvent *event) override;
  void remove();
  void setTextKeys(const QStringList &value);
};

#endif // REGISTERDIALOG_H
