#include "drawWidgets.h"
#include <QFont>
#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QtWidgets>

void drawGrid(QPrinter *printer, QPainter *painter, QWidget *widget, double koefRes_w, double koefRes_h, bool parentGeom) {
    QFont font(widget->font());
    font.setPointSize( font.pointSize()-1 );

    painter->setPen(Qt::black);
    QString txt;
    int flag;
    double cellHeight, cellWidth;
    float stX, stY;
    int left_;
    int top_;
    int width_;
    int height_;

    if (parentGeom) {
        left_   = widget->parentWidget()->geometry().x() * koefRes_w;
        top_    = widget->parentWidget()->geometry().y() * koefRes_h;
        width_  = widget->parentWidget()->geometry().width() * koefRes_w;
        height_ = widget->parentWidget()->geometry().height() * koefRes_h;
    } else {
        left_   = widget->geometry().x() * koefRes_w;
        top_    = widget->geometry().y() * koefRes_h;
        width_  = widget->geometry().width() * koefRes_w;
        height_ = widget->geometry().height() * koefRes_h;
    }

    QTableWidget *table = qobject_cast<QTableWidget *>(widget);
    //table->setItemPrototype();
    int hh = table->horizontalHeader()->height() * koefRes_h;

    //Заполняем заголовок
    stX = left_;
    stY = top_;
    font.setBold(true);
    painter->setFont(font);

    if (table->horizontalHeader()->isVisible()) {
        for (int x=0; x<table->columnCount(); x++) {
            cellWidth = table->columnWidth(x)*koefRes_w;
            cellHeight = hh;
            txt = "";
            flag = int(Qt::AlignHCenter | Qt::AlignVCenter);
            if (table->horizontalHeaderItem(x) != 0)
                txt = table->horizontalHeaderItem(x)->text();

            QRectF rect(stX, stY, cellWidth, cellHeight);
            QRectF rectTxt = rect;
            rectTxt.setLeft(rect.left()+5);
            rectTxt.setWidth(rect.width()-5);
            painter->drawRect(rect);
            painter->drawText(rectTxt, flag, txt);
            stX = stX + cellWidth;    //Расстояние от левого края таблицы до столбца
        }
    }
    font.setBold(false);
    painter->setFont(font);

    //Заполняем данные
    stY = top_+hh;
    for (int y=0; y<table->rowCount(); y++) {
        if (table->rowHeight(y) < 2)
            continue;

        if (stY + table->rowHeight(y)*koefRes_h > printer->pageRect().height()) {
            printer->newPage();
            stY = 0;
        }

        stX = left_;
        for (int x=0; x<table->columnCount(); x++) {
            if (table->columnWidth(x) < 2)
                continue;

            txt = "";
            if (table->item(y,x) != 0) {
                flag = table->item(y,x)->textAlignment();
                txt = table->item(y,x)->text();
            }

            cellWidth = table->columnWidth(x)*koefRes_w;
            if (table->columnSpan(y,x) == 2) {
                if (!txt.isEmpty())
                    cellWidth = (table->columnWidth(x) + table->columnWidth(x+1) )*koefRes_w;
                else {
                    stX = stX + cellWidth;    //Расстояние от левого края таблицы до столбца
                    continue;
                }
            }
            cellHeight = table->rowHeight(y)*koefRes_h;
            if (table->rowSpan(y,x) == 2) {
                if (!txt.isEmpty())
                    cellHeight = (table->rowHeight(y) + table->rowHeight(y+1) )*koefRes_h;
                else {
                    stX = stX + cellWidth;    //Расстояние от левого края таблицы до столбца
                    continue;
                }
            }

            QRectF rect(stX, stY, cellWidth, cellHeight);
            QRectF rectTxt = rect;
            rectTxt.setLeft(rect.left()+5);
            rectTxt.setWidth(rect.width()-5);
            painter->drawRect(rect);
            if (!txt.isEmpty())
                painter->drawText(rectTxt, flag, txt);
            cellWidth = table->columnWidth(x)*koefRes_w;
            cellHeight = table->rowHeight(y)*koefRes_h;
            stX = stX + cellWidth;    //Расстояние от левого края таблицы до столбца
        }
        stY = stY + cellHeight;        //Отступ строки по высоте от верхнего края
    }
}

void drawLabel(QPainter *painter, QWidget *widget, double koefRes_w, double koefRes_h, const QRect *geom) {
    int cor = QFontMetrics( widget->font() ).height() * koefRes_h;

    QFont font(widget->font());
    font.setPointSize( font.pointSize()-1 );

    painter->setFont(font);
    painter->setPen(Qt::black);
    int left_;
    int top_;
    int width_;
    int height_;

    if (geom) {
        left_   = geom->x() * koefRes_w;
        top_    = geom->y() * koefRes_h;
        width_  = geom->width() * koefRes_w;
        height_ = geom->height() * koefRes_h;
    } else {
        left_   = widget->geometry().x() * koefRes_w;
        top_    = widget->geometry().y() * koefRes_h;
        if (left_ < 10*koefRes_w && top_ < 10*koefRes_h) {
            left_   = widget->parentWidget()->geometry().x() * koefRes_w;
            top_    = widget->parentWidget()->geometry().y() * koefRes_h;
        }
        width_  = widget->geometry().width() * koefRes_w;
        height_ = widget->geometry().height() * koefRes_h;
    }

    QRect textRect(left_, top_-height_, width_, height_);
    textRect.translate(0, cor );

    QString txt;
    if (qobject_cast<QLabel *>(widget) != 0) {
        QLabel *lb = qobject_cast<QLabel *>(widget);
        txt = lb->text();
    }
    if (qobject_cast<QComboBox *>(widget) != 0) {
        QComboBox *lb = qobject_cast<QComboBox *>(widget);
        txt = lb->currentText();
    }
    if (qobject_cast<QLineEdit *>(widget) != 0) {
        QLineEdit *le = qobject_cast<QLineEdit *>(widget);
        txt = le->text();
        //painter->drawLine(left_,top_+height_-5,left_+width_,top_+height_-5);
    }
    if (qobject_cast<QDateEdit *>(widget) != 0) {
        QDateEdit *de = qobject_cast<QDateEdit *>(widget);
        txt = de->date().toString("dd.MM.yyyy");
    }

    QString styleSheet = widget->styleSheet();
    bool top = styleSheet.contains("border-top-color: grey;", Qt::CaseInsensitive);
    bool buttom = styleSheet.contains("border-bottom-color: grey;", Qt::CaseInsensitive);
    bool left = styleSheet.contains("border-left-color: grey;", Qt::CaseInsensitive);
    bool right = styleSheet.contains("border-right-color: grey;", Qt::CaseInsensitive);
    if (top) painter->drawLine(left_,textRect.topLeft().y(),left_+width_,textRect.topRight().y());
    if (buttom) painter->drawLine(left_,textRect.bottomLeft().y(),left_+width_,textRect.bottomRight().y());
    if (left) painter->drawLine(left_,textRect.topRight().y(),left_,textRect.bottomRight().y());
    if (right) painter->drawLine(left_+width_,textRect.topRight().y(),left_+width_,textRect.bottomRight().y());

    //if (left) painter->drawLine(left_,top_-5,left_,top_-5);

    //painter->drawText(left_, top_+height_/2, txt);
    //painter->drawText(left_, top_+height_, txt);
    //painter->drawRect(textRect);
    painter->drawStaticText(left_, top_-5, txt);
}
