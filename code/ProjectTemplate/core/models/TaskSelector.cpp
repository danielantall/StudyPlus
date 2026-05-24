#include "TaskSelector.h"
#include "../dashboardmanager.h"
#include "Task.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QFrame>
#include <QDate>

/**
 * @brief Constructs a TaskSelector widget
 * @param manager Pointer to the DashboardManager
 * @param parent The parent widget
 */
TaskSelector::TaskSelector(DashboardManager* manager, QWidget* parent)
    : QWidget(parent), manager_(manager)
{
    // Set background for entire widget - dark grey theme
    setStyleSheet("QWidget { background-color: #3a3a3a; }");
    
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // === Header ===
    auto* headerLabel = new QLabel("Select a Task to Work On");
    headerLabel->setStyleSheet("font-size: 18pt; font-weight: bold; color: #e0e0e0; background: transparent;");
    mainLayout->addWidget(headerLabel);
    
    // === Search and Filter Bar ===
    auto* controlsLayout = new QHBoxLayout();
    
    // Search box
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("🔍 Search tasks by title...");
    searchBox_->setStyleSheet("QLineEdit { padding: 8px; font-size: 11pt; border: 2px solid #555; border-radius: 4px; background: #2b2b2b; color: #e0e0e0; }"
                               "QLineEdit:focus { border-color: #0078d7; }");
    searchBox_->setMinimumWidth(300);
    
    // Filter combo
    filterCombo_ = new QComboBox();
    filterCombo_->addItem("All Tasks", All);
    filterCombo_->addItem("Incomplete", Incomplete);
    filterCombo_->addItem("Completed", Completed);
    filterCombo_->addItem("Has Deadline", HasDeadline);
    filterCombo_->addItem("Overdue", Overdue);
    filterCombo_->setStyleSheet("QComboBox { padding: 8px; font-size: 11pt; border: 2px solid #555; border-radius: 4px; background: #2b2b2b; color: #e0e0e0; }"
                                "QComboBox:focus { border-color: #0078d7; }"
                                "QComboBox::drop-down { border: none; }"
                                "QComboBox QAbstractItemView { background: #2b2b2b; color: #e0e0e0; selection-background-color: #0078d7; }");
    filterCombo_->setMinimumWidth(150);
    
    controlsLayout->addWidget(searchBox_);
    controlsLayout->addWidget(filterCombo_);
    controlsLayout->addStretch();
    
    mainLayout->addLayout(controlsLayout);
    
    // === Task Count Label ===
    countLabel_ = new QLabel("0 tasks");
    countLabel_->setStyleSheet("font-size: 10pt; color: #999; padding: 4px; background: transparent;");
    mainLayout->addWidget(countLabel_);
    
    // === Task List ===
    taskList_ = new QListWidget();
    taskList_->setStyleSheet(
        "QListWidget { "
        "   border: 2px solid #555; "
        "   border-radius: 8px; "
        "   font-size: 11pt; "
        "   background: #2b2b2b; "
        "   color: #e0e0e0; "
        "   padding: 4px; "
        "}"
        "QListWidget::item { "
        "   padding: 12px; "
        "   margin: 2px; "
        "   border-radius: 4px; "
        "   color: #e0e0e0; "
        "   background: #2b2b2b; "
        "}"
        "QListWidget::item:hover { "
        "   background: #404040; "
        "   color: #ffffff; "
        "}"
        "QListWidget::item:selected { "
        "   background: #0078d7; "
        "   color: white; "
        "   border: none; "
        "}"
    );
    taskList_->setAlternatingRowColors(false);
    mainLayout->addWidget(taskList_);
    
    // === Instruction Label ===
    auto* instructionLabel = new QLabel("💡 Double-click a task to open it");
    instructionLabel->setStyleSheet("font-size: 10pt; color: #999; font-style: italic; padding: 8px; background: transparent;");
    instructionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(instructionLabel);
    
    // === Connect Signals ===
    connect(searchBox_, &QLineEdit::textChanged, this, &TaskSelector::onSearchTextChanged);
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TaskSelector::onFilterChanged);
    connect(taskList_, &QListWidget::itemDoubleClicked, this, &TaskSelector::onTaskDoubleClicked);
    
    // Initial population
    refreshTaskList();
}

/**
 * @brief Refreshes the task list from the DashboardManager
 */
void TaskSelector::refreshTaskList()
{
    FilterType currentFilter = static_cast<FilterType>(filterCombo_->currentData().toInt());
    QString searchText = searchBox_->text();
    populateTaskList(searchText, currentFilter);
}

/**
 * @brief Handles filter changes
 * @param index The new filter index
 */
