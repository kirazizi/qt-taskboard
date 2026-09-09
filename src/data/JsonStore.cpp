#include "data/JsonStore.h"

#include "core/Board.h"
#include "core/Task.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// Internal helpers -- not exposed in header

static QJsonObject taskToJson(const Task &t)
{
    QJsonObject obj;
    obj[QStringLiteral("id")]          = t.id().toString(QUuid::WithoutBraces);
    obj[QStringLiteral("title")]       = t.title();
    obj[QStringLiteral("description")] = t.description();
    obj[QStringLiteral("priority")]    = priorityToString(t.priority());
    obj[QStringLiteral("dueDate")]     = t.dueDate().toString(Qt::ISODate);
    obj[QStringLiteral("status")]      = statusToString(t.status());

    // Item 7: tags as a JSON array of strings
    QJsonArray tagsArr;
    for (const QString &tag : t.tags())
        tagsArr.append(tag);
    obj[QStringLiteral("tags")] = tagsArr;

    // Item 8: timestamps stored in ISO 8601 format (includes timezone offset)
    obj[QStringLiteral("createdAt")]  = t.createdAt().toString(Qt::ISODate);
    obj[QStringLiteral("modifiedAt")] = t.modifiedAt().toString(Qt::ISODate);

    return obj;
}

static Task taskFromJson(const QJsonObject &obj)
{
    const QUuid    id       = QUuid::fromString(obj[QStringLiteral("id")].toString());
    const QString  title    = obj[QStringLiteral("title")].toString();
    const QString  desc     = obj[QStringLiteral("description")].toString();
    const auto     priority = priorityFromString(obj[QStringLiteral("priority")].toString());
    const QDate    dueDate  = QDate::fromString(obj[QStringLiteral("dueDate")].toString(), Qt::ISODate);
    const auto     status   = statusFromString(obj[QStringLiteral("status")].toString());

    // Item 7: reconstruct tags from JSON array
    QStringList tags;
    for (const QJsonValue &v : obj[QStringLiteral("tags")].toArray())
        tags.append(v.toString());

    // Item 8: restore timestamps (fall back gracefully if missing in old files)
    const QDateTime createdAt  = QDateTime::fromString(
        obj[QStringLiteral("createdAt")].toString(), Qt::ISODate);
    const QDateTime modifiedAt = QDateTime::fromString(
        obj[QStringLiteral("modifiedAt")].toString(), Qt::ISODate);

    return Task(id, title, desc, priority, dueDate, status,
                tags, createdAt, modifiedAt);
}

// Public API

bool JsonStore::save(const Board &board, const QString &filePath)
{
    QJsonArray array;
    for (const Task &t : board.tasks())
        array.append(taskToJson(t));

    QJsonObject root;
    root[QStringLiteral("tasks")] = array;

    QJsonDocument doc(root);
    QFileInfo(filePath).dir().mkpath(QStringLiteral("."));

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "JsonStore::save: cannot open" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool JsonStore::load(Board &board, const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        board.setTasks({});  // emits boardReset to cleanly initialize empty columns
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "JsonStore::load: cannot open" << filePath;
        return false;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JsonStore::load: parse error:" << err.errorString();
        return false;
    }

    const QJsonArray array = doc.object()[QStringLiteral("tasks")].toArray();
    QList<Task> tasks;
    for (const QJsonValue &v : array) {
        if (v.isObject())
            tasks.append(taskFromJson(v.toObject()));
    }

    board.setTasks(std::move(tasks));
    return true;
}
