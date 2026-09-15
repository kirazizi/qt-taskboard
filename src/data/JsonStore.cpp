#include "data/JsonStore.h"

#include "core/Board.h"
#include "core/BoardManager.h"
#include "core/Task.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// ── Internal helpers ─────────────────────────────────────────────────────────

static QJsonObject taskToJson(const Task &t)
{
    QJsonObject obj;
    obj[QStringLiteral("id")]          = t.id().toString(QUuid::WithoutBraces);
    obj[QStringLiteral("title")]       = t.title();
    obj[QStringLiteral("description")] = t.description();
    obj[QStringLiteral("priority")]    = priorityToString(t.priority());
    obj[QStringLiteral("dueDate")]     = t.dueDate().toString(Qt::ISODate);
    obj[QStringLiteral("status")]      = statusToString(t.status());

    QJsonArray tagsArr;
    for (const QString &tag : t.tags())
        tagsArr.append(tag);
    obj[QStringLiteral("tags")] = tagsArr;

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

    QStringList tags;
    for (const QJsonValue &v : obj[QStringLiteral("tags")].toArray())
        tags.append(v.toString());

    const QDateTime createdAt  = QDateTime::fromString(
        obj[QStringLiteral("createdAt")].toString(), Qt::ISODate);
    const QDateTime modifiedAt = QDateTime::fromString(
        obj[QStringLiteral("modifiedAt")].toString(), Qt::ISODate);

    return Task(id, title, desc, priority, dueDate, status,
                tags, createdAt, modifiedAt);
}

static QJsonArray boardTasksToJson(const Board &board)
{
    QJsonArray array;
    for (const Task &t : board.tasks())
        array.append(taskToJson(t));
    return array;
}

// ── Single-board API ─────────────────────────────────────────────────────────

bool JsonStore::save(const Board &board, const QString &filePath)
{
    QJsonObject root;
    root[QStringLiteral("tasks")] = boardTasksToJson(board);
    QJsonDocument doc(root);
    QFileInfo(filePath).dir().mkpath(QStringLiteral("."));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "JsonStore::save(Board): cannot open" << filePath;
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool JsonStore::load(Board &board, const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        board.setTasks({});
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "JsonStore::load(Board): cannot open" << filePath;
        return false;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JsonStore::load(Board): parse error:" << err.errorString();
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

// ── Multi-board API (Tier 3 Item 11) ─────────────────────────────────────────

bool JsonStore::save(const BoardManager &manager, const QString &filePath)
{
    QJsonArray boardsArr;
    for (int i = 0; i < manager.count(); ++i) {
        const BoardMeta meta = manager.metaAt(i);
        const Board *board   = manager.boardAt(i);
        QJsonObject entry;
        entry[QStringLiteral("id")]    = meta.id.toString(QUuid::WithoutBraces);
        entry[QStringLiteral("name")]  = meta.name;
        entry[QStringLiteral("tasks")] = board ? boardTasksToJson(*board) : QJsonArray{};
        boardsArr.append(entry);
    }

    QJsonObject root;
    root[QStringLiteral("version")] = 2;
    root[QStringLiteral("boards")]  = boardsArr;

    QJsonDocument doc(root);
    QFileInfo(filePath).dir().mkpath(QStringLiteral("."));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "JsonStore::save(BoardManager): cannot open" << filePath;
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool JsonStore::load(BoardManager &manager, const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        // Fresh start: BoardManager already has one default board
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "JsonStore::load(BoardManager): cannot open" << filePath;
        return false;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JsonStore::load(BoardManager): parse error:" << err.errorString();
        return false;
    }

    const QJsonObject root = doc.object();
    const int version      = root[QStringLiteral("version")].toInt(1);

    QList<BoardMeta>      metas;
    QList<QList<Task>>    taskLists;

    if (version >= 2) {
        // Multi-board format
        for (const QJsonValue &bv : root[QStringLiteral("boards")].toArray()) {
            const QJsonObject bobj = bv.toObject();
            BoardMeta meta;
            meta.id   = QUuid::fromString(bobj[QStringLiteral("id")].toString());
            meta.name = bobj[QStringLiteral("name")].toString(QStringLiteral("Board"));
            metas.append(meta);

            QList<Task> tasks;
            for (const QJsonValue &tv : bobj[QStringLiteral("tasks")].toArray()) {
                if (tv.isObject())
                    tasks.append(taskFromJson(tv.toObject()));
            }
            taskLists.append(tasks);
        }
    } else {
        // Version 1 (Tier 1/2 single-board) -- auto-migrate
        BoardMeta meta;
        meta.id   = QUuid::createUuid();
        meta.name = QStringLiteral("My Board");
        metas.append(meta);

        QList<Task> tasks;
        for (const QJsonValue &v : root[QStringLiteral("tasks")].toArray()) {
            if (v.isObject())
                tasks.append(taskFromJson(v.toObject()));
        }
        taskLists.append(tasks);
    }

    manager.setBoardsFromLoad(metas, taskLists);
    return true;
}
