#include "lrpreviewreportwidget.h"
#include "ui_lrpreviewreportwidget.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QScrollBar>
#include <QFileDialog>

#include "lrpagedesignintf.h"
#include "lrreportrender.h"
#include "lrreportengine_p.h"
#include "lrpreviewreportwidget_p.h"
#include "serializators/lrxmlwriter.h"

namespace LimeReport {

bool PreviewReportWidgetPrivate::pageIsVisible(){
    QGraphicsView* view = q_ptr->ui->graphicsView;
	if ( m_currentPage-1 >= m_reportPages.size() || m_currentPage <= 0 )
        return false;
    PageItemDesignIntf::Ptr page = m_reportPages.at(m_currentPage-1);
    return page->mapToScene(page->rect()).boundingRect().intersects(
                view->mapToScene(view->viewport()->geometry()).boundingRect()
                );
}

QRectF PreviewReportWidgetPrivate::calcPageShift(){
    QGraphicsView *view = q_ptr->ui->graphicsView;
    PageItemDesignIntf::Ptr page = m_reportPages.at(m_currentPage-1);
    qreal pageHeight = page->mapToScene(page->boundingRect()).boundingRect().height();
    qreal viewHeight = view->mapToScene(
                0, view->viewport()->height()
                ).y() - view->mapToScene(0,0).y();
    viewHeight = (pageHeight<viewHeight)?pageHeight:viewHeight;
    QRectF pageStartPos =  m_reportPages.at(m_currentPage-1)->mapRectToScene(
                m_reportPages.at(m_currentPage-1)->rect()
                );
    return QRectF(0,pageStartPos.y(),0,viewHeight);
}

void PreviewReportWidgetPrivate::setPages(ReportPages pages)
{
    m_reportPages = pages;
    if (!m_reportPages.isEmpty()){
        m_previewPage->setPageItems(m_reportPages);
        m_changingPage = true;
        m_currentPage = 1;
        if (pages.at(0)) pages.at(0)->setSelected(true);
        m_changingPage = false;
        q_ptr->initPreview();
        q_ptr->emitPageSet();
    }
}

PageItemDesignIntf::Ptr PreviewReportWidgetPrivate::currentPage()
{
    if (m_reportPages.count()>0 && m_reportPages.count()>=m_currentPage)
        return m_reportPages.at(m_currentPage-1);
    else return PageItemDesignIntf::Ptr(nullptr);
}

PreviewReportWidget::PreviewReportWidget(ReportEnginePrivate *report, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreviewReportWidget), d_ptr(new PreviewReportWidgetPrivate(this))
{
    ui->setupUi(this);
    d_ptr->m_previewPage = report->createPreviewPage();
    d_ptr->m_previewPage->setItemMode( LimeReport::PreviewMode );
    d_ptr->m_report = report;

    ui->errorsView->setVisible(false);
    connect(ui->graphicsView->verticalScrollBar(),SIGNAL(valueChanged(int)), this, SLOT(slotSliderMoved(int)));
    connect(d_ptr->m_report, SIGNAL(destroyed(QObject*)), this, SLOT(reportEngineDestroyed(QObject*)));
    d_ptr->m_zoomer = new GraphicsViewZoomer(ui->graphicsView);
    connect(d_ptr->m_zoomer, SIGNAL(zoomed(double)), this, SLOT(slotZoomed(double)));
}

PreviewReportWidget::~PreviewReportWidget()
{
    delete d_ptr->m_previewPage;
    d_ptr->m_previewPage = nullptr;
    delete d_ptr->m_zoomer;
    delete d_ptr;
    delete ui;
}

void PreviewReportWidget::initPreview()
{
    if (ui->graphicsView->scene()!=d_ptr->m_previewPage)
        ui->graphicsView->setScene(d_ptr->m_previewPage);
    ui->graphicsView->resetMatrix();
    ui->graphicsView->centerOn(0, 0);
    setScalePercent(d_ptr->m_scalePercent);
}

void PreviewReportWidget::setErrorsMesagesVisible(bool visible)
{
    ui->errorsView->setVisible(visible);
}

void PreviewReportWidget::zoomIn()
{
    d_ptr->m_scalePercent =  (d_ptr->m_scalePercent / 10) * 10 + 10;
    setScalePercent(d_ptr->m_scalePercent);
}

void PreviewReportWidget::zoomOut()
{
    if (d_ptr->m_scalePercent>0)
        d_ptr->m_scalePercent = (d_ptr->m_scalePercent / 10) * 10 - 10;
    setScalePercent(d_ptr->m_scalePercent);
}

void PreviewReportWidget::firstPage()
{
    d_ptr->m_changingPage=true;
    if ((!d_ptr->m_reportPages.isEmpty())&&(d_ptr->m_currentPage>1)){
        d_ptr->m_currentPage=1;
        ui->graphicsView->ensureVisible(d_ptr->calcPageShift(), 0, 0);
        emit pageChanged(d_ptr->m_currentPage);
    }
    d_ptr->m_changingPage=false;
}

void PreviewReportWidget::priorPage()
{
    d_ptr->m_changingPage=true;
    if ((!d_ptr->m_reportPages.isEmpty())&&(d_ptr->m_currentPage>1)){
       d_ptr->m_currentPage--;
       ui->graphicsView->ensureVisible(d_ptr->calcPageShift(), 0, 0);
       emit pageChanged(d_ptr->m_currentPage);
    }
   d_ptr->m_changingPage=false;
}

void PreviewReportWidget::nextPage()
{
    d_ptr->m_changingPage=true;
    if ((!d_ptr->m_reportPages.isEmpty())&&(d_ptr->m_reportPages.count()>(d_ptr->m_currentPage))){
        d_ptr->m_currentPage++;
        ui->graphicsView->ensureVisible(d_ptr->calcPageShift(), 0, 0);
        emit pageChanged(d_ptr->m_currentPage);
    }
    d_ptr->m_changingPage=false;
}

void PreviewReportWidget::lastPage()
{
    d_ptr->m_changingPage=true;
    if ((!d_ptr->m_reportPages.isEmpty())&&(d_ptr->m_reportPages.count()>(d_ptr->m_currentPage))){
        d_ptr->m_currentPage=d_ptr->m_reportPages.count();
        ui->graphicsView->ensureVisible(d_ptr->calcPageShift(), 0, 0);
        emit pageChanged(d_ptr->m_currentPage);
    }
    d_ptr->m_changingPage=false;
}

void PreviewReportWidget::print()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer,QApplication::activeWindow());
    if (dialog.exec()==QDialog::Accepted){
        if (!d_ptr->m_reportPages.isEmpty())
            ReportEnginePrivate::printReport(
                d_ptr->m_reportPages,
                printer
            );
        foreach(PageItemDesignIntf::Ptr pageItem, d_ptr->m_reportPages){
            d_ptr->m_previewPage->reactivatePageItem(pageItem);
        }
    }
}

