#include "pomodorowidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

/**
 * @brief Constructs the PomodoroWidget which serves as the UI layer for a Pomodoro session.
 * @param manager Pointer to the PomodoroSessionManager that handles session logic.
 * @param parent Optional QWidget parent.
 */
PomodoroWidget::PomodoroWidget(PomodoroSessionManager* manager, QWidget *parent)
    : QWidget(parent),
    manager(manager)
{
    // ===== UI Setup =====
    timerLabel = new QLabel("00:00", this);
    timerLabel->setAlignment(Qt::AlignCenter);
    QFont font = timerLabel->font();
    font.setPointSize(48);
    timerLabel->setFont(font);

    intervalTypeLabel = new QLabel("Ready to focus", this);
    intervalTypeLabel->setAlignment(Qt::AlignCenter);

    startButton = new QPushButton("Start Pomodoro", this);
    pauseButton = new QPushButton("Pause", this);
    skipButton = new QPushButton("Skip", this);
    endButton = new QPushButton("End Session", this);

    // ===== Layout =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(intervalTypeLabel);
    mainLayout->addWidget(timerLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(skipButton);
    buttonLayout->addWidget(endButton);
    mainLayout->addLayout(buttonLayout);

    // ===== Signal Connections =====
    connect(manager, &PomodoroSessionManager::tick,
            this, &PomodoroWidget::onTick);
    connect(manager, &PomodoroSessionManager::intervalStarted,
            this, &PomodoroWidget::onIntervalStarted);
    connect(manager, &PomodoroSessionManager::intervalCompleted,
            this, &PomodoroWidget::onIntervalCompleted);

    connect(startButton, &QPushButton::clicked,
            this, &PomodoroWidget::onStartClicked);
    connect(pauseButton, &QPushButton::clicked,
            this, &PomodoroWidget::onPauseClicked);
    connect(skipButton, &QPushButton::clicked,
            this, &PomodoroWidget::onSkipClicked);
    connect(endButton, &QPushButton::clicked,
            this, &PomodoroWidget::onEndClicked);

    qDebug() << "PomodoroWidget created with manager:" << manager;
    qDebug() << "Manager task:" << manager->getTask();

    updateUI();
}

/**
 * @brief Updates the countdown label each second.
 */
void PomodoroWidget::onTick(int remainingSeconds) {
    timerLabel->setText(formatTime(remainingSeconds));
}

/**
 * @brief Handles the start button click — begins or resumes the session.
 */
void PomodoroWidget::onStartClicked() {
    if (!manager->getCurrentInterval()) {
        if (manager->getCompletedIntervals().empty()) {
            manager->start();
        } else {
            manager->startNextInterval();
        }
    }
    updateUI();
}

/**
 * @brief Toggles between pause and resume.
 */
void PomodoroWidget::onPauseClicked() {
    if (manager->isActive()) {
        manager->pauseCurrentInterval();
        pauseButton->setText("Resume");
    } else {
        manager->resumeCurrentInterval();
        pauseButton->setText("Pause");
    }
    updateUI();
}

/**
 * @brief Skips the current interval and advances to the next one.
 */
void PomodoroWidget::onSkipClicked() {
    manager->skipToNextInterval();
    updateUI();
}

/**
 * @brief Ends the current Pomodoro session entirely.
 */
void PomodoroWidget::onEndClicked() {
    manager->end();
    updateUI();
}

/**
 * @brief Called when a new interval starts.
 * Updates the label to display the interval type.
 */
void PomodoroWidget::onIntervalStarted(IntervalType type) {
    intervalTypeLabel->setText(getIntervalTypeName(type));
    pauseButton->setText("Pause");
    updateUI();
}

/**
 * @brief Called when an interval completes.
 * Plays a notification sound and resets the timer label.
 */
void PomodoroWidget::onIntervalCompleted(const IntervalRecord& record) {
    timerLabel->setText("00:00");
    intervalTypeLabel->setText("Interval Complete!");

    player->setAudioOutput(audioOutput);
    player->setSource(QUrl("qrc:/assets/ring.mp3"));
    player->play();

    updateUI();
}

/**
 * @brief Updates button states and timer display according to session status.
 */
void PomodoroWidget::updateUI() {
    bool hasInterval = manager->getCurrentInterval() != nullptr;

    startButton->setVisible(!hasInterval);
    pauseButton->setVisible(hasInterval);
    skipButton->setVisible(hasInterval);
    endButton->setEnabled(hasInterval || !manager->getCompletedIntervals().empty());

    if (hasInterval) {
        timerLabel->setText(formatTime(manager->getRemainingSeconds()));
    }
}

/**
 * @brief Converts seconds to MM:SS formatted string.
 */
QString PomodoroWidget::formatTime(int seconds) {
    int mins = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2")
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

/**
 * @brief Converts interval type enum to human-readable text.
 */
QString PomodoroWidget::getIntervalTypeName(IntervalType type) {
    switch (type) {
    case IntervalType::FOCUS:       return "Focus Time";
    case IntervalType::SHORT_BREAK:  return "Short Break";
    case IntervalType::LONG_BREAK:   return "Long Break";
    }
    return "";
}
