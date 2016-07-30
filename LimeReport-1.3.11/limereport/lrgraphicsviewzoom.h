#ifndef GRAPHICSVIEWZOOM_H
#define GRAPHICSVIEWZOOM_H

#include <QGraphicsView>
#include <QObject>

namespace LimeReport {

class GraphicsViewZoomer : public QObject {
  Q_OBJECT
public:
  explicit GraphicsViewZoomer(QGraphicsView *view);
  void gentleZoom(double factor);
  void setModifiers(Qt::KeyboardModifiers modifiers);
  void setZoomFactorBase(double value);

private:
  QGraphicsView *m_view;
  Qt::KeyboardModifiers m_modifiers;
  double m_zoomFactorBase;
  QPointF m_targetScenePos, m_targetViewportPos;
  bool eventFilter(QObject *object, QEvent *event);
signals:
  void zoomed(double factor);
};

} // namespace LimeReport

#endif // GRAPHICSVIEWZOOM_H
