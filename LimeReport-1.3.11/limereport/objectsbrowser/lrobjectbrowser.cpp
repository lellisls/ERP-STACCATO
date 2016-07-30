/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "lrobjectbrowser.h"
#include "lrbanddesignintf.h"
#include "lritemdesignintf.h"
#include <QVBoxLayout>

namespace LimeReport {

ObjectBrowser::ObjectBrowser(QWidget *parent) : QWidget(parent), m_changingItemSelection(false) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);
  layout->setMargin(2);
  m_treeView = new QTreeWidget(this);
  layout->addWidget(m_treeView);
  m_treeView->headerItem()->setText(0, tr("Objects"));
  m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void ObjectBrowser::setReportEditor(ReportDesignWidget *report) {
  m_report = report;
  connect(m_report, &ReportDesignWidget::cleared, this, &ObjectBrowser::slotClear);
  connect(m_report, &ReportDesignWidget::loaded, this, &ObjectBrowser::slotReportLoaded);
  connect(m_report, &ReportDesignWidget::activePageChanged, this, &ObjectBrowser::slotActivePageChanged);

  connect(m_report, &ReportDesignWidget::itemAdded, this, &ObjectBrowser::slotItemAdded);
  connect(m_report, &ReportDesignWidget::itemDeleted, this, &ObjectBrowser::slotItemDeleted);
  connect(m_report, &ReportDesignWidget::bandAdded, this, &ObjectBrowser::slotBandAdded);
  connect(m_report, &ReportDesignWidget::bandDeleted, this, &ObjectBrowser::slotBandDeleted);
  connect(m_treeView, &QTreeWidget::itemSelectionChanged, this, &ObjectBrowser::slotObjectTreeItemSelectionChanged);
  connect(m_report, &ReportDesignWidget::itemSelected, this, &ObjectBrowser::slotItemSelected);
  connect(m_report, &ReportDesignWidget::multiItemSelected, this, &ObjectBrowser::slotMultiItemSelected);
  connect(m_report, &ReportDesignWidget::activePageUpdated, this, &ObjectBrowser::slotActivePageUpdated);
  connect(m_treeView, &QTreeWidget::itemDoubleClicked, this, &ObjectBrowser::slotItemDoubleClicked);

  buildTree();
}

void ObjectBrowser::setMainWindow(QMainWindow *mainWindow) { m_mainWindow = mainWindow; }

void ObjectBrowser::slotClear() {}

void ObjectBrowser::fillNode(QTreeWidgetItem *parentNode, BaseDesignIntf *reportItem, BaseDesignIntf *ignoredItem) {
  foreach (BaseDesignIntf *item, reportItem->childBaseItems()) {
    if (item != ignoredItem) {
      ObjectBrowserNode *treeItem = new ObjectBrowserNode(parentNode);
      treeItem->setText(0, item->objectName());
      treeItem->setObject(item);
      treeItem->setIcon(0, QIcon(":/items/" + extractClassName(item->metaObject()->className())));
      connect(item, &BaseDesignIntf::propertyObjectNameChanged, this, &ObjectBrowser::slotPropertyObjectNameChanged);
      m_itemsMap.insert(item, treeItem);
      parentNode->addChild(treeItem);
      if (!item->childBaseItems().isEmpty()) fillNode(treeItem, item, ignoredItem);
    }
  }
}

void ObjectBrowser::buildTree(BaseDesignIntf *ignoredItem) {

  m_treeView->clear();
  m_itemsMap.clear();

  ObjectBrowserNode *topLevelItem = new ObjectBrowserNode(m_treeView);
  topLevelItem->setText(0, m_report->activePage()->objectName());
  topLevelItem->setObject(m_report->activePage());
  m_itemsMap.insert(m_report->activePage(), topLevelItem);

  m_treeView->addTopLevelItem(topLevelItem);
  QList<QGraphicsItem *> itemsList = m_report->activePage()->items();
  foreach (QGraphicsItem *item, itemsList) {
    if (item != ignoredItem) {
      BaseDesignIntf *reportItem = dynamic_cast<BaseDesignIntf *>(item);
      if (reportItem and reportItem->parentItem() == 0) {
        ObjectBrowserNode *tItem = new ObjectBrowserNode(topLevelItem);
        tItem->setText(0, reportItem->objectName());
        tItem->setObject(reportItem);
        tItem->setIcon(0, QIcon(":/items/" + extractClassName(reportItem->metaObject()->className())));
        connect(reportItem, &BaseDesignIntf::propertyObjectNameChanged, this,
                &ObjectBrowser::slotPropertyObjectNameChanged);
        m_itemsMap.insert(reportItem, tItem);
        fillNode(tItem, reportItem, ignoredItem);
        topLevelItem->addChild(tItem);
      }
    }
  }
  m_treeView->sortItems(0, Qt::AscendingOrder);
  m_treeView->expandAll();
}

