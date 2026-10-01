// SPDX-License-Identifier: GPL-3.0-only
// Android stub: Qt for Android has no D-Bus. Notifications degrade to a
// debug message; in-app banners remain the user-visible path.
#include "manager.h"

#include <QDebug>

NotificationsManager::NotificationsManager(QObject* parent)
    : QObject(parent) {}

void NotificationsManager::postNotification(const QString& roomId,
                                            const QString& eventId,
                                            const QString& roomName,
                                            const QString& senderName,
                                            const QString& text,
                                            const QImage& icon,
                                            const QUrl& iconPath) {
  Q_UNUSED(roomId);
  Q_UNUSED(eventId);
  Q_UNUSED(roomName);
  Q_UNUSED(senderName);
  Q_UNUSED(text);
  Q_UNUSED(icon);
  Q_UNUSED(iconPath);
  qDebug() << "NotificationsManager: D-Bus unavailable on Android,"
              "notification skipped.";
}

void NotificationsManager::actionInvoked(uint id, QString action) {
  Q_UNUSED(id);
  Q_UNUSED(action);
}

void NotificationsManager::notificationClosed(uint id, uint reason) {
  Q_UNUSED(id);
  Q_UNUSED(reason);
}
