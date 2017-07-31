#include <QDebug>

#include "itembox.h"
#include "itemboxdelegate.h"

ItemBoxDelegate::ItemBoxDelegate(const Tipo tipo, const bool isReadOnly, QObject *parent) : QStyledItemDelegate(parent), isReadOnly(isReadOnly), tipo(tipo) {}

QWidget *ItemBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  auto *editor = new ItemBox(parent);

  editor->setReadOnlyItemBox(isReadOnly);

  if (tipo == Loja) editor->setSearchDialog(SearchDialog::loja(parent));
  if (tipo == Conta) editor->setSearchDialog(SearchDialog::conta(parent));

  connect(editor, &ItemBox::textChanged, this, &ItemBoxDelegate::commitAndCloseEditor);

  return editor;
}

void ItemBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (auto *box = qobject_cast<ItemBox *>(editor)) box->setValue(index.data(Qt::EditRole).toInt());
}

void ItemBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (auto *box = qobject_cast<ItemBox *>(editor)) model->setData(index, box->getValue(), Qt::EditRole);
}

void ItemBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const { editor->setGeometry(option.rect); }

void ItemBoxDelegate::commitAndCloseEditor() {
  QWidget *editor = qobject_cast<QWidget *>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}
