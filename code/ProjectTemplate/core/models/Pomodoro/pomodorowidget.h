// PomodoroWidget.h
#ifndef POMODOROWIDGET_H
#define POMODOROWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QPushButton>
#include "pomodorosessionmanager.h"

class PomodoroWidget : public QWidget {
    Q_OBJECT

public:
    explicit PomodoroWidget(PomodoroSessionManager* manager, QWidget *parent = nullptr);

private slots:
    void onTick(int remainingSeconds);
    void onIntervalStarted(IntervalType type);
    void onIntervalCompleted(const IntervalRecord& record);
    void updateUI();
    void onStartClicked();
    void onPauseClicked();
    void onSkipClicked();
    void onEndClicked();

private:
    QString formatTime(int seconds);
    QString getIntervalTypeName(IntervalType type);

    PomodoroSessionManager* manager;
    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);


    // UI elements
    QLabel* timerLabel;
    QLabel* intervalTypeLabel;
    QPushButton* startButton;
    QPushButton* pauseButton;
    QPushButton* skipButton;
    QPushButton* endButton;
};

#endif
