// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 https://progressive.chat contributors
// Based on Spectral (GPL-3.0). See README.md.
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#ifdef Q_OS_ANDROID
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#endif

#include "accountlistmodel.h"
#include "controller.h"
#include "emojimodel.h"
#include "imageitem.h"
#include "imageprovider.h"
#include "messageeventmodel.h"
#include "publicroomlistmodel.h"
#include "userdirectorylistmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "spectralroom.h"
#include "spectraluser.h"
#include "userlistmodel.h"

#include "csapi/joining.h"
#include "csapi/leaving.h"

#include "qqmlsortfilterproxymodel.h"

using namespace QMatrixClient;

#ifdef Q_OS_ANDROID
namespace {
// On-device startup log: tries external files dir first (accessible without
// root on Android 5+), falls back to app's cache dir. Readable with any
// file manager, no adb needed. Captures Qt/QML warnings plus explicit
// stage markers so a silent native crash still leaves a trace.
QString progressiveLogPath() {
  // Try external files dir first (accessible without root on Android 5+)
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (path.isEmpty()) {
    path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  }
  path += QStringLiteral("/progressive-chat.log");
  return path;
}
void progressiveLogRaw(const QString& line) {
  QFile f(progressiveLogPath());
  if (f.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream out(&f);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << line
        << "\n";
  }
}
void progressiveMessageHandler(QtMsgType, const QMessageLogContext&,
                               const QString& msg) {
  progressiveLogRaw(msg);
  fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}
}  // namespace
#define PROGRESSIVE_STAGE(m) progressiveLogRaw(QStringLiteral("stage: ") + QStringLiteral(m))
#else
#define PROGRESSIVE_STAGE(m) ((void)0)
#endif

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_ANDROID
  // Earliest possible stage marker - before QApplication
  PROGRESSIVE_STAGE("pre-QApplication");
#endif

  QApplication app(argc, argv);

#ifdef Q_OS_ANDROID
  qInstallMessageHandler(progressiveMessageHandler);
#endif
  PROGRESSIVE_STAGE("app created");

  app.setOrganizationName("Progressive Chat");
  app.setOrganizationDomain("progressive.chat");
  app.setApplicationName("Progressive Chat");
  app.setWindowIcon(QIcon(":/assets/img/icon.png"));

  qmlRegisterType<qqsfpm::QQmlSortFilterProxyModel>("SortFilterProxyModel", 0,
                                                    2, "SortFilterProxyModel");
  qmlRegisterType<ImageItem>("Progressive", 0, 1, "ImageItem");
  qmlRegisterType<Controller>("Progressive", 0, 1, "Controller");
  qmlRegisterType<AccountListModel>("Progressive", 0, 1, "AccountListModel");
  qmlRegisterType<RoomListModel>("Progressive", 0, 1, "RoomListModel");
  qmlRegisterType<UserListModel>("Progressive", 0, 1, "UserListModel");
  qmlRegisterType<PublicRoomListModel>("Progressive", 0, 1,
                                       "PublicRoomListModel");
  qmlRegisterType<UserDirectoryListModel>("Progressive", 0, 1,
                                          "UserDirectoryListModel");
  qmlRegisterType<MessageEventModel>("Progressive", 0, 1, "MessageEventModel");
  qmlRegisterType<EmojiModel>("Progressive", 0, 1, "EmojiModel");
  qmlRegisterUncreatableType<RoomMessageEvent>("Progressive", 0, 1,
                                               "RoomMessageEvent", "ENUM");
  qmlRegisterUncreatableType<RoomType>("Progressive", 0, 1, "RoomType", "ENUM");

  qRegisterMetaType<User *>("User*");
  qRegisterMetaType<Room *>("Room*");
  qRegisterMetaType<MessageEventType>("MessageEventType");
  qRegisterMetaType<SpectralRoom *>("SpectralRoom*");
    qRegisterMetaType<SpectralUser *>("SpectralUser*");

  QQmlApplicationEngine engine;

  engine.addImportPath("qrc:/imports");
  ImageProvider *m_provider = new ImageProvider();
  engine.rootContext()->setContextProperty("imageProvider", m_provider);
  // Qt 5.6 compat: QML StandardPaths lives in Qt.labs.platform (5.8+),
  // so expose the cache location from C++ instead. See DownloadableContent.qml.
  engine.rootContext()->setContextProperty(
      "cacheLocation", QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  PROGRESSIVE_STAGE("qml loaded");
  if (engine.rootObjects().isEmpty()) {
    PROGRESSIVE_STAGE("no root objects, exiting");
    return -1;
  }

  PROGRESSIVE_STAGE("entering event loop");
  return app.exec();
}
