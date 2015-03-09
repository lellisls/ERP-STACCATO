#ifndef ITEMBOX_H
#define ITEMBOX_H

#include <QPushButton>
#include <QLineEdit>
#include "searchdialog.h"
#include "registerdialog.h"

class ItemBox : public QLineEdit {
    Q_OBJECT

  public:
    ItemBox(QWidget *parent);
    ~ItemBox();
    virtual void resizeEvent(QResizeEvent *event);
    void setSearchDialog(SearchDialog *value);
    void setRegisterDialog(RegisterDialog *value);
    QVariant getValue() const;
    void setValue(const QVariant &value);

  public slots:
    void changeItem(QVariant value, QString text);

  protected slots:
    virtual void search();
    virtual void edit();

  protected:
    Q_PROPERTY(QVariant value READ getValue WRITE setValue STORED false)
    QPushButton *searchButton, *plusButton;
    SearchDialog *searchDialog;
    RegisterDialog *registerDialog;
    QVariant value;
};

#endif // ITEMBOX_H
