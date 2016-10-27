#include "itemboxdelegate.h"
#include "itembox.h"

#include <QDebug>

ItemBoxDelegate::ItemBoxDelegate(Tipo tipo, bool isReadOnly, QObject *parent)
    : QStyledItemDelegate(parent), tipo(tipo), isReadOnly(isReadOnly) {}

ItemBoxDelegate::~ItemBoxDelegate() {}

QWidget *ItemBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  ItemBox *editor = new ItemBox(parent);

  editor->setReadOnlyItemBox(isReadOnly);

  if (tipo == Loja) editor->setSearchDialog(SearchDialog::loja(parent));
  if (tipo == Conta) editor->setSearchDialog(SearchDialog::conta(parent));

  connect(editor, &ItemBox::textChanged, this, &ItemBoxDelegate::commitAndCloseEditor);

  return editor;
}

void ItemBoxDelegate::commitAndCloseEditor() {
  QWidget *editor = qobject_cast<QWidget *>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

void ItemBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (auto *box = qobject_cast<ItemBox *>(editor)) {
    box->setValue(index.data(Qt::EditRole).toInt());
    return;
  }
}

void ItemBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (auto *box = qobject_cast<ItemBox *>(editor)) {
    model->setData(index, box->value(), Qt::EditRole);
    return;
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

void ItemBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                           const QModelIndex &) const {
  editor->setGeometry(option.rect);
}

// TODO: refactor based on checkboxdelegate
