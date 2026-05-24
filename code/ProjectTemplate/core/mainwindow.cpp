// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDockWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTextEdit>
#include <QDateTime>
#include <QMessageBox>
#include <QStatusBar>
#include <QDialog>
#include <QScrollArea>
#include "models/Pomodoro/pomodorosessionmanager.h"
#include "models/Pomodoro/pomodorowidget.h"
#include "models/DrawingCanvas.h"
#include "models/TaskCalendar.h"
#include "models/TaskSelector.h"
#include <QDateEdit>
#include <QCheckBox>
#include <QSlider>
#include <QDialog>

//=============================================================================
// CONSTRUCTOR & DESTRUCTOR
//=============================================================================

/**
 * @brief Constructs the main window and initializes the UI
 * @param parent Parent widget (default: nullptr)
 *
 * Sets up the main window UI, connects all signals/slots for navigation,
 * task management, document management, checklist, and Pomodoro functionality.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dashboardManager_(std::make_unique<DashboardManager>(this))
{
    ui->setupUi(this);
    setWindowTitle("Study++");

    // Navigation buttons
    connect(ui->btnDashboard, &QPushButton::clicked, this, [this]() {
        ui->mainStack->setCurrentWidget(ui->pageDashboard);
    });

    connect(ui->btnTasks, &QPushButton::clicked, this, [this]() {
        // Smart navigation: show task selector if no task is open, otherwise show workspace
        if (dashboardManager_->getCurrentTask()) {
            ui->mainStack->setCurrentWidget(ui->pageWorkspace);
        } else {
            if (taskSelector_) {
                taskSelector_->refreshTaskList();
            }
            ui->mainStack->setCurrentWidget(taskSelector_);
        }
    });

    connect(ui->btnStats, &QPushButton::clicked, this, [this]() {
        updateStatistics();
        ui->mainStack->setCurrentWidget(ui->pageStats);
    });

    // Add task input
    connect(ui->taskInputHeader, &QLineEdit::returnPressed, this, [this]() {
        QString title = ui->taskInputHeader->text().trimmed();
        if (!title.isEmpty()) {
            dashboardManager_->addTask(title);
            ui->taskInputHeader->clear();
        }
    });

    // Refresh dashboard when tasks change
    connect(dashboardManager_.get(), &DashboardManager::tasksUpdated,
            this, &MainWindow::refreshDashboard);

    // Connect other DashboardManager signals
    connect(dashboardManager_.get(), &DashboardManager::documentsUpdated,
            this, [this]() {
                Task* currentTask = dashboardManager_->getCurrentTask();
                if (currentTask) refreshDocumentList(currentTask);
            });

    connect(dashboardManager_.get(), &DashboardManager::checklistUpdated,
            this, [this](const QString&) {
                refreshChecklist();
            });

    connect(dashboardManager_.get(), &DashboardManager::currentTaskChanged,
            this, [this](Task* task) {
                if (task) {
                    loadTaskIntoWorkspace(task);
                }
            });

    connect(dashboardManager_.get(), &DashboardManager::currentDocumentChanged,
            this, [this](const QString& docId) {
                if (!docId.isEmpty()) {
                    Document* doc = dashboardManager_->findDocumentById(docId);
                    if (doc) {
                        ui->textEditor->setText(doc->content());
                    }
                }
            });

    // === Pomodoro start button ===
    connect(ui->btnStartPomodoro, &QPushButton::clicked, [this]() {
        startPomodoroSession();
    });

    // === Save button - now saves current document ===
    connect(ui->btnSave, &QPushButton::clicked, [this]() {
        saveCurrentDocument();
    });

    // === Checklist functionality ===
    connect(ui->btnAddChecklistItem, &QPushButton::clicked, this, &MainWindow::addChecklistItem);
    connect(ui->checklistInput, &QLineEdit::returnPressed, this, &MainWindow::addChecklistItem);
    connect(ui->checklist, &QListWidget::itemClicked, this, &MainWindow::onChecklistItemClicked);

    // === Setup calendar view ===
    setupCalendarView();
    
    // === Setup task selector ===
    setupTaskSelector();

    refreshDashboard();
}

/**
 * @brief Destructor - cleans up Pomodoro resources and UI
 *
 * Ensures proper cleanup of Pomodoro dialog and manager before destroying
 * the main window UI.
 */
MainWindow::~MainWindow() {
    // Clean up Pomodoro resources
    if (pomodoroDialog_) {
        pomodoroDialog_->close();
        pomodoroDialog_->deleteLater();
    }

    if (pomodoroManager_) {
        pomodoroManager_->deleteLater();
    }

    delete ui;
}

//=============================================================================
// DASHBOARD MANAGEMENT
//=============================================================================

/**
 * @brief Refreshes the dashboard by rebuilding all task cards
 *
 * Clears the existing task cards from the dashboard container and creates
 * new cards for each task from the DashboardManager.
 */
