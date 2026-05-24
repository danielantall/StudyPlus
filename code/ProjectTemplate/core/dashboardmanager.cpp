#include "dashboardmanager.h"
#include <QJsonArray>
#include <QJsonObject>
#include "core/models/TextDocumentFactory.h"
#include <QJsonDocument>
#include <QUuid>
#include <QMessageBox>

/**
 * @brief Constructs a DashboardManager responsible for managing tasks, documents, and sessions.
 * @param parent The QObject parent.
 *
 * Ensures the data directory exists, initializes the document factory,
 * and loads previously saved tasks, documents, and sessions from JSON.
 */
DashboardManager::DashboardManager(QObject* parent)
    : QObject(parent)
{
    storage_.ensureDir("data");
    documentFactory_ = std::make_unique<TextDocumentFactory>();
    loadFromJson();
}

// === Task Management ===

/**
 * @brief Adds a new task to the dashboard.
 * @param title The name of the task.
 *
 * Creates a new Task with a UUID and saves it to persistent storage.
 */
void DashboardManager::addTask(const QString& title) {
    auto id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    tasks_.push_back(std::make_unique<Task>(id, title.toStdString()));
    saveToJson();
    emit tasksUpdated();
}

/**
 * @brief Adds a new task with support for documents.
 * @param title The title of the task.
 */
void DashboardManager::addTaskWithDocuments(const QString& title) {
    auto taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto task = std::make_unique<Task>(taskId.toStdString(), title.toStdString());
    tasks_.push_back(std::move(task));
    saveToJson();
    emit tasksUpdated();
}

/**
 * @brief Removes a task and all its related documents.
 * @param id The ID of the task to remove.
 */
void DashboardManager::removeTask(const QString& id) {
    std::string taskId = id.toStdString();

    // Remove documents associated with this task
    documents_.erase(
        std::remove_if(documents_.begin(), documents_.end(), [&taskId](const auto& doc) {
            return doc->id().find(taskId) == 0;
        }),
        documents_.end()
        );

    // Remove task entry
    auto it = std::remove_if(tasks_.begin(), tasks_.end(), [&](const auto& t) {
        return t->id() == taskId;
    });
    tasks_.erase(it, tasks_.end());

    saveToJson();
    emit tasksUpdated();
    emit documentsUpdated();
}

/**
 * @brief Finds a task by ID.
 * @param id Task ID.
 * @return Pointer to the Task or nullptr if not found.
 */
Task* DashboardManager::findTaskById(const QString& id) {
    std::string taskId = id.toStdString();
    for (const auto& task : tasks_) {
        if (task->id() == taskId) return task.get();
    }
    return nullptr;
}

// === Document Management ===

/**
 * @brief Creates a new document for a specific task.
 * @param taskId ID of the task that owns the document.
 * @param name Name of the new document.
 */
void DashboardManager::createDocument(const QString& taskId, const QString& name) {
    std::string docId = taskId.toStdString() + "_doc_" + std::to_string(documents_.size() + 1);

    auto document = documentFactory_->createDocument(docId, name.toStdString());
    document->setContent("# " + name + "\n\nStart writing here...");

    Task* task = findTaskById(taskId);
    if (task) task->addDocumentId(docId);

    documents_.push_back(std::move(document));
    saveToJson();
    emit documentsUpdated();
    emit tasksUpdated();
}

/**
 * @brief Deletes a document and removes its reference from its task.
 * @param docId The document ID.
 * @param taskId The parent task ID.
 */
void DashboardManager::deleteDocument(const QString& docId, const QString& taskId) {
    std::string documentId = docId.toStdString();
    documents_.erase(
        std::remove_if(documents_.begin(), documents_.end(), [&documentId](const auto& doc) {
            return doc->id() == documentId;
        }),
        documents_.end()
        );

    Task* task = findTaskById(taskId);
    if (task) task->removeDocumentId(documentId);

    saveToJson();
    emit documentsUpdated();
}

/**
 * @brief Finds a document by its ID.
 * @param docId The document's ID.
 * @return Pointer to the Document or nullptr if not found.
 */
Document* DashboardManager::findDocumentById(const QString& docId) {
    std::string documentId = docId.toStdString();
    for (const auto& doc : documents_) {
        if (doc->id() == documentId) return doc.get();
    }
    return nullptr;
}

// === Checklist Management ===

/**
 * @brief Adds a checklist item to a task.
 * @param taskId Task ID.
 * @param text Text of the checklist item.
 */
void DashboardManager::addChecklistItem(const QString& taskId, const QString& text) {
    if (auto* task = findTaskById(taskId)) {
        task->addChecklistItem(text.toStdString());
        saveToJson();
        emit checklistUpdated(taskId);
    }
}

/**
 * @brief Toggles a checklist item’s completion state.
 * @param taskId Task ID.
 * @param index Index of the item in the checklist.
 */
