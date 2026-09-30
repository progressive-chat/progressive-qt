// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 https://progressive.chat contributors
// Based on Spectral (GPL-3.0). See README.md.
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>

#include "accountlistmodel.h"
#include "controller.h"
#include "emojimodel.h"
#include "imageitem.h"
#include "imageprovider.h"
#include "messageeventmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "spectralroom.h"
#include "spectraluser.h"
#include "userlistmodel.h"

#include "csapi/joining.h"
#include "csapi/leaving.h"

#include "qqmlsortfilterproxymodel.h"

using namespace QMatrixClient;

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);

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
  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
