#pragma once

#include <QDateTime>
#include <QDate>
#include <QString>
#include <QStringList>
#include <QUuid>

/**
 * Task -- core data model for a single board task.
 *
 * Design choices: Task is a plain value type (NOT a QObject).
 *   - It can be copied, stored in QList<Task>, and passed by value.
 *   - No parent-child ownership needed; Board owns the collection.
 *   - No Q_OBJECT, no signals/slots on this class.
 *
 * Tier 2 additions:
 *   - m_tags:        QStringList of user-defined category tags (Item 7)
 *   - m_createdAt:   timestamp set once at construction (Item 8)
 *   - m_modifiedAt:  timestamp updated in every setter (Item 8)
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
                  Status   status     = Status::ToDo,
                  QStringList tags    = {});

    // Reconstruct a task from stored data (used by JsonStore when loading)
    Task(QUuid id, QString title, QString description,
         Priority priority, QDate dueDate, Status status,
         QStringList tags,
         QDateTime createdAt, QDateTime modifiedAt);

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------

    QUuid       id()          const;
    QString     title()       const;
    QString     description() const;
    Priority    priority()    const;
    QDate       dueDate()     const;
    Status      status()      const;
    QStringList tags()        const;  // Item 7
    QDateTime   createdAt()   const;  // Item 8
    QDateTime   modifiedAt()  const;  // Item 8

    // ----------------------------------------------------------
    // Setters -- each one also updates m_modifiedAt (Item 8)
    // ----------------------------------------------------------

    void setTitle(QString title);
    void setDescription(QString description);
    void setPriority(Priority priority);
    void setDueDate(QDate date);
    void setStatus(Status status);
    void setTags(QStringList tags);  // Item 7

private:
    QUuid       m_id;
    QString     m_title;
    QString     m_description;
    Priority    m_priority;
    QDate       m_dueDate;
    Status      m_status;
    QStringList m_tags;       // Item 7: categories/labels
    QDateTime   m_createdAt;  // Item 8: set once on construction
    QDateTime   m_modifiedAt; // Item 8: updated on every mutation
};

// ---------------------------------------------------------------------
// Helper free functions: convert enums <--> string for JSON serialisation
// ---------------------------------------------------------------------
QString        statusToString(Task::Status s);
Task::Status   statusFromString(const QString &s);

QString        priorityToString(Task::Priority p);
Task::Priority priorityFromString(const QString &s);
