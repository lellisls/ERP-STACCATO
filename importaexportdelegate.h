#ifndef IMPORTAEXPORTDELEGATE_H
#define IMPORTAEXPORTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class ImportaExportDelegate : public QStyledItemDelegate {

  public:
    ImportaExportDelegate(QObject *parent = 0);
    ~ImportaExportDelegate();
};

#endif // IMPORTAEXPORTDELEGATE_H
