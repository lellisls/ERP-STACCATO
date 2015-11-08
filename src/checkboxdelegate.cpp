#include <QCheckBox>

#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

CheckBoxDelegate::~CheckBoxDelegate() {}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);

  QCheckBox *editor = new QCheckBox(parent);
  editor->setAutoFillBackground(true);

  return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (QCheckBox *cb = qobject_cast<QCheckBox *>(editor)) {
    cb->setChecked(index.data(Qt::EditRole).toBool());
    return;
  }

  QStyledItemDelegate::setEditorData(editor, index);
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (QCheckBox *cb = qobject_cast<QCheckBox *>(editor)) {
    model->setData(index, cb->isChecked(), Qt::EditRole);
    return;
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  Q_UNUSED(index);

  editor->setGeometry(option.rect);
}
