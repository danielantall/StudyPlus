#include "TaskCalendar.h"
#include "../dashboardmanager.h"
#include "Task.h"
#include <QTextCharFormat>
#include <QPushButton>
#include <QListWidgetItem>

/**
 * @brief Constructs a TaskCalendar widget
 * @param manager Pointer to the DashboardManager
 * @param parent The parent widget
 */
TaskCalendar::TaskCalendar(DashboardManager* manager, QWidget* parent)
    : QWidget(parent), manager_(manager)
{
    auto* layout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Task Schedule");
    titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; padding: 8px;");
    layout->addWidget(titleLabel);
    
    // Calendar widget
    calendar_ = new QCalendarWidget(this);
    calendar_->setGridVisible(true);
    calendar_->setMinimumDate(QDate::currentDate().addYears(-1));
    calendar_->setMaximumDate(QDate::currentDate().addYears(2));
    layout->addWidget(calendar_);
    
    // Date label
    dateLabel_ = new QLabel("Select a date to view tasks");
    dateLabel_->setStyleSheet("font-weight: bold; padding: 8px;");
    layout->addWidget(dateLabel_);
    
    // Task list for selected date
    taskList_ = new QListWidget(this);
    taskList_->setMinimumHeight(150);
    layout->addWidget(taskList_);
    
    // Connect signals
    connect(calendar_, &QCalendarWidget::clicked, this, &TaskCalendar::onDateSelected);
    connect(taskList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        QString taskId = item->data(Qt::UserRole).toString();
        Task* task = manager_->findTaskById(taskId);
        if (task) {
            emit taskSelected(task);
        }
    });
    
    refreshCalendar();
}

/**
 * @brief Refreshes the calendar to show current deadline dates
 */
void TaskCalendar::refreshCalendar()
{
    highlightDeadlineDates();
}

/**
 * @brief Handles date selection in the calendar
 * @param date The selected date
 */
void TaskCalendar::onDateSelected(const QDate& date)
{
    dateLabel_->setText(QString("Tasks for %1").arg(date.toString("MMMM d, yyyy")));
    updateTasksForDate(date);
}

/**
 * @brief Updates the task list for a specific date
 * @param date The date to show tasks for
 */
void TaskCalendar::updateTasksForDate(const QDate& date)
{
    taskList_->clear();
    
    int taskCount = 0;
    for (const auto& task : manager_->tasks()) {
        if (task->deadline() && *task->deadline() == date) {
            QString displayText = QString::fromStdString(task->title());
            if (task->isCompleted()) {
                displayText = "✓ " + displayText;
            }
            
            auto* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, QString::fromStdString(task->id()));
            
            // Style based on completion
            if (task->isCompleted()) {
                QFont font = item->font();
                font.setStrikeOut(true);
                item->setFont(font);
                item->setForeground(Qt::darkGreen);
            } else if (date < QDate::currentDate()) {
                // Overdue
                item->setForeground(Qt::red);
            }
            
            taskList_->addItem(item);
            taskCount++;
        }
    }
    
    if (taskCount == 0) {
        taskList_->addItem("No tasks scheduled for this date");
    }
}

/**
 * @brief Highlights dates that have task deadlines
 */
void TaskCalendar::highlightDeadlineDates()
{
    // Reset all date formats
    QTextCharFormat defaultFormat;
    calendar_->setDateTextFormat(QDate(), defaultFormat);
    
    // Highlight dates with deadlines
    QTextCharFormat deadlineFormat;
    deadlineFormat.setBackground(QColor(255, 200, 200));
    deadlineFormat.setFontWeight(QFont::Bold);
    
    QTextCharFormat completedFormat;
    completedFormat.setBackground(QColor(200, 255, 200));
    
    QTextCharFormat overdueFormat;
    overdueFormat.setBackground(QColor(255, 100, 100));
    overdueFormat.setFontWeight(QFont::Bold);
    
    for (const auto& task : manager_->tasks()) {
        if (task->deadline()) {
            QDate deadline = *task->deadline();
            
            if (task->isCompleted()) {
                calendar_->setDateTextFormat(deadline, completedFormat);
            } else if (deadline < QDate::currentDate()) {
                calendar_->setDateTextFormat(deadline, overdueFormat);
            } else {
                calendar_->setDateTextFormat(deadline, deadlineFormat);
            }
        }
    }
}

