#include "core/Task.h"

// ---------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------

Task::Task(QString title, QString description,
           Priority priority, QDate dueDate, Status status)
    : m_id(QUuid::createUuid())          // generate a fresh UUID
    , m_title(std::move(title))          // move avoids a string copy
    , m_description(std::move(description))
    , m_priority(priority)
    , m_dueDate(dueDate)
    , m_status(status)
{}

Task::Task(QUuid id, QString title, QString description,
           Priority priority, QDate dueDate, Status status)
    : m_id(id)                            // restore the original ID from JSON
    , m_title(std::move(title))
    , m_description(std::move(description))
    , m_priority(priority)
    , m_dueDate(dueDate)
    , m_status(status)
{}

// ---------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------

QUuid    Task::id()          const { return m_id; }
QString  Task::title()       const { return m_title; }
QString  Task::description() const { return m_description; }
Task::Priority Task::priority() const { return m_priority; }
QDate    Task::dueDate()     const { return m_dueDate; }
Task::Status Task::status()  const { return m_status; }

// ---------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------

void Task::setTitle(QString title)              { m_title = std::move(title); }
void Task::setDescription(QString description)  { m_description = std::move(description); }
void Task::setPriority(Priority priority)        { m_priority = priority; }
void Task::setDueDate(QDate date)               { m_dueDate = date; }
void Task::setStatus(Status status)             { m_status = status; }

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
    return Task::Status::ToDo;              // default / unknown
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
    return Task::Priority::Medium;           // default / unknown
}
