#pragma once
#include <QWidget>
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QDate>
#include <vector>
#include <memory>

// Forward declaration
class Task;
class DashboardManager;

/**
 * @brief A calendar widget that displays tasks by deadline
 * 
 * Shows a calendar view with tasks organized by their deadline dates.
 * Allows users to visualize upcoming deadlines and navigate to specific tasks.
 */
class TaskCalendar : public QWidget {
    Q_OBJECT

public:
    explicit TaskCalendar(DashboardManager* manager, QWidget* parent = nullptr);
    
    void refreshCalendar();

signals:
    void taskSelected(Task* task);

private slots:
    void onDateSelected(const QDate& date);

private:
    DashboardManager* manager_;
    QCalendarWidget* calendar_;
    QListWidget* taskList_;
    QLabel* dateLabel_;
    
    void updateTasksForDate(const QDate& date);
    void highlightDeadlineDates();
};

