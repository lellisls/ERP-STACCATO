#include <QDebug>
#include <QMouseEvent>

#include "itembox.h"

ItemBox::ItemBox(QWidget *parent) : QLineEdit(parent) {
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
  connect(this, &QLineEdit::cursorPositionChanged, this, &ItemBox::resetCursor);
}

void ItemBox::resizeEvent(QResizeEvent *event) {
  QLineEdit::resizeEvent(event);

  const QSize size = searchButton->minimumSizeHint();
  int x = rect().right();
  int y = (rect().height() - size.height()) / 2;

  if (searchDialog) {
    x -= size.width();
    searchButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    searchButton->hide();
  }

  if (registerDialog) {
    x -= size.width();
    plusButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    plusButton->hide();
  }

  int left, top, bottom;
  getTextMargins(&left, &top, nullptr, &bottom);
  setTextMargins(left, top, 2 + rect().right() - x + 2, bottom);
}

void ItemBox::search() {
  if (searchDialog) searchDialog->show();
}

void ItemBox::edit() {
  if (registerDialog) {
    if (not value.isNull()) registerDialog->viewRegisterById(value);

    registerDialog->show();
  }
}

void ItemBox::resetCursor() { setCursorPosition(0); }

void ItemBox::setRegisterDialog(RegisterDialog *value) {
  registerDialog = value;
  connect(value, &RegisterDialog::registerUpdated, this, &ItemBox::changeItem);
}

SearchDialog *ItemBox::getSearchDialog() { return searchDialog; }

QVariant ItemBox::getValue() const { return value; }

void ItemBox::setValue(const QVariant &value) {
  if (value.isNull()) return;

  this->value = value;

  if (searchDialog) setText(searchDialog->getText(value));

  QLineEdit::setToolTip(text());
}

void ItemBox::setReadOnlyItemBox(const bool isReadOnly) {
  readOnlyItemBox = isReadOnly;

  plusButton->setHidden(isReadOnly);
  plusButton->setDisabled(isReadOnly);
  searchButton->setHidden(isReadOnly);
  searchButton->setDisabled(isReadOnly);
}

void ItemBox::clear() {
  value.clear();

  QLineEdit::clear();
}

void ItemBox::setSearchDialog(SearchDialog *value) {
  searchDialog = value;
  connect(searchDialog, &SearchDialog::itemSelected, this, &ItemBox::changeItem);
}

void ItemBox::changeItem(const QVariant &value) {
  setValue(value);

  if (registerDialog) registerDialog->close();
  if (searchDialog) searchDialog->close();
}

void ItemBox::mouseDoubleClickEvent(QMouseEvent *event) {
  if (readOnlyItemBox) return;

  search();
  event->accept();
}
