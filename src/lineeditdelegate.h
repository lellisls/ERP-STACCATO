#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QStyledItemDelegate>

class LineEditDelegate : public QStyledItemDelegate {

public:
  enum Tipo { ContraPartePagar, ContraParteReceber, Grupo };
  explicit LineEditDelegate(const Tipo tipo, QObject *parent);
  ~LineEditDelegate() = default;

  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;

private:
  const Tipo tipo;

  // QAbstractItemDelegate interface
public:
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
};

#endif // LINEEDITDELEGATE_H
