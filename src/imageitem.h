#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QImage>
#include <QObject>
#include <QVariant>
#include <QPainter>
#include <QQuickItem>
#include <QQuickPaintedItem>

class ImageItem : public QQuickPaintedItem {
  Q_OBJECT
  // FORK-ONLY: QVariant rather than QImage. With a QImage-typed property
  // QML warns "Unable to assign QString/null to QImage" whenever a row has
  // no avatar yet (null) or hands us an mxc id (QString). The setter
  // validates and stores an empty image instead.
  Q_PROPERTY(QVariant image READ image WRITE setImage NOTIFY imageChanged)
  Q_PROPERTY(QString hint READ hint WRITE setHint NOTIFY hintChanged)
  Q_PROPERTY(QString defaultColor READ defaultColor WRITE setDefaultColor NOTIFY
                 defaultColorChanged)
  Q_PROPERTY(bool round READ round WRITE setRound NOTIFY roundChanged)

 public:
  ImageItem(QQuickItem *parent = nullptr);

  void paint(QPainter *painter);

  QVariant image() const { return QVariant::fromValue(m_image); }
  void setImage(const QVariant &image);

  QString hint() { return m_hint; }
  void setHint(QString hint);

  QString defaultColor() { return m_color; }
  void setDefaultColor(QString color);

  bool round() { return m_round; }
  void setRound(bool value);

 signals:
  void imageChanged();
  void hintChanged();
  void defaultColorChanged();
  void roundChanged();

 private:
  QImage m_image;
  QString m_hint = "H";
  QString m_color;
  bool m_round = true;

  QString stringtoColor(QString string);
};

#endif  // IMAGEITEM_H