void MainWindow::refreshDashboard() {
    // Clear existing task cards
    auto *containerLayout = qobject_cast<QVBoxLayout*>(ui->dashboardContainer->layout());
    if (!containerLayout) return;

    QLayoutItem* item;
    while ((item = containerLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Create new task cards
    for (const auto& task : dashboardManager_->tasks()) {
        createTaskCard(*task);
    }
}

/**
 * @brief Creates a visual card widget for a task
 * @param task The task to create a card for
 *
 * Creates a styled frame containing task information and action buttons
 * (Open, Delete). Adds the card to the dashboard container layout.
 */
void MainWindow::createTaskCard(const Task& task) {
    QFrame *card = new QFrame(ui->dashboardContainer);
    QString cardStyle = task.isCompleted() ? 
        "QFrame { border:2px solid #4caf50; border-radius:8px; padding:12px; background:#2e4a2e; }" :
        "QFrame { border:1px solid #555; border-radius:8px; padding:12px; background:#2b2b2b; }";
    card->setStyleSheet(cardStyle);
    auto *layout = new QVBoxLayout(card);

    // Title with completion indicator
    QString titleText = task.isCompleted() ? 
        "✓ " + QString::fromStdString(task.title()) :
        QString::fromStdString(task.title());
    QLabel *lbl = new QLabel(titleText);
    lbl->setStyleSheet("font-weight:600; font-size:14pt; color:#e0e0e0;");
    layout->addWidget(lbl);

    // Task info with deadline
    QString deadlineText = task.deadline() ? 
        task.deadline()->toString("MMM dd, yyyy") : 
        "No deadline";
    QLabel *desc = new QLabel(QString("Task ID: %1 | Documents: %2 | Deadline: %3")
                                  .arg(QString::fromStdString(task.id()))
                                  .arg(task.getDocumentIds().size())
                                  .arg(deadlineText));
    desc->setStyleSheet("color:#999;");
    layout->addWidget(desc);

    // === Quick Actions Row ===
    QHBoxLayout *bottom = new QHBoxLayout();
    
    // Mark Complete/Incomplete button
    QPushButton *btnComplete = new QPushButton(task.isCompleted() ? "Mark Incomplete" : "✅ Mark Complete");
    btnComplete->setStyleSheet("QPushButton { background: #4caf50; color: white; border: none; padding: 6px 12px; border-radius: 4px; }"
                               "QPushButton:hover { background: #45a049; }");
    if (task.isCompleted()) {
        btnComplete->setStyleSheet("QPushButton { background: #ff9800; color: white; border: none; padding: 6px 12px; border-radius: 4px; }"
                                   "QPushButton:hover { background: #f57c00; }");
    }
    
    // Set/Edit Deadline button
    QPushButton *btnDeadline = new QPushButton(task.deadline() ? "📅 Edit Deadline" : "📅 Set Deadline");
    btnDeadline->setStyleSheet("QPushButton { background: #2196f3; color: white; border: none; padding: 6px 12px; border-radius: 4px; }"
                               "QPushButton:hover { background: #1976d2; }");
    
    QPushButton *btnOpen = new QPushButton("Open");
    btnOpen->setStyleSheet("QPushButton { background: #404040; color: white; border: 1px solid #555; padding: 6px 12px; border-radius: 4px; }"
                           "QPushButton:hover { background: #4a4a4a; }");
    
    QPushButton *btnDelete = new QPushButton("Delete");
    btnDelete->setStyleSheet("QPushButton { background: #d32f2f; color: white; border: none; padding: 6px 12px; border-radius: 4px; }"
                             "QPushButton:hover { background: #b71c1c; }");
    
    bottom->addWidget(btnComplete);
    bottom->addWidget(btnDeadline);
    bottom->addStretch();
    bottom->addWidget(btnOpen);
    bottom->addWidget(btnDelete);
    layout->addLayout(bottom);

    auto *containerLayout = qobject_cast<QVBoxLayout*>(ui->dashboardContainer->layout());
    containerLayout->addWidget(card);

    // Find the actual task pointer from dashboardManager
    Task* taskPtr = nullptr;
    for (const auto& t : dashboardManager_->tasks()) {
        if (t->id() == task.id()) {
            taskPtr = t.get();
            break;
        }
    }

    // === Connect Button Actions ===
    
    // Mark Complete/Incomplete button
    connect(btnComplete, &QPushButton::clicked, this, [this, taskPtr]() {
        if (taskPtr) {
            bool newStatus = !taskPtr->isCompleted();
            dashboardManager_->setTaskCompleted(QString::fromStdString(taskPtr->id()), newStatus);
            
            QString message = newStatus ? "Task marked as completed! ✅" : "Task marked as incomplete.";
            statusBar()->showMessage(message, 2000);
        }
    });
    
    // Set/Edit Deadline button
    connect(btnDeadline, &QPushButton::clicked, this, [this, taskPtr]() {
        if (!taskPtr) return;
        
        QDialog dialog(this);
        dialog.setWindowTitle("Set Task Deadline");
        dialog.setMinimumWidth(350);
        dialog.setStyleSheet("QDialog { background: #2b2b2b; }");
        
        auto* layout = new QVBoxLayout(&dialog);
        
        auto* label = new QLabel("Select deadline for: " + QString::fromStdString(taskPtr->title()));
        label->setStyleSheet("font-weight: bold; padding: 8px; color: #e0e0e0;");
        layout->addWidget(label);
        
        auto* datePicker = new QDateEdit(&dialog);
        datePicker->setCalendarPopup(true);
        datePicker->setMinimumDate(QDate::currentDate());
        datePicker->setDate(taskPtr->deadline() ? *taskPtr->deadline() : QDate::currentDate());
        datePicker->setStyleSheet("QDateEdit { padding: 8px; font-size: 11pt; background: #404040; color: #e0e0e0; border: 1px solid #555; }");
        layout->addWidget(datePicker);
        
        auto* buttonLayout = new QHBoxLayout();
        auto* saveBtn = new QPushButton("Save");
        auto* clearBtn = new QPushButton("Clear Deadline");
        auto* cancelBtn = new QPushButton("Cancel");
        
        saveBtn->setStyleSheet("QPushButton { background: #4caf50; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
                               "QPushButton:hover { background: #45a049; }");
        clearBtn->setStyleSheet("QPushButton { background: #ff9800; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
                                "QPushButton:hover { background: #f57c00; }");
        cancelBtn->setStyleSheet("QPushButton { background: #555; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
                                 "QPushButton:hover { background: #666; }");
        
        buttonLayout->addWidget(saveBtn);
        buttonLayout->addWidget(clearBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(cancelBtn);
        layout->addLayout(buttonLayout);
        
        connect(saveBtn, &QPushButton::clicked, [&dialog, taskPtr, datePicker, this]() {
            taskPtr->setDeadline(datePicker->date());
            dashboardManager_->saveToJson();
            refreshDashboard();
            if (calendarWidget_) calendarWidget_->refreshCalendar();
            statusBar()->showMessage(QString("Deadline set to %1").arg(datePicker->date().toString("MMM dd, yyyy")), 2000);
            dialog.accept();
        });
        
        connect(clearBtn, &QPushButton::clicked, [&dialog, taskPtr, this]() {
            taskPtr->setDeadline(std::nullopt);
            dashboardManager_->saveToJson();
            refreshDashboard();
            if (calendarWidget_) calendarWidget_->refreshCalendar();
            statusBar()->showMessage("Deadline cleared", 2000);
            dialog.accept();
        });
        
        connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
        
        dialog.exec();
    });
    
    // Open task button
    connect(btnOpen, &QPushButton::clicked, this, [this, taskPtr]() {
        if (taskPtr) {
            openTask(taskPtr);
        }
    });

    // Delete task button
    connect(btnDelete, &QPushButton::clicked, this, [this, id = QString::fromStdString(task.id())]() {
        dashboardManager_->removeTask(id);
    });
}

/**
 * @brief Opens a task and switches to the workspace view
 * @param task Pointer to the task to open
 *
 * Sets the current task in DashboardManager, updates the workspace title,
 * loads task content, and automatically starts a Pomodoro session.
 */
void MainWindow::openTask(Task* task) {
    if (!task) return;

    dashboardManager_->setCurrentTask(QString::fromStdString(task->id()));
    ui->lblTaskTitle->setText(QString::fromStdString(task->title()));
    ui->mainStack->setCurrentWidget(ui->pageWorkspace);

    // Load task documents into workspace (integrated approach)
    loadTaskIntoWorkspace(task);

    // Automatically show/start pomodoro for this task
    startPomodoroSession();
}

//=============================================================================
// WORKSPACE MANAGEMENT
//=============================================================================

/**
 * @brief Loads a task into the workspace view
 * @param task Pointer to the task to load
 *
 * Updates the workspace UI with task information, sets up the document
 * dashboard, loads the first document into the editor, and refreshes the
 * checklist. Also resets Pomodoro UI state for the new task.
 */
void MainWindow::loadTaskIntoWorkspace(Task* task)
{
    if (!task) return;

    // Set current task in DashboardManager (this will emit signals)
    dashboardManager_->setCurrentTask(QString::fromStdString(task->id()));

    // Update workspace title
    ui->lblTaskTitle->setText(QString::fromStdString(task->title()));

    // Reset Pomodoro UI for new task
    ui->btnStartPomodoro->setText("⏱ Start Pomodoro");
    ui->btnStartPomodoro->setEnabled(true);
    updatePomodoroSummary();

    // Close any existing Pomodoro session
    if (pomodoroDialog_) {
        pomodoroDialog_->close();
    }

    // Create document dashboard in the first tab
    setupDocumentDashboard(task);

    // Load first document into text editor if available
    const auto& docIds = task->getDocumentIds();
    if (!docIds.empty()) {
        Document* doc = dashboardManager_->findDocumentById(QString::fromStdString(docIds[0]));
        if (doc) {
            ui->textEditor->setText(doc->content());
            dashboardManager_->setCurrentDocument(QString::fromStdString(docIds[0]));
        }
    }

    // Setup drawing tab
    setupDrawingTab(task);

    // Refresh checklist for this task
    refreshChecklist();
    
    // Setup task metadata UI (completion and deadline)
    setupTaskMetadataUI(task);
}

/**
 * @brief Sets up the task metadata UI (completion checkbox and deadline picker)
 * @param task Pointer to the task
 */
void MainWindow::setupTaskMetadataUI(Task* task)
{
    if (!task) return;
    
    // Find or create metadata widget
    QWidget* metadataWidget = ui->workspaceTabs->findChild<QWidget*>("taskMetadataWidget");
    if (!metadataWidget) {
        metadataWidget = new QWidget();
        metadataWidget->setObjectName("taskMetadataWidget");
        auto* metaLayout = new QVBoxLayout(metadataWidget);
        
        // Completion checkbox
        auto* completionCheck = new QCheckBox("Mark as Completed");
        completionCheck->setObjectName("completionCheckbox");
        completionCheck->setStyleSheet("font-size: 11pt; padding: 8px;");
        metaLayout->addWidget(completionCheck);
        
        // Deadline section
        auto* deadlineLayout = new QHBoxLayout();
        auto* deadlineLabel = new QLabel("Deadline:");
        deadlineLabel->setStyleSheet("font-weight: bold;");
        auto* deadlinePicker = new QDateEdit();
        deadlinePicker->setObjectName("deadlinePicker");
        deadlinePicker->setCalendarPopup(true);
        deadlinePicker->setDate(QDate::currentDate());
        deadlinePicker->setMinimumDate(QDate::currentDate());
        
        auto* clearDeadlineBtn = new QPushButton("Clear");
        clearDeadlineBtn->setObjectName("clearDeadlineBtn");
        
        deadlineLayout->addWidget(deadlineLabel);
        deadlineLayout->addWidget(deadlinePicker);
        deadlineLayout->addWidget(clearDeadlineBtn);
        deadlineLayout->addStretch();
        
        metaLayout->addLayout(deadlineLayout);
        metaLayout->addStretch();
        
        // Add as first item in checklist tab
        auto* checklistLayout = qobject_cast<QVBoxLayout*>(ui->tabChecklist->layout());
        if (checklistLayout) {
            checklistLayout->insertWidget(0, metadataWidget);
        }
    }
    
    // Update values
    auto* completionCheck = metadataWidget->findChild<QCheckBox*>("completionCheckbox");
    auto* deadlinePicker = metadataWidget->findChild<QDateEdit*>("deadlinePicker");
    auto* clearDeadlineBtn = metadataWidget->findChild<QPushButton*>("clearDeadlineBtn");
    
    if (completionCheck) {
        completionCheck->blockSignals(true);
        completionCheck->setChecked(task->isCompleted());
        completionCheck->blockSignals(false);
        
        disconnect(completionCheck, nullptr, this, nullptr);
        connect(completionCheck, &QCheckBox::toggled, this, &MainWindow::updateTaskCompletion);
    }
    
    if (deadlinePicker) {
        deadlinePicker->blockSignals(true);
        if (task->deadline()) {
            deadlinePicker->setDate(*task->deadline());
        } else {
            deadlinePicker->setDate(QDate::currentDate());
        }
        deadlinePicker->blockSignals(false);
        
        disconnect(deadlinePicker, nullptr, this, nullptr);
        connect(deadlinePicker, &QDateEdit::dateChanged, this, &MainWindow::updateDeadline);
    }
    
    if (clearDeadlineBtn) {
        disconnect(clearDeadlineBtn, nullptr, this, nullptr);
        connect(clearDeadlineBtn, &QPushButton::clicked, this, [this, task]() {
            if (task) {
                task->setDeadline(std::nullopt);
                dashboardManager_->saveToJson();
                if (calendarWidget_) {
                    calendarWidget_->refreshCalendar();
                }
                QMessageBox::information(this, "Deadline", "Deadline cleared.");
            }
        });
    }
}

/**
 * @brief Saves the current document and task data
 *
 * Saves the text editor content to the current document (if any) and
 * persists all task data via DashboardManager. Shows confirmation message.
 */
void MainWindow::saveCurrentDocument()
{
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) {
        QMessageBox::information(this, "Save", "No task currently open to save.");
        return;
    }

    // Save document content if there's a current document
    QString currentDocId = dashboardManager_->getCurrentDocumentId();
    if (!currentDocId.isEmpty()) {
        Document* doc = dashboardManager_->findDocumentById(currentDocId);
        if (doc) {
            // Save content from text editor to document
            doc->setContent(ui->textEditor->toPlainText());

            // Refresh document list to show updated content length
            refreshDocumentList(currentTask);
        }
    }

    // Save task data (including checklist) via DashboardManager
    dashboardManager_->saveToJson();

    QMessageBox::information(this, "Save", "Task and document data saved successfully!");
}

//=============================================================================
// DOCUMENT MANAGEMENT
//=============================================================================

/**
 * @brief Sets up the document dashboard tab for a task
 * @param task Pointer to the task whose documents to display
 *
 * Creates a new document dashboard widget with a header, "New Document" button,
 * and scrollable list of existing documents. Adds it as the first tab in the
 * workspace tab widget.
 */
void MainWindow::setupDocumentDashboard(Task* task)
{
    if (!task) return;

    // Clear existing document dashboard
    if (documentDashboard_) {
        documentDashboard_->deleteLater();
    }

    // Create document dashboard widget
    documentDashboard_ = new QWidget();
    auto* dashboardLayout = new QVBoxLayout(documentDashboard_);

    // === Header with "New Document" button ===
    auto* headerLayout = new QHBoxLayout();
    auto* headerLabel = new QLabel("Documents");
    headerLabel->setStyleSheet("font-weight:600; font-size:12pt; color:#e0e0e0;");

    auto* newDocBtn = new QPushButton("+ New Document");
    newDocBtn->setStyleSheet("QPushButton { background:#0078d7; color:white; border:none; padding:6px 12px; border-radius:4px; }"
                             "QPushButton:hover { background:#106ebe; }");

    headerLayout->addWidget(headerLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(newDocBtn);
    dashboardLayout->addLayout(headerLayout);

    // === Document list scroll area ===
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    auto* scrollWidget = new QWidget();
    documentListLayout_ = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);
    dashboardLayout->addWidget(scrollArea);

    // === Populate with existing documents ===
    refreshDocumentList(task);

    // === Connect new document button ===
    connect(newDocBtn, &QPushButton::clicked, [this, task]() {
        createNewDocument(task);
    });

    // === Add dashboard as a new tab ===
    ui->workspaceTabs->insertTab(0, documentDashboard_, "Documents");
    ui->workspaceTabs->setCurrentIndex(0); // Show documents tab first
}

/**
 * @brief Refreshes the document list for the current task
 * @param task Pointer to the task whose documents to refresh
 *
 * Clears and rebuilds the document card list in the document dashboard.
 */
void MainWindow::refreshDocumentList(Task* task)
{
    if (!task || !documentListLayout_) return;

    // Clear existing document cards
    QLayoutItem* item;
    while ((item = documentListLayout_->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Create cards for each document
    const auto& docIds = task->getDocumentIds();
    for (const auto& docId : docIds) {
        Document* doc = dashboardManager_->findDocumentById(QString::fromStdString(docId));
        if (doc) {
            createDocumentCard(doc, task);
        }
    }

    // Add stretch at the end
    documentListLayout_->addStretch();
}

/**
 * @brief Creates a visual card widget for a document
 * @param doc Pointer to the document
 * @param task Pointer to the task owning the document
 *
 * Creates a styled frame containing document information and action buttons
 * (Open, Edit, Delete). Adds the card to the document list layout.
 */
void MainWindow::createDocumentCard(Document* doc, Task* task)
{
    if (!doc || !documentListLayout_) return;

    // === Create document card ===
    auto* card = new QFrame();
    card->setStyleSheet("QFrame { border:1px solid #555; border-radius:6px; margin:2px; background:#2b2b2b; }"
                        "QFrame:hover { background:#333333; }");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);

    // === Document name and info ===
    auto* nameLabel = new QLabel(QString::fromStdString(doc->name()));
    nameLabel->setStyleSheet("font-weight:600; font-size:11pt; color:#e0e0e0;");

    auto* infoLabel = new QLabel(QString("ID: %1 | Length: %2 chars")
                                     .arg(QString::fromStdString(doc->id()))
                                     .arg(doc->content().length()));
    infoLabel->setStyleSheet("color:#999; font-size:9pt;");

    // === Button row ===
    auto* buttonLayout = new QHBoxLayout();
    auto* openBtn = new QPushButton("Open");
    auto* editBtn = new QPushButton("Edit");
    auto* deleteBtn = new QPushButton("Delete");

    openBtn->setStyleSheet("QPushButton { border:1px solid #555; padding:4px 8px; border-radius:3px; background:#404040; color:#e0e0e0; }"
                           "QPushButton:hover { background:#4a4a4a; }");
    editBtn->setStyleSheet("QPushButton { border:1px solid #555; padding:4px 8px; border-radius:3px; background:#404040; color:#e0e0e0; }"
                           "QPushButton:hover { background:#4a4a4a; }");
    deleteBtn->setStyleSheet("QPushButton { border:none; color:white; padding:4px 8px; border-radius:3px; background:#d32f2f; }"
                             "QPushButton:hover { background:#b71c1c; }");

    buttonLayout->addWidget(openBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(deleteBtn);

    cardLayout->addWidget(nameLabel);
    cardLayout->addWidget(infoLabel);
    cardLayout->addLayout(buttonLayout);

    // === Connect buttons ===
    connect(openBtn, &QPushButton::clicked, [this, doc]() {
        // Load document content into text editor
        ui->textEditor->setText(doc->content());
        dashboardManager_->setCurrentDocument(QString::fromStdString(doc->id()));
        ui->workspaceTabs->setCurrentWidget(ui->tabText); // Switch to text editor tab
    });

    connect(editBtn, &QPushButton::clicked, [this, doc]() {
        // Same as open for now - could add inline editing later
        ui->textEditor->setText(doc->content());
        dashboardManager_->setCurrentDocument(QString::fromStdString(doc->id()));
        ui->workspaceTabs->setCurrentWidget(ui->tabText);
    });

    connect(deleteBtn, &QPushButton::clicked, [this, doc, task]() {
        deleteDocument(doc->id(), task);
    });

    documentListLayout_->addWidget(card);
}

/**
 * @brief Creates a new document for a task
 * @param task Pointer to the task to create a document for
 *
 * Creates a new document via DashboardManager with a default name based on
 * the task title. The document list will refresh automatically via signals.
 */
void MainWindow::createNewDocument(Task* task)
{
    if (!task) return;

    QString taskId = QString::fromStdString(task->id());
    QString docName = QString::fromStdString(task->title()) + " - New Document";

    // Use DashboardManager to create the document
    dashboardManager_->createDocument(taskId, docName);

    // Refresh will happen automatically via documentsUpdated signal
}

/**
 * @brief Deletes a document from a task
 * @param docId The document ID to delete
 * @param task Pointer to the task owning the document
 *
 * Deletes the document via DashboardManager and clears the text editor if
 * the deleted document was currently open.
 */
void MainWindow::deleteDocument(const std::string& docId, Task* task)
{
    if (!task) return;

    QString documentId = QString::fromStdString(docId);
    QString taskId = QString::fromStdString(task->id());

    // Use DashboardManager to delete the document
    dashboardManager_->deleteDocument(documentId, taskId);

    // Clear editor if this was the current document
    if (dashboardManager_->getCurrentDocumentId() == documentId) {
        ui->textEditor->clear();
        dashboardManager_->setCurrentDocument("");
    }
}

//=============================================================================
// POMODORO SESSION MANAGEMENT
//=============================================================================

/**
 * @brief Starts a new Pomodoro session for the current task
 *
 * Creates a new PomodoroSessionManager and displays the Pomodoro timer in a
 * dock widget. If a session already exists for this task, shows the existing
 * session. If a session exists for a different task, ends it first.
 */
void MainWindow::startPomodoroSession() {
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) {
        QMessageBox::warning(this, "Pomodoro", "Please open a task first before starting a Pomodoro session.");
        return;
    }

    // If there's already an active session for this task, just show it
    if (pomodoroManager_ && pomodoroManager_->getTask() == currentTask) {
        if (pomodoroDock_) {
            pomodoroDock_->show();
            pomodoroDock_->raise();
        }
        return;
    }

    // If there's a session for a different task, end it first
    if (pomodoroManager_) {
        pomodoroManager_->end();
        // onPomodoroSessionEnded will be called and clean up
    }

    // Create new session for current task
    int sessionId = nextSessionId_++;

    pomodoroManager_ = new PomodoroSessionManager(currentTask, sessionId, this);

    // Connect session ended signal
    connect(pomodoroManager_, &PomodoroSessionManager::sessionEnded,
            this, &MainWindow::onPomodoroSessionEnded);

    // Create or reuse dock widget
    pomodoroDock_ = new QDockWidget("Pomodoro Timer", this);
    pomodoroDock_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, pomodoroDock_);

    connect(pomodoroDock_, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (!visible) {
            ui->btnStartPomodoro->setText("⏱ Start Pomodoro");
            ui->btnStartPomodoro->setEnabled(true);
        }else{
            ui->btnStartPomodoro->setText("🍅 Pomodoro Active");
            ui->btnStartPomodoro->setEnabled(false);
        }
    });

    // Create new pomodoro widget
    pomodoroWidget_ = new PomodoroWidget(pomodoroManager_, pomodoroDock_);
    pomodoroDock_->setWidget(pomodoroWidget_);
    pomodoroDock_->show();
}

/**
 * @brief Handles Pomodoro session completion
 * @param record The session record containing interval history and statistics
 *
 * Saves the completed session to DashboardManager, displays a completion
 * message with statistics, cleans up Pomodoro resources, and updates the UI.
 */
void MainWindow::onPomodoroSessionEnded(const SessionRecord& record)
{
    // Save session to dashboard manager
    if (dashboardManager_) {
        dashboardManager_->recordSession(record);
    }

    // Show completion message
    QMessageBox::information(this, "Pomodoro Complete!",
                             QString("Session completed!\n"
                                     "Task: %1\n"
                                     "Total Focus Time: %2 minutes\n"
                                     "Intervals Completed: %3")
                                 .arg(QString::fromStdString(dashboardManager_->getCurrentTask()->title()))
                                 .arg(record.getTotalFocusMinutes())
                                 .arg(record.getIntervalCount()));

    // Clean up pomodoro resources
    if (pomodoroWidget_) {
        pomodoroWidget_->deleteLater();
        pomodoroWidget_ = nullptr;
    }

    if (pomodoroManager_) {
        pomodoroManager_->deleteLater();
        pomodoroManager_ = nullptr;
    }

    // Hide dock or close dialog
    if (pomodoroDock_) {
        pomodoroDock_->close();
        pomodoroDock_->deleteLater();
        pomodoroDock_ = nullptr;
    }

    if (pomodoroDialog_) {
        pomodoroDialog_->close();
        pomodoroDialog_->deleteLater();
        pomodoroDialog_ = nullptr;
    }

    // Update UI
    updatePomodoroSummary();

    // Reset UI
    ui->btnStartPomodoro->setText("⏱ Start Pomodoro");
    ui->btnStartPomodoro->setEnabled(true);
}

/**
 * @brief Updates the Pomodoro summary display for the current task
 *
 * Shows the number of completed Pomodoro sessions for the current task.
 */
void MainWindow::updatePomodoroSummary()
{
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (currentTask) {
        // For now, show placeholder text. You could extend this to track actual sessions
        ui->lblPomodoroSummary->setText(QString("🍅 %1 sessions completed")
                                            .arg(nextSessionId_ - 1));
    }
}

//=============================================================================
// CHECKLIST MANAGEMENT
//=============================================================================

/**
 * @brief Adds a new item to the current task's checklist
 *
 * Reads text from the checklist input field, adds it to the current task's
 * checklist via DashboardManager, and refreshes the checklist display.
 */
void MainWindow::addChecklistItem() {
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) return;

    QString itemText = ui->checklistInput->text().trimmed();
    if (itemText.isEmpty()) return;

    // Use centralized DashboardManager
    dashboardManager_->addChecklistItem(QString::fromStdString(currentTask->id()), itemText);
    ui->checklistInput->clear();
    refreshChecklist();
}