void PreviewReportWidget::printToPDF()
{
    QString filter = "PDF (*.pdf)";
    QString fileName = QFileDialog::getSaveFileName(this,tr("PDF file name"),"","PDF (*.pdf)");
    if (!fileName.isEmpty()){
        QFileInfo fi(fileName);
        if (fi.suffix().isEmpty())
            fileName+=".pdf";
        QPrinter printer;
        printer.setOutputFileName(fileName);
        printer.setOutputFormat(QPrinter::PdfFormat);
        if (!d_ptr->m_reportPages.isEmpty()){
            ReportEnginePrivate::printReport(d_ptr->m_reportPages,printer);
        }
        foreach(PageItemDesignIntf::Ptr pageItem, d_ptr->m_reportPages){
            d_ptr->m_previewPage->reactivatePageItem(pageItem);
        }
    }
}

void PreviewReportWidget::pageNavigatorChanged(int value)
{
    if (d_ptr->m_changingPage) return;
    d_ptr->m_changingPage = true;
    if ((!d_ptr->m_reportPages.isEmpty())&&(d_ptr->m_reportPages.count() >= value) && value>0){
        d_ptr->m_currentPage = value;
        ui->graphicsView->ensureVisible(d_ptr->calcPageShift(), 0, 0);
    }
    d_ptr->m_changingPage=false;
}

void PreviewReportWidget::saveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Report file name"));
    if (!fileName.isEmpty()){
        QScopedPointer< ItemsWriterIntf > writer(new XMLWriter());
        foreach (PageItemDesignIntf::Ptr page, d_ptr->m_reportPages){
            writer->putItem(page.data());
        }
        writer->saveToFile(fileName);
    }
}

void PreviewReportWidget::setScalePercent(int percent)
{
    ui->graphicsView->resetMatrix();
    d_ptr->m_scalePercent = percent;
    qreal scaleSize = percent/100.0;
    ui->graphicsView->scale(scaleSize, scaleSize);
    emit scalePercentChanged(percent);
}

void PreviewReportWidget::fitWidth()
{
    if (d_ptr->currentPage()){
        qreal scalePercent = ui->graphicsView->viewport()->width() / ui->graphicsView->scene()->width();
        setScalePercent(scalePercent*100);
    }
}

void PreviewReportWidget::fitPage()
{
    if (d_ptr->currentPage()){
        qreal vScale = ui->graphicsView->viewport()->width() / ui->graphicsView->scene()->width();
        qreal hScale = ui->graphicsView->viewport()->height() / d_ptr->currentPage()->height();
        setScalePercent(qMin(vScale,hScale)*100);
    }
}

void PreviewReportWidget::setErrorMessages(const QStringList &value)
{
    foreach (QString line, value) {
        ui->errorsView->append(line);
    }
}

void PreviewReportWidget::emitPageSet()
{
    emit pagesSet(d_ptr->m_reportPages.count());
}

void PreviewReportWidget::refreshPages()
{
    if (d_ptr->m_report){
        try{
            d_ptr->m_report->dataManager()->setDesignTime(false);
            ReportPages pages = d_ptr->m_report->renderToPages();
            d_ptr->m_report->dataManager()->setDesignTime(true);
            if (pages.count()>0){
                d_ptr->setPages(pages);
            }
        } catch (ReportError &exception){
            d_ptr->m_report->saveError(exception.what());
            d_ptr->m_report->showError(exception.what());
       }
    }
}

void PreviewReportWidget::slotSliderMoved(int value)
{
    if (ui->graphicsView->verticalScrollBar()->minimum()==value){
        d_ptr->m_currentPage = 1;
    } else if (ui->graphicsView->verticalScrollBar()->maximum()==value){
        d_ptr->m_currentPage = d_ptr->m_reportPages.count();
    }

    if (!d_ptr->pageIsVisible()){
        if (value>d_ptr->m_priorScrolValue){
            d_ptr->m_currentPage++;
        } else {
            d_ptr->m_currentPage--;
        }
    }

    d_ptr->m_changingPage = true;
    emit pageChanged(d_ptr->m_currentPage);

    d_ptr->m_changingPage = false;
    d_ptr->m_priorScrolValue = value;
}

void PreviewReportWidget::reportEngineDestroyed(QObject *object)
{
    if (object == d_ptr->m_report){
        d_ptr->m_report = nullptr;
    }
}

void PreviewReportWidget::slotZoomed(double )
{
    d_ptr->m_scalePercent = ui->graphicsView->matrix().m11()*100;
    emit scalePercentChanged(d_ptr->m_scalePercent);
}



}
