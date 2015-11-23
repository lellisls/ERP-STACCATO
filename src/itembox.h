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
    void setSearchDialog(SearchDialog *m_value);
    void setRegisterDialog(RegisterDialog *m_value);
    SearchDialog *searchDialog();
    QVariant value() const;
    void setValue(const QVariant &m_value);
    void setReadOnlyItemBox(const bool &readOnly);
    void clear();

  public slots:
    void changeItem(const QVariant &m_value);

  protected slots:
    virtual void search();
    virtual void edit();
    void resetCursor();

  protected:
    Q_PROPERTY(QVariant value READ value WRITE setValue STORED false)
    // attributes
    QPushButton *m_searchButton, *m_plusButton;
    SearchDialog *m_searchDialog = nullptr;
    RegisterDialog *m_registerDialog = nullptr;
    QVariant m_value;
    bool readOnlyItemBox = false;
    // methods
    void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    RegisterDialog *registerDialog();
};

#endif // ITEMBOX_H
