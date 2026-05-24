#include "Task.h"
#include <algorithm>

/**
 * @brief Constructs a Task with an ID, title, and optional deadline.
 * @param id Unique identifier for the task.
 * @param title Title or name of the task.
 * @param deadline Optional due date for the task.
 */
Task::Task(std::string id, std::string title, std::optional<QDate> deadline)
    : id_(std::move(id)), title_(std::move(title)), deadline_(std::move(deadline)) {}

/**
 * @brief Updates the title of the task.
 * @param t New title string.
 */
void Task::setTitle(std::string t) {
    title_ = std::move(t);
}

/**
 * @brief Sets or clears the deadline for the task.
 * @param d Optional QDate representing the new deadline.
 */
void Task::setDeadline(std::optional<QDate> d) {
    deadline_ = std::move(d);
}

/**
 * @brief Adds a document ID to this task’s list of linked documents.
 * @param docId ID of the document to associate with this task.
 */
void Task::addDocumentId(std::string docId) {
    documentIds_.push_back(std::move(docId));
}

/**
 * @brief Removes a linked document ID from this task.
 * @param docId ID of the document to remove.
 */
void Task::removeDocumentId(std::string docId) {
    documentIds_.erase(
        std::remove_if(documentIds_.begin(), documentIds_.end(),
                       [&docId](const std::string& id) { return id == docId; }),
        documentIds_.end()
        );
}

/**
 * @brief Adds a new checklist item to the task.
 * @param text The text or description of the checklist item.
 */
void Task::addChecklistItem(const std::string& text) {
    checklistItems_.emplace_back(text, false);
}

/**
 * @brief Toggles the completion status of a checklist item.
 * @param index Index of the checklist item to toggle.
 */
void Task::toggleChecklistItem(size_t index) {
    if (index < checklistItems_.size()) {
        checklistItems_[index].completed = !checklistItems_[index].completed;
    }
}

/**
 * @brief Removes a checklist item from the task.
 * @param index Index of the checklist item to remove.
 */
void Task::removeChecklistItem(size_t index) {
    if (index < checklistItems_.size()) {
        checklistItems_.erase(checklistItems_.begin() + index);
    }
}

/**
 * @brief Sets the completion status of the task.
 * @param completed Whether the task is completed.
 */
void Task::setCompleted(bool completed) {
    completed_ = completed;
}

/**
 * @brief Sets the drawing data for the task.
 * @param data The drawing data as a string (base64 encoded image or JSON).
 */
void Task::setDrawingData(std::string data) {
    drawingData_ = std::move(data);
}
