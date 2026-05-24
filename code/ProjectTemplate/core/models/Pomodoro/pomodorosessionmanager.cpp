#include "pomodorosessionmanager.h"

/**
 * @brief Constructs a PomodoroSessionManager with default interval durations.
 * @param task The associated Task this session belongs to.
 * @param sessionId Unique session identifier.
 * @param parent Optional QObject parent.
 */
PomodoroSessionManager::PomodoroSessionManager(Task* task, int sessionId, QObject *parent)
    : QObject{parent},
    sessionId_(sessionId),
    task_(task)
{}

/**
 * @brief Constructs a PomodoroSessionManager with custom Pomodoro settings.
 * @param task The associated Task this session belongs to.
 * @param sessionId Unique session identifier.
 * @param focusMins Focus interval duration in minutes.
 * @param shortBreakMins Short break duration in minutes.
 * @param longBreakMins Long break duration in minutes.
 * @param focusIntervalsBeforeLongBreak Number of focus sessions before a long break.
 * @param parent Optional QObject parent.
 */
PomodoroSessionManager::PomodoroSessionManager(Task* task, int sessionId,
                                               int focusMins, int shortBreakMins,
                                               int longBreakMins, int focusIntervalsBeforeLongBreak,
                                               QObject *parent)
    : QObject{parent},
    sessionId_(sessionId),
    task_(task),
    focusMins_(focusMins),
    shortBreakMins_(shortBreakMins),
    longBreakMins_(longBreakMins),
    focusIntervalsBeforeLongBreak_(focusIntervalsBeforeLongBreak)
{}

/**
 * @brief Starts the Pomodoro session with an initial focus interval.
 */
void PomodoroSessionManager::start() {
    sessionStartTime = QDateTime::currentDateTime();
    currentInterval = new PomodoroInterval(1, IntervalType::FOCUS, focusMins_, this);

    connect(currentInterval, &PomodoroInterval::tick, this, &PomodoroSessionManager::onTimerTick);
    connect(currentInterval, &PomodoroInterval::completed, this, &PomodoroSessionManager::onIntervalCompleted);

    currentInterval->start();
    emit intervalStarted(IntervalType::FOCUS);
}

/**
 * @brief Returns the remaining time in seconds for the current interval.
 */
int PomodoroSessionManager::getRemainingSeconds() const {
    return currentInterval ? currentInterval->getRemainingSeconds() : 0;
}

/**
 * @brief Checks if a Pomodoro interval is currently active.
 */
bool PomodoroSessionManager::isActive() const {
    return currentInterval && currentInterval->isActive();
}

/**
 * @brief Ends the Pomodoro session, cleans up the current interval, and emits a session record.
 */
void PomodoroSessionManager::end() {
    if (currentInterval != nullptr) currentInterval->deleteLater();
    currentInterval = nullptr;

    SessionRecord record = createSessionRecord();
    emit sessionEnded(record);
}

/**
 * @brief Handles the tick signal from PomodoroInterval.
 * @param remainingSeconds Time left in the current interval.
 */
void PomodoroSessionManager::onTimerTick(int remainingSeconds) {
    emit tick(remainingSeconds);
}

/**
 * @brief Called when the current interval completes.
 * Creates an IntervalRecord, emits completion, and prepares for the next interval.
 */
void PomodoroSessionManager::onIntervalCompleted() {
    IntervalRecord record = createIntervalRecord(currentInterval);
    emit intervalCompleted(record);
    completedIntervals.push_back(record);

    currentInterval->deleteLater();
    currentInterval = nullptr;
}

/**
 * @brief Starts the next Pomodoro interval based on the previous one.
 */
void PomodoroSessionManager::startNextInterval() {
    IntervalType type = determineNextIntervalType();
    int id = completedIntervals.back().intervalId + 1;

    switch (type) {
    case IntervalType::FOCUS:
        currentInterval = new PomodoroInterval(id, type, focusMins_, this);
        break;
    case IntervalType::SHORT_BREAK:
        currentInterval = new PomodoroInterval(id, type, shortBreakMins_, this);
        break;
    case IntervalType::LONG_BREAK:
        currentInterval = new PomodoroInterval(id, type, longBreakMins_, this);
        break;
    }

    connect(currentInterval, &PomodoroInterval::tick, this, &PomodoroSessionManager::onTimerTick);
    connect(currentInterval, &PomodoroInterval::completed, this, &PomodoroSessionManager::onIntervalCompleted);

    currentInterval->start();
    emit intervalStarted(type);
}

/**
 * @brief Determines what the next interval type should be (focus, short break, or long break).
 */
IntervalType PomodoroSessionManager::determineNextIntervalType() {
    IntervalRecord interval = completedIntervals.back();

    if (interval.type == IntervalType::LONG_BREAK || interval.type == IntervalType::SHORT_BREAK)
        return IntervalType::FOCUS;

    int numOfFocus = 0;
    for (const IntervalRecord &i : completedIntervals)
        if (i.type == IntervalType::FOCUS) numOfFocus++;

    return (numOfFocus % focusIntervalsBeforeLongBreak_ == 0)
               ? IntervalType::LONG_BREAK
               : IntervalType::SHORT_BREAK;
}

/**
 * @brief Creates a record for a completed interval.
 */
IntervalRecord PomodoroSessionManager::createIntervalRecord(PomodoroInterval *interval) {
    return {
        interval->getIntervalId(),
        interval->getType(),
        interval->getDurationMinutes(),
        interval->getStartTime(),
        QDateTime::currentDateTime()
    };
}

/**
 * @brief Creates a record for the completed Pomodoro session.
 */
SessionRecord PomodoroSessionManager::createSessionRecord() {
    return {
        sessionId_,
        task_->id(),
        sessionStartTime,
        QDateTime::currentDateTime(),
        completedIntervals
    };
}

/**
 * @brief Pauses the current interval timer.
 */
void PomodoroSessionManager::pauseCurrentInterval() {
    if (currentInterval) currentInterval->pause();
}

/**
 * @brief Resumes a paused interval timer.
 */
void PomodoroSessionManager::resumeCurrentInterval() {
    if (currentInterval) currentInterval->resume();
}

/**
 * @brief Skips to the next interval immediately.
 */
void PomodoroSessionManager::skipToNextInterval() {
    onIntervalCompleted();
}