void TaskSelector::onFilterChanged(int index)
{
    Q_UNUSED(index);
    refreshTaskList();
}

/**
 * @brief Handles search text changes
 * @param text The new search text
 */
void TaskSelector::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    refreshTaskList();
}

/**
 * @brief Handles task double-click to open it
 * @param item The clicked list item
 */
void TaskSelector::onTaskDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    QString taskId = item->data(Qt::UserRole).toString();
    Task* task = manager_->findTaskById(taskId);
    
    if (task) {
        emit taskSelected(task);
    }
}

/**
 * @brief Populates the task list with filtering and search
 * @param searchText Text to filter by (searches in title)
 * @param filter Filter type to apply
 */
void TaskSelector::populateTaskList(const QString& searchText, FilterType filter)
{
    taskList_->clear();
    
    int matchCount = 0;
    QString lowerSearch = searchText.toLower();
    
    for (const auto& task : manager_->tasks()) {
        // Apply filter
        if (!matchesFilter(task.get(), filter)) {
            continue;
        }
        
        // Apply search
        if (!searchText.isEmpty()) {
            QString title = QString::fromStdString(task->title()).toLower();
            if (!title.contains(lowerSearch)) {
                continue;
            }
        }
        
        matchCount++;
        
        // Create list item
        QString displayText;
        QString iconPrefix;
        
        // Status icon
        if (task->isCompleted()) {
            iconPrefix = "✅ ";
        } else if (task->deadline() && *task->deadline() < QDate::currentDate()) {
            iconPrefix = "🔴 ";  // Overdue
        } else if (task->deadline()) {
            iconPrefix = "📅 ";  // Has deadline
        } else {
            iconPrefix = "📝 ";  // Regular task
        }
        
        displayText = iconPrefix + QString::fromStdString(task->title());
        
        // Add metadata line
        QStringList metadata;
        if (task->deadline()) {
            QString deadlineStr = task->deadline()->toString("MMM dd, yyyy");
            if (*task->deadline() < QDate::currentDate() && !task->isCompleted()) {
                metadata << QString("⚠️ Overdue: %1").arg(deadlineStr);
            } else {
                metadata << QString("Due: %1").arg(deadlineStr);
            }
        }
        
        int docCount = task->getDocumentIds().size();
        if (docCount > 0) {
            metadata << QString("%1 doc%2").arg(docCount).arg(docCount == 1 ? "" : "s");
        }
        
        int checklistCount = task->getChecklistItems().size();
        if (checklistCount > 0) {
            int completedCount = 0;
            for (const auto& item : task->getChecklistItems()) {
                if (item.completed) completedCount++;
            }
            metadata << QString("Checklist: %1/%2").arg(completedCount).arg(checklistCount);
        }
        
        if (!metadata.isEmpty()) {
            displayText += "\n   " + metadata.join(" • ");
        }
        
        auto* listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, QString::fromStdString(task->id()));
        
        // Style based on status
        QFont font = listItem->font();
        if (task->isCompleted()) {
            font.setStrikeOut(true);
            listItem->setForeground(QColor("#81c784"));  // Light green for completed
        } else if (task->deadline() && *task->deadline() < QDate::currentDate()) {
            listItem->setForeground(QColor("#ff5252"));  // Bright red for overdue
            font.setBold(true);
        } else {
            listItem->setForeground(QColor("#e0e0e0"));  // Default light text
        }
        listItem->setFont(font);
        
        taskList_->addItem(listItem);
    }
    
    // Update count label
    if (matchCount == 0) {
        countLabel_->setText("No tasks found");
        countLabel_->setStyleSheet("font-size: 10pt; color: #ff5252; padding: 4px; background: transparent;");
    } else {
        countLabel_->setText(QString("%1 task%2").arg(matchCount).arg(matchCount == 1 ? "" : "s"));
        countLabel_->setStyleSheet("font-size: 10pt; color: #999; padding: 4px; background: transparent;");
    }
}

/**
 * @brief Checks if a task matches the current filter
 * @param task The task to check
 * @param filter The filter type
 * @return true if task matches filter
 */
bool TaskSelector::matchesFilter(const Task* task, FilterType filter) const
{
    if (!task) return false;
    
    switch (filter) {
        case All:
            return true;
            
        case Incomplete:
            return !task->isCompleted();
            
        case Completed:
            return task->isCompleted();
            
        case HasDeadline:
            return task->deadline().has_value();
            
        case Overdue:
            return task->deadline().has_value() 
                   && *task->deadline() < QDate::currentDate()
                   && !task->isCompleted();
            
        default:
            return true;
    }
}

