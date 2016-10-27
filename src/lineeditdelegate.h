#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QStyledItemDelegate>

class LineEditDelegate : public QStyledItemDelegate {

public:
  enum Tipo { ContraPartePagar, ContraParteReceber, Grupo };
  explicit LineEditDelegate(Tipo tipo, QObject *parent);
  ~LineEditDelegate();

  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;

private:
  Tipo tipo;

  // QAbstractItemDelegate interface
public:
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
};

#endif // LINEEDITDELEGATE_H