/**
 * @brief Refreshes the checklist display for the current task
 *
 * Clears the checklist widget and repopulates it with items from the current
 * task, showing completion status and formatting completed items with strikethrough.
 */
void MainWindow::refreshChecklist() {
    ui->checklist->clear();

    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) return;

    const auto& items = currentTask->getChecklistItems();
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        QString displayText = item.completed ?
                                  QString("✓ %1").arg(QString::fromStdString(item.text)) :
                                  QString("☐ %1").arg(QString::fromStdString(item.text));

        auto* listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, static_cast<int>(i)); // Store index

        if (item.completed) {
            QFont font = listItem->font();
            font.setStrikeOut(true);
            listItem->setFont(font);
        }

        ui->checklist->addItem(listItem);
    }
}

/**
 * @brief Handles checklist item click events
 * @param item The QListWidgetItem that was clicked
 *
 * Toggles the completion status of the clicked checklist item and refreshes
 * the display.
 */
void MainWindow::onChecklistItemClicked(QListWidgetItem* item) {
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask || !item) return;

    int index = item->data(Qt::UserRole).toInt();
    // Use centralized DashboardManager
    dashboardManager_->toggleChecklistItem(QString::fromStdString(currentTask->id()), static_cast<size_t>(index));
    refreshChecklist();
}

