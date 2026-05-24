#include "pomodoroInterval.h"

PomodoroInterval::PomodoroInterval(int intervalId, IntervalType type, int durationMinutes, QObject *parent)
    : QObject(parent)
    , intervalId(intervalId)
    , type(type)
    , durationMinutes(durationMinutes)
    , startTime(QDateTime::currentDateTime())
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PomodoroInterval::onTimerTick);
}

PomodoroInterval::~PomodoroInterval() {
    if (timer && timer->isActive()) {
        timer->stop();
    }

    delete timer;
    timer = nullptr;

    qDebug() << "PomodoroInterval destroyed:" << intervalId;
}

/*
 * returns true if active timer
 * false if otherwise
 */
bool PomodoroInterval::isActive() const {
    return timer->isActive();
}

void PomodoroInterval::start() {
    timer->start(1000);
}

void PomodoroInterval::pause() {
    if (timer->isActive()) {
        pausedAtSeconds = elapsedTimeSeconds;
        timer->stop();
    }
}

void PomodoroInterval::resume() {
    if (!timer->isActive() && pausedAtSeconds > 0) {
        timer->start(1000);
    }
}

/*
 * emits a signal to update the ui
 * stops the timer if the time is up
 */
void PomodoroInterval::onTimerTick() {
    elapsedTimeSeconds++;
    emit tick(getRemainingSeconds());

    if (elapsedTimeSeconds >= durationMinutes * 60) {
        timer->stop();
        emit completed();
    }
}
