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
  ~ItemBox();
  QVariant value() const;
  SearchDialog *searchDialog();
  void changeItem(const QVariant &m_value);
  void clear();
  void setReadOnlyItemBox(const bool isReadOnly);
  void setRegisterDialog(RegisterDialog *m_value);
  void setSearchDialog(SearchDialog *m_value);
  void setValue(const QVariant &m_value);

private:
  Q_PROPERTY(QVariant value READ value WRITE setValue STORED false)
  // attributes
  bool isReadOnlyItemBox = false;
  QPushButton *m_searchButton, *m_plusButton;
  QVariant m_value;
  RegisterDialog *m_registerDialog = nullptr;
  SearchDialog *m_searchDialog = nullptr;
  // methods
  RegisterDialog *registerDialog();
  virtual void edit();
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void search();
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void resetCursor();
};

#endif // ITEMBOX_H
