#include <QComboBox>
#include <QDebug>
#include <QTableView>

#include "comboboxdelegate.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

ComboBoxDelegate::~ComboBoxDelegate() {}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);

  QComboBox *editor = new QComboBox(parent);
  QStringList list;
  list << "Pendente"
       << "Comprar"
       << "Pago"
       << "Recebido";
  editor->addItems(list);
  return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
    QString currentText = index.data(Qt::EditRole).toString();
    int cbIndex = cb->findText(currentText);
    if (cbIndex >= 0) {
      cb->setCurrentIndex(cbIndex);
    }
  } else {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
  if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
    model->setData(index, cb->currentText(), Qt::EditRole);
  } else {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  Q_UNUSED(index);

  editor->setGeometry(option.rect);
}
