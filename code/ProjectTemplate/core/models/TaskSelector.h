#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

// Forward declarations
class Task;
class DashboardManager;

/**
 * @brief A task selection widget with filtering and search capabilities
 * 
 * Displays all tasks in a searchable, filterable list.
 * Allows users to select and open tasks without going through the dashboard.
 */
class TaskSelector : public QWidget {
    Q_OBJECT

public:
    explicit TaskSelector(DashboardManager* manager, QWidget* parent = nullptr);
    
    void refreshTaskList();

signals:
    void taskSelected(Task* task);

private slots:
    void onFilterChanged(int index);
    void onSearchTextChanged(const QString& text);
    void onTaskDoubleClicked(class QListWidgetItem* item);

private:
    DashboardManager* manager_;
    QListWidget* taskList_;
    QLineEdit* searchBox_;
    QComboBox* filterCombo_;
    QLabel* countLabel_;
    
    enum FilterType {
        All = 0,
        Incomplete = 1,
        Completed = 2,
        HasDeadline = 3,
        Overdue = 4
    };
    
    void populateTaskList(const QString& searchText = "", FilterType filter = All);
    bool matchesFilter(const Task* task, FilterType filter) const;
};

