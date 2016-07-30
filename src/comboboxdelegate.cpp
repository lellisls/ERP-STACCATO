#include <QComboBox>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "comboboxdelegate.h"

ComboBoxDelegate::ComboBoxDelegate(Tipo tipo, QObject *parent) : QStyledItemDelegate(parent), tipo(tipo) {}

ComboBoxDelegate::~ComboBoxDelegate() {}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  QComboBox *editor = new QComboBox(parent);

  QStringList list;

  if (tipo == Status) {
    list << "Pendente"
         << "Comprar"
         << "Pago"
         << "Recebido";
  }

  if (tipo == StatusReceber) {
    list << "Pendente"
         << "Recebido";
  }

  if (tipo == StatusPagar) {
    list << "Pendente"
         << "Pago";
  }

  if (tipo == Conta) {
    QSqlQuery query;

    if (not query.exec("SELECT banco, agencia, conta FROM loja_has_conta")) {
      QMessageBox::critical(parent, "Erro!", "Erro lendo contas da loja: " + query.lastError().text());
    }

    list << "";

    while (query.next()) {
      list << query.value("banco").toString() + " - " + query.value("agencia").toString() + " - " +
                  query.value("conta").toString();
    }
  }

  editor->addItems(list);

  return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (auto *cb = qobject_cast<QComboBox *>(editor)) {
    const int cbIndex = cb->findText(index.data(Qt::EditRole).toString());

    if (cbIndex >= 0) cb->setCurrentIndex(cbIndex);

    return;
  }

  QStyledItemDelegate::setEditorData(editor, index);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (auto *cb = qobject_cast<QComboBox *>(editor)) {
    model->setData(index, cb->currentText(), Qt::EditRole);
    return;
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &) const {
  editor->setGeometry(option.rect);
}
