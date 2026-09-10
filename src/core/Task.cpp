#include "core/Task.h"

// ---------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------

// New task: generate UUID + capture creation time now

// Item 13: default constructor for Q_DECLARE_METATYPE compatibility
Task::Task()
    : Task(QStringLiteral("Untitled"))
{}

Task::Task(QString title, QString description,
           Priority priority, QDate dueDate, Status status,
           QStringList tags)
    : m_id(QUuid::createUuid())
    , m_title(std::move(title))
    , m_description(std::move(description))
    , m_priority(priority)
    , m_dueDate(dueDate)
    , m_status(status)
    , m_tags(std::move(tags))
    , m_createdAt(QDateTime::currentDateTime())
    , m_modifiedAt(m_createdAt)
{}

// Load from JSON: restore all fields as-is (preserves original timestamps)
Task::Task(QUuid id, QString title, QString description,
           Priority priority, QDate dueDate, Status status,
           QStringList tags,
           QDateTime createdAt, QDateTime modifiedAt)
    : m_id(id)
    , m_title(std::move(title))
    , m_description(std::move(description))
    , m_priority(priority)
    , m_dueDate(dueDate)
    , m_status(status)
    , m_tags(std::move(tags))
    , m_createdAt(createdAt.isValid() ? createdAt : QDateTime::currentDateTime())
    , m_modifiedAt(modifiedAt.isValid() ? modifiedAt : m_createdAt)
{}

// ---------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------

QUuid       Task::id()          const { return m_id; }
QString     Task::title()       const { return m_title; }
QString     Task::description() const { return m_description; }
Task::Priority Task::priority() const { return m_priority; }
QDate       Task::dueDate()     const { return m_dueDate; }
Task::Status Task::status()     const { return m_status; }
QStringList Task::tags()        const { return m_tags; }
QDateTime   Task::createdAt()   const { return m_createdAt; }
QDateTime   Task::modifiedAt()  const { return m_modifiedAt; }

// ---------------------------------------------------------------------
// Setters -- each updates m_modifiedAt (Item 8)
// ---------------------------------------------------------------------

void Task::setTitle(QString title)
{
    m_title = std::move(title);
    m_modifiedAt = QDateTime::currentDateTime();
}

void Task::setDescription(QString description)
{
    m_description = std::move(description);
    m_modifiedAt = QDateTime::currentDateTime();
}

void Task::setPriority(Priority priority)
{
    m_priority = priority;
    m_modifiedAt = QDateTime::currentDateTime();
}

void Task::setDueDate(QDate date)
{
    m_dueDate = date;
    m_modifiedAt = QDateTime::currentDateTime();
}

void Task::setStatus(Status status)
{
    m_status = status;
    m_modifiedAt = QDateTime::currentDateTime();
}

void Task::setTags(QStringList tags)
{
    m_tags = std::move(tags);
    m_modifiedAt = QDateTime::currentDateTime();
}

// ---------------------------------------------------------------------
// Enum <--> String helpers (used by JsonStore)
// ---------------------------------------------------------------------

QString statusToString(Task::Status s)
{
    switch (s) {
        case Task::Status::ToDo:       return QStringLiteral("todo");
        case Task::Status::InProgress: return QStringLiteral("inprogress");
        case Task::Status::Done:       return QStringLiteral("done");
    }
    return QStringLiteral("todo");
}

Task::Status statusFromString(const QString &s)
{
    if (s == QStringLiteral("inprogress")) return Task::Status::InProgress;
    if (s == QStringLiteral("done"))       return Task::Status::Done;
    return Task::Status::ToDo;
}

QString priorityToString(Task::Priority p)
{
    switch (p) {
        case Task::Priority::Low:    return QStringLiteral("low");
        case Task::Priority::Medium: return QStringLiteral("medium");
        case Task::Priority::High:   return QStringLiteral("high");
    }
    return QStringLiteral("medium");
}

Task::Priority priorityFromString(const QString &s)
{
    if (s == QStringLiteral("low"))  return Task::Priority::Low;
    if (s == QStringLiteral("high")) return Task::Priority::High;
    return Task::Priority::Medium;
}
