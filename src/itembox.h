#ifndef ITEMBOX_H
#define ITEMBOX_H

#include <QLineEdit>
#include <QPushButton>

#include "registerdialog.h"
#include "searchdialog.h"

class ItemBox : public QLineEdit {
  Q_OBJECT

public:
  explicit ItemBox(QWidget *parent);
  ~ItemBox() = default;
  QVariant getValue() const;
  SearchDialog *getSearchDialog();
  void changeItem(const QVariant &value);
  void clear();
  void setReadOnlyItemBox(const bool isReadOnly);
  void setRegisterDialog(RegisterDialog *value);
  void setSearchDialog(SearchDialog *value);
  void setValue(const QVariant &value);

private:
  Q_PROPERTY(QVariant value READ getValue WRITE setValue STORED false)
  // attributes
  bool readOnlyItemBox = false;
  QPushButton *searchButton;
  QPushButton *plusButton;
  QVariant value;
  RegisterDialog *registerDialog = nullptr;
  SearchDialog *searchDialog = nullptr;
  // methods
  virtual void edit();
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void search();
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void resetCursor();
};

#endif // ITEMBOX_H
