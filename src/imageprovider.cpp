#include "imageprovider.h"

#include <QFile>
#include <QMetaObject>
#include <QStandardPaths>
#include <QtCore/QDebug>
#include <QtCore/QWaitCondition>

#include "jobs/mediathumbnailjob.h"

#include "connection.h"

using QMatrixClient::MediaThumbnailJob;

ImageProvider::ImageProvider(QObject* parent)
    : QObject(parent),
      QQuickImageProvider(
          QQmlImageProviderBase::Image,
          QQmlImageProviderBase::ForceAsynchronousImageLoading) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
  qRegisterMetaType<MediaThumbnailJob*>();
#endif
}

QImage ImageProvider::requestImage(const QString& id, QSize* pSize,
                                   const QSize& requestedSize) {
  // FORK-ONLY: the id does NOT still carry the scheme.
  //
  // The provider is registered under the `mxc` scheme (main.cpp:
  // engine.addImageProvider("mxc", ...)), so Qt strips `image://mxc/` before
  // calling us and hands over the bare remainder - "t2l.io/0e92cc..." for an
  // avatar. The old guard required the id to *start with* "mxc://", so it
  // rejected every single request, logged "doesn't follow server/mediaId
  // pattern" for all of them, and returned an empty image. The public room
  // directory then showed no avatars at all and its list stopped responding
  // to clicks.
  //
  // What a valid id looks like is exactly "serverName/localMediaId" - two
  // parts, because Connection::getThumbnail() splits on '/' and asserts two
  // (silently taking the wrong parts in a release build). A bare host name is
  // not resolvable and a three-part path is not a media id. Accept an explicit
  // mxc:// prefix too, in case a caller ever hands one over unstripped.
  QString mxcId = id;
  if (mxcId.startsWith("mxc://")) mxcId = mxcId.mid(6);
  if (mxcId.count('/') != 1 || mxcId.startsWith('/') || mxcId.endsWith('/')) {
    qWarning() << "ImageProvider: won't fetch an invalid id:" << id
               << "doesn't follow server/mediaId pattern";
    return {};
  }

  // The scheme is stripped by the time we get here (see above), and
  // getThumbnail() wants "mxc://server/mediaId", so rebuild it from the
  // validated id rather than from the raw argument.
  QUrl mxcUri{"mxc://" + mxcId};

  QUrl tempfilePath = QUrl::fromLocalFile(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" +
      mxcUri.fileName() + "-" + QString::number(requestedSize.width()) + "x" +
      QString::number(requestedSize.height()) + ".png");

  QImage cachedImage;
  if (cachedImage.load(tempfilePath.toLocalFile())) {
    if (pSize != nullptr) *pSize = cachedImage.size();
    return cachedImage;
  }

  MediaThumbnailJob* job = nullptr;
  QReadLocker locker(&m_lock);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  QMetaObject::invokeMethod(
      m_connection,
      [=] { return m_connection->getThumbnail(mxcUri, requestedSize); },
      Qt::BlockingQueuedConnection, &job);
#else
  QMetaObject::invokeMethod(m_connection, "getThumbnail",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(MediaThumbnailJob*, job),
                            Q_ARG(QUrl, mxcUri), Q_ARG(QSize, requestedSize));
#endif
  if (!job) {
    qDebug() << "ImageProvider: failed to send a request";
    return {};
  }
  QImage result;
  {
    QWaitCondition condition;  // The most compact way to block on a signal
    job->connect(job, &MediaThumbnailJob::finished, job, [&] {
      result = job->thumbnail();
      condition.wakeAll();
    });
    condition.wait(&m_lock);
  }

  if (pSize != nullptr) *pSize = result.size();

  result.save(tempfilePath.toLocalFile());

  return result;
}
