#pragma once
#include <string>
#include <vector>
#include <optional>
#include <QDate>

struct ChecklistItem {
    std::string text;
    bool completed;
    
    ChecklistItem(const std::string& text, bool completed = false) 
        : text(text), completed(completed) {}
};

class Task {
public:
    Task(std::string id, std::string title, std::optional<QDate> deadline = std::nullopt);

    const std::string& id() const noexcept { return id_; }
    const std::string& title() const noexcept { return title_; }
    const std::optional<QDate>& deadline() const noexcept { return deadline_; }
    const std::vector<std::string>& getDocumentIds() const noexcept { return documentIds_; }
    bool isCompleted() const noexcept { return completed_; }
    const std::string& getDrawingData() const noexcept { return drawingData_; }

    void setTitle(std::string t);
    void setDeadline(std::optional<QDate> d);
    void addDocumentId(std::string docId);
    void removeDocumentId(std::string docId);
    void setCompleted(bool completed);
    void setDrawingData(std::string data);

    const std::vector<std::string>& documentIds() const noexcept { return documentIds_; }
    
    // Checklist methods
    void addChecklistItem(const std::string& text);
    void toggleChecklistItem(size_t index);
    void removeChecklistItem(size_t index);
    const std::vector<ChecklistItem>& getChecklistItems() const noexcept { return checklistItems_; }

private:
    std::string id_;
    std::string title_;
    std::optional<QDate> deadline_;
    std::vector<std::string> documentIds_;
    std::vector<ChecklistItem> checklistItems_;
    bool completed_ = false;
    std::string drawingData_;  // Store drawing as base64 or JSON
};