void ObjectBrowser::findAndRemove(QTreeWidgetItem *node, BaseDesignIntf *item) {

  for (int i = 0; i < node->childCount(); i++) {
    QTreeWidgetItem *treeItem = node->child(i);
    if (treeItem->text(0) == item->objectName()) {
      node->removeChild(treeItem);
      break;
    } else {
      if (treeItem->childCount() > 0) findAndRemove(treeItem, item);
    }
  }
}

void ObjectBrowser::slotPropertyObjectNameChanged(const QString &oldName, const QString &newName) {
  Q_UNUSED(oldName)
  if (m_itemsMap.contains(sender())) {
    m_itemsMap.value(sender())->setText(0, newName);
  }
}

// void ObjectBrowser::slotObjectNameChanged(const QString &objectName)
//{
//    if (m_itemsMap.contains(sender())){
//        m_itemsMap.value(sender())->setText(0,objectName);
//    }
//}

void ObjectBrowser::removeItem(BaseDesignIntf *item) { findAndRemove(m_treeView->topLevelItem(0), item); }

void ObjectBrowser::slotReportLoaded() { buildTree(); }

void ObjectBrowser::slotActivePageChanged() { buildTree(); }

void ObjectBrowser::slotBandAdded(LimeReport::PageDesignIntf *, BandDesignIntf *) { buildTree(); }

void ObjectBrowser::slotBandDeleted(PageDesignIntf *, BandDesignIntf *item) { buildTree(item); }

void ObjectBrowser::slotItemAdded(PageDesignIntf *page, BaseDesignIntf *) {
  if (!page->isUpdating()) buildTree();
}

void ObjectBrowser::slotItemDeleted(PageDesignIntf *, BaseDesignIntf *item) {
  if (dynamic_cast<LayoutDesignIntf *>(item)) {
    buildTree(item);
  } else {
    removeItem(item);
  }
}

void ObjectBrowser::slotObjectTreeItemSelectionChanged() {
  if (!m_changingItemSelection) {
    m_changingItemSelection = true;
    m_report->activePage()->clearSelection();
    foreach (QTreeWidgetItem *item, m_treeView->selectedItems()) {
      ObjectBrowserNode *tn = dynamic_cast<ObjectBrowserNode *>(item);
      if (tn) {
        BaseDesignIntf *si = dynamic_cast<BaseDesignIntf *>(tn->object());
        if (si) {
          m_report->activePage()->animateItem(si);
          si->setSelected(true);
        }
      }
    }
    m_changingItemSelection = false;
  }
}

void ObjectBrowser::slotItemSelected(LimeReport::BaseDesignIntf *item) {
  if (!m_changingItemSelection) {
    m_changingItemSelection = true;

    m_treeView->selectionModel()->clear();
    BaseDesignIntf *bg = dynamic_cast<BaseDesignIntf *>(item);
    if (bg) {
      if (m_itemsMap.value(bg)) m_itemsMap.value(bg)->setSelected(true);
    }

    m_changingItemSelection = false;
  }
}

void ObjectBrowser::slotMultiItemSelected() {
  if (!m_changingItemSelection) {
    m_changingItemSelection = true;

    m_treeView->selectionModel()->clear();

    foreach (QGraphicsItem *item, m_report->activePage()->selectedItems()) {
      BaseDesignIntf *bg = dynamic_cast<BaseDesignIntf *>(item);
      if (bg) {
        ObjectBrowserNode *node = m_itemsMap.value(bg);
        if (node) node->setSelected(true);
      }
    }

    m_changingItemSelection = false;
  }
}

void ObjectBrowser::slotItemDoubleClicked(QTreeWidgetItem *item, int) {
  ObjectBrowserNode *node = dynamic_cast<ObjectBrowserNode *>(item);
  if (node) {
    BaseDesignIntf *baseItem = dynamic_cast<BaseDesignIntf *>(node->object());
    if (baseItem) {
      baseItem->showEditorDialog();
    }
  }
}

void ObjectBrowser::slotActivePageUpdated(LimeReport::PageDesignIntf *) { buildTree(); }

void ObjectBrowserNode::setObject(QObject *value) { m_object = value; }

QObject *ObjectBrowserNode::object() const { return m_object; }

ObjectBrowserNode::ObjectBrowserNode(QTreeWidget *view) : QTreeWidgetItem(view), m_object(0) {}

ObjectBrowserNode::ObjectBrowserNode(QTreeWidgetItem *parent) : QTreeWidgetItem(parent), m_object(0) {}

bool ObjectBrowserNode::operator<(const QTreeWidgetItem &other) const {
  BandDesignIntf *band1 = dynamic_cast<BandDesignIntf *>(m_object);
  BandDesignIntf *band2 = dynamic_cast<BandDesignIntf *>(dynamic_cast<const ObjectBrowserNode &>(other).object());
  if (band1 and band2) return band1->bandIndex() < band2->bandIndex();
  return false;
}

} // namespace LimeReport
