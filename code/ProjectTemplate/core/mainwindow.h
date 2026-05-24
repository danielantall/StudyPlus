#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QDialog>
#include <QDockWidget>
#include <memory>
#include "dashboardmanager.h"
#include "core/models/Task.h"
#include "core/models/TextDocument.h"
#include "core/models/Pomodoro/pomodorowidget.h"
#include "core/models/Pomodoro/pomodorosessionmanager.h"
#include "core/models/Pomodoro/pomodoroSessionRecord.h"
#include "core/models/DrawingCanvas.h"
#include "core/models/TaskCalendar.h"
#include "core/models/TaskSelector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward declarations
class PomodoroSessionManager;
class PomodoroWidget;
class DrawingCanvas;
class TaskCalendar;
class TaskSelector;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshDashboard();
    void openTask(Task* task);
    void onPomodoroSessionEnded(const SessionRecord& record);
    void updateStatistics();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<DashboardManager> dashboardManager_;
    
    // Document dashboard widgets
    QWidget* documentDashboard_ = nullptr;
    QVBoxLayout* documentListLayout_ = nullptr;
    
    // Pomodoro integration - supporting both approaches
    PomodoroSessionManager* pomodoroManager_ = nullptr;
    PomodoroWidget* pomodoroWidget_ = nullptr;
    QDockWidget* pomodoroDock_ = nullptr;
    QDialog* pomodoroDialog_ = nullptr;
    int nextSessionId_ = 1;

    void createTaskCard(const Task& task);
    void loadTaskIntoWorkspace(Task* task);
    
    // Document management
    void setupDocumentDashboard(Task* task);
    void refreshDocumentList(Task* task);
    void createDocumentCard(Document *doc, Task* task);
    void createNewDocument(Task* task);
    void deleteDocument(const std::string& docId, Task* task);
    void saveCurrentDocument();
    
    // Pomodoro integration
    void startPomodoroSession();
    void updatePomodoroSummary();
    
    // Checklist management
    void addChecklistItem();
    void refreshChecklist();
    void onChecklistItemClicked(class QListWidgetItem* item);
    
    // New features
    void setupDrawingTab(Task* task);
    void saveDrawing();
    void setupCalendarView();
    void updateTaskCompletion(bool completed);
    void updateDeadline(const QDate& date);
    void setupTaskMetadataUI(Task* task);
    
    // UI components for new features
    DrawingCanvas* drawingCanvas_ = nullptr;
    TaskCalendar* calendarWidget_ = nullptr;
    TaskSelector* taskSelector_ = nullptr;
    
    void setupTaskSelector();
};