void DashboardManager::toggleChecklistItem(const QString& taskId, size_t index) {
    if (auto* task = findTaskById(taskId)) {
        task->toggleChecklistItem(index);
        saveToJson();
        emit checklistUpdated(taskId);
    }
}

/**
 * @brief Removes a checklist item from a task.
 * @param taskId Task ID.
 * @param index Index of the item.
 */
void DashboardManager::removeChecklistItem(const QString& taskId, size_t index) {
    if (auto* task = findTaskById(taskId)) {
        task->removeChecklistItem(index);
        saveToJson();
        emit checklistUpdated(taskId);
    }
}

/**
 * @brief Sets the completion status of a task.
 * @param taskId Task ID.
 * @param completed Whether the task is completed.
 */
void DashboardManager::setTaskCompleted(const QString& taskId, bool completed) {
    if (auto* task = findTaskById(taskId)) {
        task->setCompleted(completed);
        saveToJson();
        emit tasksUpdated();
    }
}

/**
 * @brief Sets the drawing data for a task.
 * @param taskId Task ID.
 * @param data The drawing data.
 */
void DashboardManager::setTaskDrawingData(const QString& taskId, const QString& data) {
    if (auto* task = findTaskById(taskId)) {
        task->setDrawingData(data.toStdString());
        saveToJson();
        emit tasksUpdated();
    }
}

// === Pomodoro Session Management ===

/**
 * @brief Records a completed Pomodoro session linked to a task.
 * @param session The session record containing intervals and metadata.
 */
void DashboardManager::recordSession(const SessionRecord& session) {
    sessionsByTask_[session.taskId].push_back(session);
    saveToJson();
    emit sessionsUpdated();
}

// === Statistics ===

/**
 * @brief Gets the total number of tasks.
 * @return Task count.
 */
int DashboardManager::getTotalTasks() const { return static_cast<int>(tasks_.size()); }

/**
 * @brief Gets the total number of completed tasks.
 * @return Completed task count.
 */
int DashboardManager::getCompletedTasks() const {
    int count = 0;
    for (const auto& task : tasks_) {
        if (task->isCompleted()) count++;
    }
    return count;
}

/**
 * @brief Gets the total number of stored documents.
 * @return Document count.
 */
int DashboardManager::getTotalDocuments() const { return static_cast<int>(documents_.size()); }

/**
 * @brief Gets the total number of recorded Pomodoro sessions.
 * @return Session count.
 */
int DashboardManager::getTotalPomodoroSessions() const {
    int total = 0;
    for (const auto& [taskId, sessions] : sessionsByTask_) total += (int)sessions.size();
    return total;
}

/**
 * @brief Calculates total focus minutes across all tasks.
 * @return Sum of focus durations.
 */
int DashboardManager::getTotalFocusMinutes() const {
    int total = 0;
    for (const auto& [taskId, sessions] : sessionsByTask_) {
        for (const auto& session : sessions) total += session.getTotalFocusMinutes();
    }
    return total;
}

// === Workspace State ===

/**
 * @brief Sets the currently active task.
 * @param taskId ID of the task to activate.
 */
void DashboardManager::setCurrentTask(const QString& taskId) {
    Task* task = findTaskById(taskId);
    if (task != currentTask_) {
        currentTask_ = task;
        currentDocumentId_.clear();
        emit currentTaskChanged(currentTask_);
    }
}

/**
 * @brief Gets the currently active task.
 * @return Pointer to Task or nullptr.
 */
Task* DashboardManager::getCurrentTask() const { return currentTask_; }

/**
 * @brief Sets the current active document.
 * @param docId Document ID.
 */
void DashboardManager::setCurrentDocument(const QString& docId) {
    if (currentDocumentId_ != docId) {
        currentDocumentId_ = docId;
        emit currentDocumentChanged(currentDocumentId_);
    }
}

/**
 * @brief Gets the ID of the currently active document.
 * @return Current document ID as QString.
 */
QString DashboardManager::getCurrentDocumentId() const { return currentDocumentId_; }

// === JSON Persistence ===

/**
 * @brief Saves all tasks, documents, and sessions to JSON files.
 */
void DashboardManager::saveToJson() const {
    // Serialize tasks
    QJsonArray taskArray;
    for (const auto& task : tasks_) taskArray.append(taskToJson(*task));
    storage_.writeText(tasksFilePath_, QString(QJsonDocument(taskArray).toJson()));

    // Serialize documents
    QJsonArray documentArray;
    for (const auto& doc : documents_) documentArray.append(documentToJson(*doc));
    storage_.writeText(documentsFilePath_, QString(QJsonDocument(documentArray).toJson()));

    // Serialize sessions
    QJsonArray sessionArray;
    for (const auto& [taskId, sessions] : sessionsByTask_) {
        for (const auto& s : sessions) {
            QJsonObject obj;
            obj["taskId"] = QString::fromStdString(s.taskId);
            obj["startTime"] = s.startTime.toString(Qt::ISODate);
            obj["endTime"] = s.endTime.toString(Qt::ISODate);

            QJsonArray intervalsArr;
            for (const auto& i : s.intervals) {
                QJsonObject io;
                io["type"] = static_cast<int>(i.type);
                io["durationMinutes"] = i.durationMinutes;
                intervalsArr.append(io);
            }
            obj["intervals"] = intervalsArr;
            sessionArray.append(obj);
        }
    }
    storage_.writeText(sessionsFilePath_, QString(QJsonDocument(sessionArray).toJson()));
}

