# Real Qt 5.6 toolchain

The Qt 5.6 port is verified with a real Qt 5.6.3 built from source
(`qt-everywhere-opensource-src-5.6.3`), not just API review. This file
documents how to reproduce it. No root needed.

## Layout (on the build machine)

- `/home/user/qt56-toolchain/` — toolchain root (persistent disk, NOT /tmp)
  - `qt-5.6.3.tar.xz` — upstream tarball (`download.qt.io/archive/qt/5.6/5.6.3/single/`)
  - `qt-everywhere-opensource-src-5.6.3/` — extracted sources + 2 small patches (below)
  - `build-qt56.sh` — configure+build+install script used
  - `install/` — the toolchain (`install/bin/qmake` reports `5.6.3`)
  - `app-build/` — Progressive Chat Qt built with that qmake
  - `qmltest/` — headless QML harness + farm tests (persistent re-creation
    of the /tmp scratch tests)
  - `build.log`, `install.log`, `app-build.log` — full logs

## Reproduce

```sh
# 1. fetch + extract
curl -o qt-5.6.3.tar.xz https://download.qt.io/archive/qt/5.6/5.6.3/single/qt-everywhere-opensource-src-5.6.3.tar.xz
tar -xf qt-5.6.3.tar.xz
# 2. configure + build + install (COMPAT/toolchain-build.sh)
./configure -prefix $PREFIX -release -opensource -confirm-license \
  -c++std c++11 -no-openssl -no-icu \
  -no-xcb -no-eglfs -no-directfb -no-kms \
  -nomake examples -nomake tests \
  -skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcanvas3d \
  -skip qtconnectivity -skip qtdoc -skip qtenginio -skip qtimageformats \
  -skip qtlocation -skip qtmacextras -skip qtmultimedia \
  -skip qtscript -skip qtsensors -skip qtserialport -skip qtsvg \
  -skip qtquickcontrols2 -skip qttools -skip qtwayland -skip qtwebchannel \
  -skip qtwebengine -skip qtwebview -skip qtwebsockets \
  -skip qtwinextras -skip qtx11extras -skip qtxmlpatterns
make -j$(nproc)
make install
# 3. build the app
$PREFIX/bin/qmake progressive-qt.pro && make -j$(nproc)
# 4. run headless (needs a platform plugin; offscreen/minimal ship by default)
QT_QPA_PLATFORM=offscreen ./progressive-chat
```

Notes:

- Host compiler here is GCC 16 (aarch64); `-c++std c++11` keeps the 2017
  codebase compiling. Two Qt-source patches were needed (kept in the source
  tree, both documented at the patch site):
  - `qtbase/qmake/generators/makefile.cpp`: `requires` → `pkgconfigRequires`
    (`requires` is a C++20 keyword).
  - configure `-no-icu`: system ICU is too new for Qt 5.6's `qicucodec`
    (iconv fallback is used instead).
- If you re-run `configure` in an existing tree, delete stale PCH first:
  `find . -type d -name .pch -exec rm -rf {} +`.
- QtMultimedia is skipped on purpose: the app compiles without it
  (`PROGRESSIVE_NO_MULTIMEDIA`, audio playback becomes a warning).
  Same for OpenSSL (`-no-openssl`; `QT_NO_SSL` guards in the vendored lib).
- `make install` provides real QtQuick.Controls 1.4 / Styles /
  QtGraphicalEffects / `Qt.labs.settings` 1.0 — no `Qt.labs.platform`
  on 5.6 (tray icon stays `Loader`-guarded by design).

## Verification results (this toolchain)

- `qmake progressive-qt.pro && make` — clean, `progressive-chat` links.
- App startup headless: all QML loads (login/room/setting pages, dialogs,
  menus, drawer), models init, `Model timeline reset` reached. First paint
  needs GL (absent on headless CI) — identical for any QtQuick app.
- QML farm tests (delegates, room/account delegates, component kit) under
  the real 5.6 QML engine — zero errors/warnings.
- Bugs the real toolchain caught (all fixed in repo): missing
  `QDateTime`/`QPainterPath` includes, `QT_NO_SSL` guards,
  `qmlWarning`/`qmlInfo` shims for Qt < 5.9, `QImage::byteCount` fallback,
  `typingChanged` NOTIFY re-declaration, direct `<QObject>` includes for
  moc, `LocalStorage.transaction()` V4 crash (settings use
  `Qt.labs.settings`, which 5.6 ships), CheckBox binding inversion,
  TextArea width loop, null `currentRoom` guard.

## Android note

This host toolchain proves source compatibility with Qt 5.6. APKs are built
on x86_64 CI (`.github/workflows/android.yml`): Qt 5.6.3 for Android
(armeabi-v7a, API 14) + NDK r10e + SDK android-14, ant + JDK 8, then
`androiddeployqt --deployment bundled`. A local APK build on aarch64 dev
boxes is not supported (NDK r10e host tools are x86_64-only, no qemu).
`android/AndroidManifest.xml` declares minSdk 9 / targetSdk 14.
