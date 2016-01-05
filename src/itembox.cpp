#include <QMouseEvent>

#include "itembox.h"

ItemBox::ItemBox(QWidget *parent) : QLineEdit(parent) {
  setReadOnly(true);

  m_searchButton = new QPushButton(this);
  m_searchButton->setIcon(QIcon(":/search.png"));
  m_searchButton->setAutoDefault(false);
  m_searchButton->setFlat(true);
  m_searchButton->setIconSize(QSize(14, 14));

  m_plusButton = new QPushButton(this);
  m_plusButton->setIcon(QIcon(":/plus.png"));
  m_plusButton->setAutoDefault(false);
  m_plusButton->setFlat(true);
  m_plusButton->setIconSize(QSize(14, 14));

  ensurePolished();

  connect(m_searchButton, &QAbstractButton::clicked, this, &ItemBox::search);
  connect(m_plusButton, &QAbstractButton::clicked, this, &ItemBox::edit);
  connect(this, &QLineEdit::cursorPositionChanged, this, &ItemBox::resetCursor);
}

ItemBox::~ItemBox() {}

void ItemBox::resizeEvent(QResizeEvent *event) {
  QLineEdit::resizeEvent(event);
  const QSize size = m_searchButton->minimumSizeHint();
  int x = rect().right();
  int y = (rect().height() - size.height()) / 2.;

  if (m_searchDialog) {
    x -= size.width();
    m_searchButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    m_searchButton->hide();
  }

  if (m_registerDialog) {
    x -= size.width();
    m_plusButton->setGeometry(QRect(QPoint(x, y), size));
  } else {
    m_plusButton->hide();
  }

  int left, top, bottom;
  getTextMargins(&left, &top, 0, &bottom);
  setTextMargins(left, top, 2 + rect().right() - x + 2, bottom);
}

void ItemBox::search() {
  if (m_searchDialog) m_searchDialog->show();
}

void ItemBox::edit() {
  if (m_registerDialog) {
    if (not m_value.isNull()) m_registerDialog->viewRegisterById(m_value);

    m_registerDialog->show();
  }
}

void ItemBox::resetCursor() { setCursorPosition(0); }

void ItemBox::setRegisterDialog(RegisterDialog *value) {
  m_registerDialog = value;
  connect(value, &RegisterDialog::registerUpdated, this, &ItemBox::changeItem);
}

SearchDialog *ItemBox::searchDialog() { return m_searchDialog; }

RegisterDialog *ItemBox::registerDialog() { return m_registerDialog; }

QVariant ItemBox::value() const { return m_value; }

void ItemBox::setValue(const QVariant &value) {
  if (value.isNull()) return;

  m_value = value;

  if (m_searchDialog) setText(m_searchDialog->getText(value));
}

void ItemBox::setReadOnlyItemBox(const bool &readOnly) {
  readOnlyItemBox = readOnly;

  m_plusButton->setHidden(readOnly);
  m_plusButton->setDisabled(readOnly);
  m_searchButton->setHidden(readOnly);
  m_searchButton->setDisabled(readOnly);
}

void ItemBox::clear() {
  m_value.clear();

  QLineEdit::clear();
}

void ItemBox::setSearchDialog(SearchDialog *value) {
  m_searchDialog = value;
  connect(m_searchDialog, &SearchDialog::itemSelected, this, &ItemBox::changeItem);
}

void ItemBox::changeItem(const QVariant &value) {
  setValue(value);

  if (m_registerDialog) m_registerDialog->close();
  if (m_searchDialog) m_searchDialog->close();
}

void ItemBox::mouseDoubleClickEvent(QMouseEvent *event) {
  if (readOnlyItemBox) return;

  search();
  event->accept();
}
