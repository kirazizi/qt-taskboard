#include "data/JsonStore.h"

#include "core/Board.h"
#include "core/Task.h"

#include <QDebug>
#include <QFile>
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
    return Task(id, title, desc, priority, dueDate, status);
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
    if (!file.exists())
        return true;  // first run -- no save file yet

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
