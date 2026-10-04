// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 https://progressive.chat contributors
// Based on Spectral (GPL-3.0). See README.md.
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QKeyEvent>
#include <QQuickWindow>
#include <QQmlContext>
#include <QStandardPaths>
#ifdef Q_OS_ANDROID
#include <QDateTime>
#include <QDir>
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

#ifdef PROGRESSIVE_STATIC_QT
// Static Qt build: import the QML/platform plugins explicitly (no dlopen).
#include <QtPlugin>
Q_IMPORT_PLUGIN(QMinimalIntegrationPlugin)
Q_IMPORT_PLUGIN(QOffscreenIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickControlsPlugin)
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
Q_IMPORT_PLUGIN(QtQmlModelsPlugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPlugin)
Q_IMPORT_PLUGIN(QmlSettingsPlugin)
#endif
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

// FORK-ONLY: print a native backtrace on fatal signals.
//
// Crashes on exotic/old hardware are otherwise reported as a bare
// "Segmentation fault (core dumped)" with no clue where they come from.
// This writes a plain-text backtrace to stderr (and to the on-device log
// file, if one is writable) before re-raising with the default handler, so
// a user report contains actionable information. No allocation is done in
// the handler (backtrace_symbols_fd only writes to an fd).
#include <csignal>
#include <unistd.h>

// Android/Bionic has neither execinfo.h nor backtrace(), so the handler is
// desktop-only. The release APK still reports crashes via logcat.
#if !defined(Q_OS_ANDROID) && defined(__linux__)
#include <execinfo.h>
#define PROGRESSIVE_HAVE_BACKTRACE 1
#endif

extern "C" void progressiveFatalSignalHandler(int sig)
{
#ifdef PROGRESSIVE_HAVE_BACKTRACE
  void* frames[64];
  const int n = ::backtrace(frames, 64);
  // Marker so it is greppable in the crash output.
  static const char msg[] = "\n=== progressive-chat: fatal signal, backtrace follows ===\n";
  const ssize_t ignored = ::write(STDERR_FILENO, msg, sizeof(msg) - 1);
  Q_UNUSED(ignored);
  ::backtrace_symbols_fd(frames, n, STDERR_FILENO);
  static const char msg2[] = "=== end backtrace ===\n";
  const ssize_t ignored2 = ::write(STDERR_FILENO, msg2, sizeof(msg2) - 1);
  Q_UNUSED(ignored2);
#endif

  // Restore the default action and re-raise, so the exit status and any
  // core dump behave exactly as they would have without this handler.
  struct sigaction dfl;
  dfl.sa_handler = SIG_DFL;
  sigemptyset(&dfl.sa_mask);
  dfl.sa_flags = 0;
  ::sigaction(sig, &dfl, nullptr);
  ::raise(sig);
}

static void progressiveInstallCrashHandler()
{
#ifdef PROGRESSIVE_HAVE_BACKTRACE
  // A stack-overflow SIGSEGV cannot be reported from the handler itself:
  // the guard page is already hit, so the handler has no stack to run on
  // and the process dies silently. Give the handler its own stack via
  // sigaltstack() + SA_ONSTACK - this is what makes QML binding loops
  // (i.e. infinite recursion) visible instead of an unexplained segfault.
  // 256 KiB: SIGSTKSZ is not a compile-time constant on modern glibc.
  static char altStack[256 * 1024];
  stack_t ss;
  ss.ss_sp = altStack;
  ss.ss_size = sizeof(altStack);
  ss.ss_flags = 0;
  if (sigaltstack(&ss, nullptr) != 0) {
    static const char msg[] =
        "\n=== progressive-chat: sigaltstack() failed; stack-overflow "
        "crashes will not be reportable ===\n";
    const ssize_t ignored = ::write(STDERR_FILENO, msg, sizeof(msg) - 1);
    Q_UNUSED(ignored);
  }
#endif

  // NOTE: do NOT name this "signals" - Qt defines that as a macro.
  const int fatalSignals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
  struct sigaction sa;
  sa.sa_handler = progressiveFatalSignalHandler;
  sigemptyset(&sa.sa_mask);
  // No SA_RESETHARD: we reset it ourselves before re-raising.
  sa.sa_flags = SA_ONSTACK;
  for (int sig : fatalSignals) ::sigaction(sig, &sa, nullptr);
}