//=============================================================================
// STATISTICS
//=============================================================================

/**
 * @brief Updates all statistics displays on the statistics page
 *
 * Queries DashboardManager for current statistics and updates all stat labels:
 * total tasks, completed tasks, total documents, Pomodoro sessions, and focus time.
 */
void MainWindow::updateStatistics() {
    // Use centralized DashboardManager for all statistics
    ui->lblTotalTasksValue->setText(QString::number(dashboardManager_->getTotalTasks()));
    ui->lblCompletedTasksValue->setText(QString::number(dashboardManager_->getCompletedTasks()));
    ui->lblTotalDocumentsValue->setText(QString::number(dashboardManager_->getTotalDocuments()));
    ui->lblPomodoroSessionsValue->setText(QString::number(dashboardManager_->getTotalPomodoroSessions()));
    ui->lblTotalFocusTimeValue->setText(QString::number(dashboardManager_->getTotalFocusMinutes()) + " min");
}

//=============================================================================
// NEW FEATURES - DRAWING, CALENDAR, COMPLETION
//=============================================================================

/**
 * @brief Sets up the drawing tab for a task
 * @param task Pointer to the task
 */
void MainWindow::setupDrawingTab(Task* task)
{
    if (!task) return;
    
    // Remove old drawing canvas if exists
    if (drawingCanvas_) {
        drawingCanvas_->deleteLater();
    }
    
    // Create new drawing canvas
    QWidget* drawingTab = new QWidget();
    auto* drawingLayout = new QVBoxLayout(drawingTab);
    
    // Toolbar
    auto* toolbar = new QWidget();
    auto* toolbarLayout = new QHBoxLayout(toolbar);
    
    auto* penWidthLabel = new QLabel("Pen Width:");
    auto* penWidthSlider = new QSlider(Qt::Horizontal);
    penWidthSlider->setRange(1, 10);
    penWidthSlider->setValue(2);
    penWidthSlider->setMaximumWidth(150);
    
    auto* colorLabel = new QLabel("Color:");
    auto* blackBtn = new QPushButton("Black");
    auto* redBtn = new QPushButton("Red");
    auto* blueBtn = new QPushButton("Blue");
    auto* greenBtn = new QPushButton("Green");
    auto* clearBtn = new QPushButton("Clear Canvas");
    auto* saveDrawingBtn = new QPushButton("Save Drawing");
    
    blackBtn->setStyleSheet("background: black; color: white;");
    redBtn->setStyleSheet("background: red; color: white;");
    blueBtn->setStyleSheet("background: blue; color: white;");
    greenBtn->setStyleSheet("background: green; color: white;");
    
    toolbarLayout->addWidget(penWidthLabel);
    toolbarLayout->addWidget(penWidthSlider);
    toolbarLayout->addWidget(colorLabel);
    toolbarLayout->addWidget(blackBtn);
    toolbarLayout->addWidget(redBtn);
    toolbarLayout->addWidget(blueBtn);
    toolbarLayout->addWidget(greenBtn);
    toolbarLayout->addWidget(clearBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(saveDrawingBtn);
    
    drawingLayout->addWidget(toolbar);
    
    // Canvas
    drawingCanvas_ = new DrawingCanvas(drawingTab);
    drawingLayout->addWidget(drawingCanvas_);
    
    // Load existing drawing if any
    if (!task->getDrawingData().empty()) {
        drawingCanvas_->fromBase64(QString::fromStdString(task->getDrawingData()));
    }
    
    // Connect toolbar controls
    connect(penWidthSlider, &QSlider::valueChanged, drawingCanvas_, &DrawingCanvas::setPenWidth);
    connect(blackBtn, &QPushButton::clicked, [this]() { drawingCanvas_->setPenColor(Qt::black); });
    connect(redBtn, &QPushButton::clicked, [this]() { drawingCanvas_->setPenColor(Qt::red); });
    connect(blueBtn, &QPushButton::clicked, [this]() { drawingCanvas_->setPenColor(Qt::blue); });
    connect(greenBtn, &QPushButton::clicked, [this]() { drawingCanvas_->setPenColor(Qt::green); });
    connect(clearBtn, &QPushButton::clicked, drawingCanvas_, &DrawingCanvas::clearCanvas);
    connect(saveDrawingBtn, &QPushButton::clicked, this, &MainWindow::saveDrawing);
    
    // Add or replace drawing tab
    int drawingTabIndex = -1;
    for (int i = 0; i < ui->workspaceTabs->count(); ++i) {
        if (ui->workspaceTabs->tabText(i) == "Drawing") {
            drawingTabIndex = i;
            break;
        }
    }
    
    if (drawingTabIndex >= 0) {
        ui->workspaceTabs->removeTab(drawingTabIndex);
    }
    
    ui->workspaceTabs->addTab(drawingTab, "Drawing");
}

/**
 * @brief Saves the current drawing to the task
 */
void MainWindow::saveDrawing()
{
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask || !drawingCanvas_) {
        QMessageBox::warning(this, "Save Drawing", "No task is currently open.");
        return;
    }
    
    QString drawingData = drawingCanvas_->toBase64();
    dashboardManager_->setTaskDrawingData(QString::fromStdString(currentTask->id()), drawingData);
    
    QMessageBox::information(this, "Save Drawing", "Drawing saved successfully!");
}

