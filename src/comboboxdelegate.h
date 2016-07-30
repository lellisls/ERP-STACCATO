#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

class ComboBoxDelegate : public QStyledItemDelegate {

public:
  enum Tipo { Status, StatusReceber, StatusPagar, Conta };
  explicit ComboBoxDelegate(Tipo tipo, QObject *parent = 0);
  ~ComboBoxDelegate();

private:
  Tipo tipo;
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                    const QModelIndex &) const override;
};

#endif // COMBOBOXDELEGATE_H
