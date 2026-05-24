#ifndef POMODOROSESSIONMANAGER_H
#define POMODOROSESSIONMANAGER_H
#include "../Task.h"
#include "pomodoroInterval.h"
#include "intervalRecord.h"
#include "pomodoroSessionRecord.h"

class PomodoroSessionManager : public QObject {
    Q_OBJECT

public:
    /*
     * 2 constructors allows for pomodoro configurations
     */
    explicit PomodoroSessionManager(Task* task, int sessionId, QObject *parent = nullptr);
    explicit PomodoroSessionManager(Task* task, int sessionId,int focusMins,int shortBreakMins ,int longBreakMins,int focusIntervalsBeforeLongBreak, QObject *parent = nullptr);

    // Session lifecycle
    void start();              // Start the entire Pomodoro session
    void end();

    // Interval control
    void pauseCurrentInterval();
    void resumeCurrentInterval();
    void skipToNextInterval();

    // Getters
    const Task* getTask() const { return task_; }
    PomodoroInterval* getCurrentInterval() const { return currentInterval; }
    std::vector<IntervalRecord> getCompletedIntervals() const { return completedIntervals; }
    int getSessionId() const { return sessionId_; }
    QDateTime getSessionStartTime() const { return sessionStartTime; }

    //UI Queries
    int getRemainingSeconds() const;
    bool isActive() const;

    // Configuration
    int getFocusMins() const { return focusMins_; }
    int getShortBreakMins() const { return shortBreakMins_; }
    int getLongBreakMins() const { return longBreakMins_; }

    void setFocusMins(int mins) { focusMins_ = mins; }
    void setShortBreakMins(int mins) { shortBreakMins_ = mins; }
    void setLongBreakMins(int mins) { longBreakMins_ = mins; }
    void startNextInterval();

signals:
    void tick(int remainingSeconds);
    void sessionEnded(const SessionRecord& record);
    void intervalStarted(IntervalType type);
    void intervalCompleted(const IntervalRecord& record);

private slots:
    void onIntervalCompleted();
    void onTimerTick(int remainingSeconds);

private:
    IntervalType determineNextIntervalType();
    IntervalRecord createIntervalRecord(PomodoroInterval *interval);
    SessionRecord createSessionRecord();

    // Core data
    const int sessionId_;
    const Task* task_;
    QDateTime sessionStartTime;

    // Interval management
    PomodoroInterval* currentInterval = nullptr;
    std::vector<IntervalRecord> completedIntervals;

    // Configuration
    int focusMins_ = 25;
    int shortBreakMins_ = 5;
    int longBreakMins_ = 15;
    int focusIntervalsBeforeLongBreak_ = 4;
};

#endif // POMODOROSESSIONMANAGER_H