/**
 * @brief Sets up the calendar view in the dashboard
 */
void MainWindow::setupCalendarView()
{
    // Create calendar widget
    calendarWidget_ = new TaskCalendar(dashboardManager_.get(), this);
    
    // Connect calendar task selection to open task
    connect(calendarWidget_, &TaskCalendar::taskSelected, this, &MainWindow::openTask);
    
    // Add calendar as a new page in the main stack
    ui->mainStack->addWidget(calendarWidget_);
    
    // Add navigation button for calendar
    QPushButton* btnCalendar = new QPushButton("📅 Calendar", ui->sidebar);
    btnCalendar->setStyleSheet(ui->btnDashboard->styleSheet());
    
    // Find the sidebar layout and add the calendar button
    QVBoxLayout* sidebarLayout = qobject_cast<QVBoxLayout*>(ui->sidebar->layout());
    if (sidebarLayout) {
        // Insert after Stats button
        int insertIndex = 3; // After Dashboard, Tasks, Stats
        sidebarLayout->insertWidget(insertIndex, btnCalendar);
    }
    
    connect(btnCalendar, &QPushButton::clicked, this, [this]() {
        if (calendarWidget_) {
            calendarWidget_->refreshCalendar();
        }
        ui->mainStack->setCurrentWidget(calendarWidget_);
    });
    
    // Connect dashboard manager updates to calendar refresh
    connect(dashboardManager_.get(), &DashboardManager::tasksUpdated,
            this, [this]() {
                if (calendarWidget_) {
                    calendarWidget_->refreshCalendar();
                }
            });
}

