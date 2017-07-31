#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>

class CheckBoxDelegate : public QStyledItemDelegate {

public:
  explicit CheckBoxDelegate(QObject *parent, const bool readOnly = false);
  ~CheckBoxDelegate() = default;

private:
  // attributes
  const bool readOnly;
  // methods
  virtual QString displayText(const QVariant &, const QLocale &) const override;
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
  void commitAndCloseEditor();
};

#endif // CHECKBOXDELEGATE_H
