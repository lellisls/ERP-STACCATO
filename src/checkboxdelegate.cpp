#include <QCheckBox>

#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent, const bool &readOnly)
    : QStyledItemDelegate(parent), readOnly(readOnly) {}

CheckBoxDelegate::~CheckBoxDelegate() {}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  QCheckBox *editor = new QCheckBox(parent);
  if (readOnly) editor->setDisabled(true);

  return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  qobject_cast<QCheckBox *>(editor)->setChecked(index.data(Qt::EditRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  model->setData(index, qobject_cast<QCheckBox *>(editor)->isChecked(), Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &) const {
  editor->setGeometry(option.rect);
}

QString CheckBoxDelegate::displayText(const QVariant &, const QLocale &) const { return QString(); }