/**
 * @brief Sets up the task selector view
 */
void MainWindow::setupTaskSelector()
{
    // Create task selector widget
    taskSelector_ = new TaskSelector(dashboardManager_.get(), this);
    
    // Connect task selection to open task
    connect(taskSelector_, &TaskSelector::taskSelected, this, &MainWindow::openTask);
    
    // Add task selector as a page in the main stack
    ui->mainStack->addWidget(taskSelector_);
    
    // Connect dashboard manager updates to task selector refresh
    connect(dashboardManager_.get(), &DashboardManager::tasksUpdated,
            this, [this]() {
                if (taskSelector_) {
                    taskSelector_->refreshTaskList();
                }
            });
}

/**
 * @brief Updates the completion status of the current task
 * @param completed Whether the task is completed
 */
void MainWindow::updateTaskCompletion(bool completed)
{
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) return;
    
    dashboardManager_->setTaskCompleted(QString::fromStdString(currentTask->id()), completed);
    
    // Refresh dashboard to show updated completion status
    refreshDashboard();
    
    // Refresh calendar
    if (calendarWidget_) {
        calendarWidget_->refreshCalendar();
    }
    
    QString message = completed ? "Task marked as completed!" : "Task marked as incomplete.";
    statusBar()->showMessage(message, 3000);
}

/**
 * @brief Updates the deadline of the current task
 * @param date The new deadline date
 */
void MainWindow::updateDeadline(const QDate& date)
{
    Task* currentTask = dashboardManager_->getCurrentTask();
    if (!currentTask) return;
    
    currentTask->setDeadline(date);
    dashboardManager_->saveToJson();
    
    // Refresh calendar to show updated deadline
    if (calendarWidget_) {
        calendarWidget_->refreshCalendar();
    }
    
    // Refresh dashboard
    refreshDashboard();
    
    statusBar()->showMessage(QString("Deadline set to %1").arg(date.toString("MMM dd, yyyy")), 3000);
}
