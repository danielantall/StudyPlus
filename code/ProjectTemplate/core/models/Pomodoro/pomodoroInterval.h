#ifndef POMODOROINTERVAL_H
#define POMODOROINTERVAL_H

#include "intervalRecord.h"
#include <QObject>
#include <QTimer>

class PomodoroInterval : public QObject {
    Q_OBJECT
public:
    explicit PomodoroInterval(int intervalId, IntervalType type, int durationMinutes, QObject *parent = nullptr);
    ~PomodoroInterval();

    //Getters
    int getIntervalId() const noexcept { return intervalId; }
    IntervalType getType() const noexcept { return type; }
    int getDurationMinutes() const noexcept { return durationMinutes; }
    QDateTime getStartTime() const noexcept { return startTime; }
    int getRemainingSeconds() const noexcept {return durationMinutes * 60 - elapsedTimeSeconds;};

    //timer functions
    void start();
    void pause();
    void resume();
    bool isActive() const;

signals:
    void completed();
    void tick(int remainingSeconds);

private slots:
    // recieves a signal from the Qtimer
    void onTimerTick();

private:

    //Interval Metadata
    const int intervalId;
    const IntervalType type;
    const int durationMinutes;
    const QDateTime startTime;

    //interval timer
    QTimer* timer = nullptr;

    //timer data - allows for pause and resumes
    int elapsedTimeSeconds = 0;
    int pausedAtSeconds = 0;
};

#endif // POMODOROINTERVAL_H
