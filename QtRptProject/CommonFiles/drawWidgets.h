#ifndef DRAWWIDGETS_H
#define DRAWWIDGETS_H

#include <QtGui>
#include <QPrinter>

void drawGrid(QPrinter *printer, QPainter *painter, QWidget *widget, double koefRes_w, double koefRes_h, bool parentGeom);
void drawLabel(QPainter *painter, QWidget *widget, double koefRes_w, double koefRes_h, const QRect *geom);

#endif // DRAWWIDGETS_H
