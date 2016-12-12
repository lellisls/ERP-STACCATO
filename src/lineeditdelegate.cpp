#include <QCompleter>
#include <QLineEdit>
#include <QSqlQueryModel>

#include "lineeditdelegate.h"

LineEditDelegate::LineEditDelegate(const Tipo tipo, QObject *parent) : QStyledItemDelegate(parent), tipo(tipo) {}

LineEditDelegate::~LineEditDelegate() {}

QWidget *LineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  QLineEdit *editor = new QLineEdit(parent);

  QSqlQueryModel *model = new QSqlQueryModel(parent);

  if (tipo == ContraPartePagar) model->setQuery("SELECT DISTINCT(contraParte) FROM conta_a_pagar_has_pagamento");
  if (tipo == ContraParteReceber) model->setQuery("SELECT DISTINCT(contraParte) FROM conta_a_receber_has_pagamento");
  if (tipo == Grupo) model->setQuery("SELECT tipo FROM despesa");

  QCompleter *completer = new QCompleter(model, parent);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  editor->setCompleter(completer);

  return editor;
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &) const {
  editor->setGeometry(option.rect.adjusted(0, 0, 200, 0));
}