// FORK-ONLY: Ctrl+Shift+S saves a PNG of the main window and prints the
// path. Minimal/embedded systems often have no screenshot tool installed,
// and bug reports are far more useful with a picture. Also honours
// PROGRESSIVE_SCREENSHOT=/path/file.png to capture once after startup.
static QString progressiveSaveScreenshot(QQuickWindow* window)
{
  if (!window) return {};
  const QImage shot = window->grabWindow();
  if (shot.isNull()) {
    qWarning() << "Screenshot failed (grabWindow returned a null image)";
    return {};
  }
  // Stamped name so several shots do not overwrite each other.
  const QString stamp =
      QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss"));
  // Default to the system temp directory (/tmp on Linux) rather than the
  // current working directory: on a phone/desktop the CWD is often the
  // user's home or a read-only location, and /tmp is where a screenshot
  // is easiest to find from a terminal.
  QDir dir(QStringLiteral("."));
  if (qEnvironmentVariableIsSet("PROGRESSIVE_SCREENSHOT_DIR")) {
    dir = QDir(QString::fromLocal8Bit(qgetenv("PROGRESSIVE_SCREENSHOT_DIR")));
  } else {
    dir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
  }
  if (!dir.exists())
    dir.mkpath(QStringLiteral("."));
  const QString path =
      dir.filePath(QStringLiteral("progressive-chat-%1.png").arg(stamp));
  if (!shot.save(path, "PNG")) {
    qWarning() << "Screenshot failed: cannot write" << path;
    return {};
  }
  qWarning().noquote() << "Screenshot saved:" << QFileInfo(path).absoluteFilePath();
  return path;
}

// Ctrl+Shift+S key filter. A QShortcut would need a QWidget parent, and a
// QQuickWindow is not one, so match the key press ourselves.
class ProgressiveScreenshotKeyFilter : public QObject
{
  public:
    explicit ProgressiveScreenshotKeyFilter(QQuickWindow* window)
        : m_window(window)
    {
        window->installEventFilter(this);
    }

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(event);
            const bool ctrl = key->modifiers().testFlag(Qt::ControlModifier);
            const bool shift = key->modifiers().testFlag(Qt::ShiftModifier);
            if (ctrl && shift && key->key() == Qt::Key_S) {
                progressiveSaveScreenshot(m_window);
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }

  private:
    QQuickWindow* m_window;
};

static void progressiveInstallScreenshotShortcut(QQuickWindow* window)
{
  if (!window) return;
  new ProgressiveScreenshotKeyFilter(window);
}

int main(int argc, char *argv[]) {
  progressiveInstallCrashHandler();
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
#ifdef PROGRESSIVE_STATIC_QT
  // Static Qt build: QML modules are embedded in resources (see static-qml
  // handling in progressive-qt.pro).
  engine.addImportPath("qrc:/qt56qml");
#endif
  ImageProvider *m_provider = new ImageProvider();
  engine.rootContext()->setContextProperty("imageProvider", m_provider);
  // Qt 5.6 compat: QML StandardPaths lives in Qt.labs.platform (5.8+),
  // so expose the cache location from C++ instead. See DownloadableContent.qml.
  engine.rootContext()->setContextProperty(
      "cacheLocation", QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  // FORK-ONLY: PROGRESSIVE_DEBUG_DELEGATE=1 makes the timeline delegate log
  // the roles it actually received. This is the only reliable way to see
  // what a delegate gets on a device we cannot inspect.
  engine.rootContext()->setContextProperty(
      "progressiveDebugDelegate",
      qEnvironmentVariableIsSet("PROGRESSIVE_DEBUG_DELEGATE"));
  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  PROGRESSIVE_STAGE("qml loaded");
  if (engine.rootObjects().isEmpty()) {
    PROGRESSIVE_STAGE("no root objects, exiting");
    return -1;
  }

  PROGRESSIVE_STAGE("entering event loop");

  // FORK-ONLY: in-app screenshot (Ctrl+Shift+S).
  if (auto* window =
          qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
    progressiveInstallScreenshotShortcut(window);
    if (qEnvironmentVariableIsSet("PROGRESSIVE_SCREENSHOT")) {
      // Capture once the first frame is on screen.
      QTimer::singleShot(4000, window,
                         [window] { progressiveSaveScreenshot(window); });
    }
  }

  return app.exec();
}
