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
    QVariant value() const;
    SearchDialog *searchDialog();
    void clear();
    void setReadOnlyItemBox(const bool &readOnly);
    void setRegisterDialog(RegisterDialog *m_value);
    void setSearchDialog(SearchDialog *m_value);
    void setValue(const QVariant &m_value);

  public slots:
    void changeItem(const QVariant &m_value);

  private slots:
    virtual void edit();
    virtual void search();
    void resetCursor();

  private:
    Q_PROPERTY(QVariant value READ value WRITE setValue STORED false)
    // attributes
    bool readOnlyItemBox = false;
    QPushButton *m_searchButton, *m_plusButton;
    QVariant m_value;
    RegisterDialog *m_registerDialog = nullptr;
    SearchDialog *m_searchDialog = nullptr;
   // methods
    RegisterDialog *registerDialog();
    virtual void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // ITEMBOX_H
