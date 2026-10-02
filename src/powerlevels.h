// SPDX-License-Identifier: GPL-3.0-only
// Minimal power-level helpers (fork-only backport of Jan 2020 concepts).
//
// The vendored May 2019 lib has no RoomPowerLevelsEvent class, so power
// levels are read straight from the m.room.power_levels content JSON —
// the same approach the lib itself uses in Room::canSwitchVersions().
// Spec defaults: users/events default 0, state default 50.
#ifndef POWERLEVELS_H
#define POWERLEVELS_H

#include <QJsonObject>

namespace powerlevels {
inline int intOr(const QJsonObject& o, const QString& key, int fallback) {
  auto v = o.value(key);
  return v.isUndefined() || v.isNull() ? fallback : v.toInt(fallback);
}

inline int userLevel(const QJsonObject& content, const QString& userId) {
  const auto users = content.value(QStringLiteral("users")).toObject();
  if (users.contains(userId)) return users.value(userId).toInt();
  return intOr(content, QStringLiteral("users_default"), 0);
}

inline int usersDefault(const QJsonObject& content) {
  return intOr(content, QStringLiteral("users_default"), 0);
}

inline int requiredForEvent(const QJsonObject& content,
                            const QString& eventType) {
  const auto events = content.value(QStringLiteral("events")).toObject();
  if (events.contains(eventType)) return events.value(eventType).toInt();
  return intOr(content, QStringLiteral("events_default"), 0);
}

inline int requiredForState(const QJsonObject& content,
                            const QString& eventType) {
  const auto events = content.value(QStringLiteral("events")).toObject();
  if (events.contains(eventType)) return events.value(eventType).toInt();
  return intOr(content, QStringLiteral("state_default"), 50);
}

inline int inviteLevel(const QJsonObject& content) {
  return intOr(content, QStringLiteral("invite"), 0);
}
inline int kickLevel(const QJsonObject& content) {
  return intOr(content, QStringLiteral("kick"), 50);
}
inline int banLevel(const QJsonObject& content) {
  return intOr(content, QStringLiteral("ban"), 50);
}
inline int redactLevel(const QJsonObject& content) {
  return intOr(content, QStringLiteral("redact"), 50);
}

inline int highestExplicitLevel(const QJsonObject& content) {
  int highest = usersDefault(content);
  const auto users = content.value(QStringLiteral("users")).toObject();
  for (auto it = users.constBegin(); it != users.constEnd(); ++it) {
    if (it.value().toInt() > highest) highest = it.value().toInt();
  }
  return highest;
}
}  // namespace powerlevels

#endif  // POWERLEVELS_H
