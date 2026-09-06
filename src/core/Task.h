#pragma once

#include <QDate>
#include <QString>
#include <QUuid>

/**
 * Task -- core data model for a single board task.
 *
 * Design choices: Task is a plain value type (NOT a QObject).
 *   - It can be copied, stored in QList<Task>, and passed by value.
 *   - No parent-child ownership needed; Board owns the collection.
 *   - No Q_OBJECT, no signals/slots on this class.
 */
class Task
{
public:
    // ----------------------------------------------------------
    // Enums
    // ----------------------------------------------------------

    // Which column the task lives in (maps to the 3 board columns)
    enum class Status {
        ToDo,
        InProgress,
        Done
    };

    // Task importance -- used for visual colour coding and filtering
    enum class Priority {
        Low,
        Medium,
        High
    };

    // ----------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------

    // Create a new task with a fresh UUID (used when user clicks "Add")
    explicit Task(QString title,
                  QString description = {},
                  Priority priority   = Priority::Medium,
                  QDate    dueDate    = {},
                  Status   status     = Status::ToDo);

    // Reconstruct a task from stored data (used by JsonStore when loading)
    Task(QUuid id, QString title, QString description,
         Priority priority, QDate dueDate, Status status);

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------

    QUuid    id()          const;
    QString  title()       const;
    QString  description() const;
    Priority priority()   const;
    QDate    dueDate()     const;
    Status   status()      const;

    // ----------------------------------------------------------
    // Setters -- return void; Board is responsible for notifying UI
    // ----------------------------------------------------------

    void setTitle(QString title);
    void setDescription(QString description);
    void setPriority(Priority priority);
    void setDueDate(QDate date);
    void setStatus(Status status);

private:
    QUuid    m_id;
    QString  m_title;
    QString  m_description;
    Priority m_priority;
    QDate    m_dueDate;
    Status   m_status;
};

// ---------------------------------------------------------------------
// Helper free functions: convert enums <--> string for JSON serialisation
// ---------------------------------------------------------------------
QString        statusToString(Task::Status s);
Task::Status   statusFromString(const QString &s);

QString        priorityToString(Task::Priority p);
Task::Priority priorityFromString(const QString &s);
