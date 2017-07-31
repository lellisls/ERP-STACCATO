#include <QCheckBox>
#include <QDebug>

#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent, const bool readOnly) : QStyledItemDelegate(parent), readOnly(readOnly) {}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  auto *editor = new QCheckBox(parent);
  if (readOnly) editor->setDisabled(true);

  connect(editor, &QCheckBox::toggled, this, &CheckBoxDelegate::commitAndCloseEditor);

  return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (auto *cb = qobject_cast<QCheckBox *>(editor)) cb->setChecked(index.data(Qt::EditRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (auto *cb = qobject_cast<QCheckBox *>(editor)) model->setData(index, cb->isChecked(), Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const { editor->setGeometry(option.rect); }

QString CheckBoxDelegate::displayText(const QVariant &, const QLocale &) const { return QString(); }

void CheckBoxDelegate::commitAndCloseEditor() {
  QWidget *editor = qobject_cast<QWidget *>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}