/**
 * @brief Serializes document into JSON.
 * @param doc The document to be serialized
 */
QJsonObject DashboardManager::documentToJson(const Document& doc) const {
    QJsonObject obj;
    obj["id"] = QString::fromStdString(doc.id());
    obj["name"] = QString::fromStdString(doc.name());
    obj["content"] = doc.content();
    return obj;
}

/**
 * @brief Deserializes document from JSON.
 * @param doc The document to be deserialized
 */
std::unique_ptr<Document> DashboardManager::documentFromJson(const QJsonObject& obj) const {
    auto id = obj["id"].toString().toStdString();
    auto name = obj["name"].toString().toStdString();
    auto document = documentFactory_->createDocument(id,name);
    document->setContent(obj["content"].toString());
    return document;
}

/**
 * @brief Generate task from JSON.
 * @param obj The JSON object to be deserialized
 */
std::unique_ptr<Task> DashboardManager::taskFromJson(const QJsonObject& obj) const {
    auto id = obj["id"].toString().toStdString();
    auto title = obj["title"].toString().toStdString();
    std::optional<QDate> deadline;
    if (obj.contains("deadline")) deadline = QDate::fromString(obj["deadline"].toString(), Qt::ISODate);

    auto task = std::make_unique<Task>(id, title, deadline);
    
    // Load completion status
    if (obj.contains("completed")) {
        task->setCompleted(obj["completed"].toBool());
    }
    
    // Load drawing data
    if (obj.contains("drawingData")) {
        task->setDrawingData(obj["drawingData"].toString().toStdString());
    }
    
    // Load document IDs
    if (obj.contains("documentIds")) {
        QJsonArray docArray = obj["documentIds"].toArray();
        for (const auto& docVal : docArray) {
            task->addDocumentId(docVal.toString().toStdString());
        }
    }
    // Load checklist items
    if (obj.contains("checklist")) {
        QJsonArray checklistArray = obj["checklist"].toArray();
        for (const auto& itemVal : checklistArray) {
            QJsonObject itemObj = itemVal.toObject();
            task->addChecklistItem(itemObj["text"].toString().toStdString());
            // If the item was completed, toggle it
            if (itemObj["completed"].toBool()) {
                size_t lastIndex = task->getChecklistItems().size() - 1;
                task->toggleChecklistItem(lastIndex);
            }
        }
    }
    return task;
}

/**
 * @brief Generate JSON from Task.
 * @param t The Task to be converted
 */
QJsonObject DashboardManager::taskToJson(const Task& t) const {
    QJsonObject obj; obj["id"] = QString::fromStdString(t.id());
    obj["title"] = QString::fromStdString(t.title());
    if (t.deadline()) obj["deadline"] = t.deadline()->toString(Qt::ISODate);
    obj["completed"] = t.isCompleted();
    if (!t.getDrawingData().empty()) {
        obj["drawingData"] = QString::fromStdString(t.getDrawingData());
    }

    // Add document IDs
    QJsonArray docArray;
    for (const auto& docId : t.documentIds()) {
        docArray.append(QString::fromStdString(docId));
    }

    obj["documentIds"] = docArray;

    // Add checklist items
    QJsonArray checklistArray;
    for (const auto& item : t.getChecklistItems()) {
        QJsonObject checklistItem;
        checklistItem["text"] = QString::fromStdString(item.text);
        checklistItem["completed"] = item.completed;
        checklistArray.append(checklistItem);
    }

    obj["checklist"] = checklistArray;
    return obj;
}

/**
 * @brief Loads all stored data (tasks, documents, sessions) from JSON files.
 */
void DashboardManager::loadFromJson() {
    // Load tasks
    auto taskData = storage_.readText(tasksFilePath_);
    if (taskData) {
        QJsonDocument doc = QJsonDocument::fromJson(taskData->toUtf8());
        if (doc.isArray()) {
            for (const auto& val : doc.array())
                if (val.isObject()) tasks_.push_back(taskFromJson(val.toObject()));
        }
    }

    // Load documents
    auto docData = storage_.readText(documentsFilePath_);
    if (docData) {
        QJsonDocument doc = QJsonDocument::fromJson(docData->toUtf8());
        if (doc.isArray()) {
            for (const auto& val : doc.array())
                if (val.isObject()) documents_.push_back(documentFromJson(val.toObject()));
        }
    }
}
