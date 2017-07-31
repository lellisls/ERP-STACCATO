#include <QComboBox>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "comboboxdelegate.h"
#include "usersession.h"

ComboBoxDelegate::ComboBoxDelegate(const Tipo tipo, QObject *parent) : QStyledItemDelegate(parent), tipo(tipo) {}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  auto *editor = new QComboBox(parent);

  QStringList list;

  if (tipo == Status) {
    list << "PENDENTE"
         << "COMPRAR"
         << "PAGO"
         << "RECEBIDO";
  }

  if (tipo == StatusReceber) {
    list << "PENDENTE"
         << "RECEBIDO"
         << "CANCELADO"
         << "CONFERIDO";
  }

  if (tipo == StatusPagar) {
    list << "PENDENTE"
         << "PAGO"
         << "CANCELADO"
         << "CONFERIDO";
  }

  if (tipo == Pagamento) {
    QSqlQuery query;
    query.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
    query.bindValue(":idLoja", UserSession::idLoja());

    if (not query.exec()) {
      QMessageBox::critical(parent, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    }

    list << "";

    while (query.next()) list << query.value("pagamento").toString();

    list << "Conta Cliente";
  }

  if (tipo == Conta) {
    QSqlQuery query;

    if (not query.exec("SELECT banco, agencia, conta FROM loja_has_conta")) {
      QMessageBox::critical(parent, "Erro!", "Erro lendo contas da loja: " + query.lastError().text());
    }

    list << "";

    while (query.next()) {
      list << query.value("banco").toString() + " - " + query.value("agencia").toString() + " - " + query.value("conta").toString();
    }
  }

  if (tipo == Grupo) {
    QSqlQuery query;

    if (not query.exec("SELECT tipo FROM despesa ORDER BY tipo")) {
      QMessageBox::critical(parent, "Erro!", "Erro lendo grupos de despesa: " + query.lastError().text());
    }

    list << "";

    while (query.next()) list << query.value("tipo").toString();
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

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const { editor->setGeometry(option.rect); }
