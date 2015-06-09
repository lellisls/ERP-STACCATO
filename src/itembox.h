#ifndef ITEMBOX_H
#define ITEMBOX_H

#include <QPushButton>
#include <QLineEdit>

#include "searchdialog.h"
#include "registerdialog.h"

class ItemBox : public QLineEdit {
    Q_OBJECT

  public:
    explicit ItemBox(QWidget *parent);
    ~ItemBox();
    virtual void resizeEvent(QResizeEvent *event);
    void setSearchDialog(SearchDialog *m_value);
    void setRegisterDialog(RegisterDialog *m_value);
    SearchDialog *searchDialog();
    RegisterDialog *registerDialog();
    QVariant value() const;
    void setValue(const QVariant &m_value);

  public slots:
    void changeItem(QVariant m_value, QString text);

  protected slots:
    virtual void search();
    virtual void edit();

  protected:
    Q_PROPERTY(QVariant value READ value WRITE setValue STORED false)
    QPushButton *m_searchButton, *m_plusButton;
    SearchDialog *m_searchDialog;
    RegisterDialog *m_registerDialog;
    QVariant m_value;

    // QWidget interface
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // ITEMBOX_H
