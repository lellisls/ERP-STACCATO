#include "itembox.h"
#include <QDebug>

ItemBox::ItemBox(QWidget *parent) :
  QLineEdit(parent),
  searchDialog(nullptr),
  registerDialog(nullptr) {
  setReadOnly(true);

  searchButton = new QPushButton(this);
  searchButton->setIcon(QIcon(":/search.png"));
  searchButton->setAutoDefault(false);
  searchButton->setFlat(true);
  searchButton->setIconSize(QSize(14, 14));

  plusButton = new QPushButton(this);
  plusButton->setIcon(QIcon(":/plus.png"));
  plusButton->setAutoDefault(false);
  plusButton->setFlat(true);
  plusButton->setIconSize(QSize(14, 14));

  ensurePolished();

  connect(searchButton, &QAbstractButton::clicked, this, &ItemBox::search);
  connect(plusButton, &QAbstractButton::clicked, this, &ItemBox::edit);
}

ItemBox::~ItemBox() {}

void ItemBox::resizeEvent(QResizeEvent *event) {
  QLineEdit::resizeEvent(event);
  QSize size = searchButton->minimumSizeHint();
  int x = rect().right();
  int y = (rect().height() - size.height()) / 2.0;
  if(searchDialog) {
    x -= size.width();
    searchButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    searchButton->hide();
  }
  if(registerDialog) {
    x -= size.width();
    plusButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    plusButton->hide();
  }
  int left, top, bottom;
  getTextMargins(&left, &top, 0, &bottom);
  setTextMargins(left, top, 2 + rect().right() - x + 2, bottom);
}

void ItemBox::search() {
  if (searchDialog) {
    searchDialog->show();
  }
}

void ItemBox::edit() {
  if(registerDialog) {
    if(!value.isNull()) {
      registerDialog->viewRegisterById(value);
    }
    registerDialog->show();
  }
}

void ItemBox::setRegisterDialog(RegisterDialog *value) {
  registerDialog = value;
  connect(value, &RegisterDialog::registerUpdated, this, &ItemBox::changeItem);
}

QVariant ItemBox::getValue() const {
  return value;
}

void ItemBox::setValue(const QVariant &value) {
  qDebug() << "Set value : " << value;
  this->value = value;
  if(value.isNull()) {
    setText("");
  } else if( searchDialog ) {
    setText(searchDialog->getText(value));
    qDebug() << "Text = " << text();
  }
}

void ItemBox::setSearchDialog(SearchDialog *value) {
  searchDialog = value;
  connect(searchDialog, &SearchDialog::itemSelected, this, &ItemBox::changeItem);
}

void ItemBox::changeItem(QVariant value, QString text) {
  qDebug() << objectName() << " : changeItem : " << __LINE__ << ", value = " << value << ", text = " << text;

  setValue(value);
//  setText(text);
  if(registerDialog) {
    registerDialog->cancel();
  }
  if(searchDialog) {
    searchDialog->close();
  }
  qDebug() << "Value changed: " << value << ", " << text;
}
