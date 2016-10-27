#ifndef ITEMBOXDELEGATE_H
#define ITEMBOXDELEGATE_H

#include <QStyledItemDelegate>

class ItemBoxDelegate : public QStyledItemDelegate {

public:
  enum Tipo { Loja, Conta };
  ItemBoxDelegate(Tipo tipo, bool isReadOnly, QObject *parent);
  ~ItemBoxDelegate();

  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                    const QModelIndex &) const override;

private:
  Tipo tipo;
  bool isReadOnly;
  void commitAndCloseEditor();
};

#endif // ITEMBOXDELEGATE_H
