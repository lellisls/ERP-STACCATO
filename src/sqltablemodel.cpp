#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "sqltablemodel.h"

SqlTableModel::SqlTableModel(QObject *parent) : QSqlRelationalTableModel(parent) {}

QVariant SqlTableModel::data(const int row, const int column) const { return QSqlTableModel::data(QSqlTableModel::index(row, column)); }

QVariant SqlTableModel::data(const int row, const QString &column) const {
  if (QSqlTableModel::fieldIndex(column) == -1) {
    QMessageBox::critical(nullptr, "Erro!", "Chave " + column + " não encontrada na tabela " + QSqlTableModel::tableName());
    return QVariant();
  }

  return QSqlTableModel::data(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)));
}

bool SqlTableModel::setData(const int row, const int column, const QVariant &value) {
  if (not QSqlTableModel::setData(QSqlTableModel::index(row, column), value)) {
    QMessageBox::critical(nullptr, "Erro!", "Erro inserindo " + QSqlTableModel::record().fieldName(column) + " na tabela: " + QSqlTableModel::lastError().text());
    return false;
  }

  return true;
}

bool SqlTableModel::setData(const int row, const QString &column, const QVariant &value) {
  if (row == -1) {
    QMessageBox::critical(nullptr, "Erro!", "Erro: linha -1 SqlTableModel");
    return false;
  }

  if (QSqlTableModel::fieldIndex(column) == -1) {
    QMessageBox::critical(nullptr, "Erro!", "Chave " + column + " não encontrada na tabela " + QSqlTableModel::tableName());
    return false;
  }

  if (not QSqlTableModel::setData(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)), value)) {
    QMessageBox::critical(nullptr, "Erro!",
                          "Erro inserindo " + column + " na tabela " + tableName() + ": " + QSqlTableModel::lastError().text() + " - linha: " + QString::number(row) + " - valor: " + value.toString());
    return false;
  }

  return true;
}

bool SqlTableModel::setHeaderData(const QString &column, const QVariant &value) { return QSqlTableModel::setHeaderData(QSqlTableModel::fieldIndex(column), Qt::Horizontal, value); }

Qt::ItemFlags SqlTableModel::flags(const QModelIndex &index) const { return QSqlRelationalTableModel::flags(index); }

Qt::DropActions SqlTableModel::supportedDropActions() const { return Qt::MoveAction; }
