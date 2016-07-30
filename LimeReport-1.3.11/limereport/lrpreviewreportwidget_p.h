#ifndef LRPREVIEWREPORTWIDGET_P_H
#define LRPREVIEWREPORTWIDGET_P_H

#include "lrgraphicsviewzoom.h"
#include "lrpagedesignintf.h"
#include "lrreportrender.h"

namespace LimeReport {

class PreviewReportWidget;

class PreviewReportWidgetPrivate {
public:
  explicit PreviewReportWidgetPrivate(PreviewReportWidget *previewReportWidget)
      : m_currentPage(1), m_changingPage(false), m_priorScrolValue(0), m_scalePercent(50), q_ptr(previewReportWidget) {}
  bool pageIsVisible();
  QRectF calcPageShift();
  void setPages(ReportPages pages);
  PageItemDesignIntf::Ptr currentPage();

public:
  PageDesignIntf *m_previewPage;
  ReportPages m_reportPages;
  ReportEnginePrivate *m_report;
  GraphicsViewZoomer *m_zoomer;
  int m_currentPage;
  bool m_changingPage;
  int m_priorScrolValue;
  int m_scalePercent;
  PreviewReportWidget *q_ptr;
};
}
#endif // LRPREVIEWREPORTWIDGET_P_H
