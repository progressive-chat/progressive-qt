// SPDX-License-Identifier: GPL-3.0-only
// User directory search model, ported from Spectral (Dec 2019).
#include "userdirectorylistmodel.h"

UserDirectoryListModel::UserDirectoryListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void UserDirectoryListModel::setConnection(Connection* conn) {
  if (m_connection == conn) return;

  beginResetModel();

  m_limited = false;
  attempted = false;
  users.clear();

  if (m_connection) {
    m_connection->disconnect(this);
  }

  endResetModel();

  m_connection = conn;

  if (job) {
    job->abandon();
    job = nullptr;
  }

  emit connectionChanged();
  emit limitedChanged();
}

void UserDirectoryListModel::setKeyword(const QString& value) {
  if (m_keyword == value) return;

  m_keyword = value;

  m_limited = false;
  attempted = false;

  if (job) {
    job->abandon();
    job = nullptr;
  }

  emit keywordChanged();
  emit limitedChanged();
}

void UserDirectoryListModel::search(int count) {
  if (!m_connection || count < 1) return;

  if (job) {
    qDebug() << "UserDirectoryListModel: Other jobs running, ignore";
    return;
  }

  if (attempted) return;

  job = m_connection->callApi<SearchUserDirectoryJob>(m_keyword, count);

  connect(job, &BaseJob::finished, this, [=] {
    attempted = true;

    if (job->status() == BaseJob::Success) {
      auto res = job->results();

      this->beginResetModel();

      this->users = res;
      this->m_limited = job->limited();

      this->endResetModel();
    }

    this->job = nullptr;

    emit limitedChanged();
  });
}

QVariant UserDirectoryListModel::data(const QModelIndex& index,
                                      int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= users.count()) {
    qDebug() << "UserDirectoryListModel, something's wrong: index.row() >= "
                "users.count()";
    return {};
  }
  auto user = users.at(index.row());
  if (role == NameRole) {
    if (!user.displayName.isEmpty()) return user.displayName;
    if (!user.userId.isEmpty()) return user.userId;
    return "Unknown User";
  }
  if (role == AvatarRole) {
    if (user.avatarUrl.isEmpty()) return "";
    return user.avatarUrl.remove(0, 6);
  }
  if (role == UserIDRole) {
    return user.userId;
  }
  if (role == DirectChatsRole) {
    if (!m_connection) return {};

    auto userObj = m_connection->user(user.userId);
    auto directChats = m_connection->directChats();

    if (userObj && directChats.contains(userObj)) {
      auto directChatsForUser = directChats.values(userObj);
      if (!directChatsForUser.isEmpty()) {
        return QVariant::fromValue(directChatsForUser);
      }
    }
  }

  return {};
}

QHash<int, QByteArray> UserDirectoryListModel::roleNames() const {
  QHash<int, QByteArray> roles;

  roles[NameRole] = "name";
  roles[AvatarRole] = "avatar";
  roles[UserIDRole] = "userID";
  roles[DirectChatsRole] = "directChats";

  return roles;
}

int UserDirectoryListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;

  return users.count();
}
