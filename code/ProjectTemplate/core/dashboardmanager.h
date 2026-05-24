#pragma once
#include "models/Task.h"
#include "models/Document.h"
#include "models/DocumentFactory.h"
#include "models/Pomodoro/pomodoroSessionRecord.h"
#include "infra/FileSystemStorage.h"
#include <QString>
#include <QJsonObject>
#include <unordered_map>

class DashboardManager : public QObject {
    Q_OBJECT
public:
    explicit DashboardManager(QObject* parent = nullptr);

    // Task management
    void addTask(const QString& title);
    void removeTask(const QString& id);
    Task* findTaskById(const QString& id);
    const std::vector<std::unique_ptr<Task>>& tasks() const { return tasks_; }

    // Document management
    void createDocument(const QString& taskId, const QString& name);
    void deleteDocument(const QString& docId, const QString& taskId);
    Document* findDocumentById(const QString& docId);
    const std::vector<std::unique_ptr<Document>>& documents() const { return documents_; }

    // Checklist management
    void addChecklistItem(const QString& taskId, const QString& text);
    void toggleChecklistItem(const QString& taskId, size_t index);
    void removeChecklistItem(const QString& taskId, size_t index);
    
    // Task completion and drawing
    void setTaskCompleted(const QString& taskId, bool completed);
    void setTaskDrawingData(const QString& taskId, const QString& data);

    // Session management
    void recordSession(const SessionRecord& session);
    int getTotalPomodoroSessions() const;
    int getTotalFocusMinutes() const;

    // Workspace state management
    void setCurrentTask(const QString& taskId);
    Task* getCurrentTask() const;
    void setCurrentDocument(const QString& docId);
    QString getCurrentDocumentId() const;
    
    // Task operations with automatic document creation
    void addTaskWithDocuments(const QString& title);
    
    // Statistics
    int getTotalTasks() const;
    int getCompletedTasks() const;
    int getTotalDocuments() const;

    // Persistence
    void loadFromJson();
    void saveToJson() const;

signals:
    void tasksUpdated();
    void sessionsUpdated();
    void documentsUpdated();
    void checklistUpdated(const QString& taskId);
    void currentTaskChanged(Task* task);
    void currentDocumentChanged(const QString& docId);

private:
    std::vector<std::unique_ptr<Task>> tasks_;
    std::vector<std::unique_ptr<Document>> documents_;
    std::unordered_map<std::string, std::vector<SessionRecord>> sessionsByTask_;
    std::unique_ptr<DocumentFactory> documentFactory_;
    
    // Current workspace state
    Task* currentTask_ = nullptr;
    QString currentDocumentId_;

    QString tasksFilePath_ = "data/tasks.json";
    QString sessionsFilePath_ = "data/sessions.json";
    QString documentsFilePath_ = "data/documents.json";

    FileSystemStorage storage_;

    // Serialization helpers
    QJsonObject taskToJson(const Task& task) const;
    std::unique_ptr<Task> taskFromJson(const QJsonObject& obj) const;
    QJsonObject documentToJson(const Document& doc) const;
    std::unique_ptr<Document> documentFromJson(const QJsonObject& obj) const;
};
