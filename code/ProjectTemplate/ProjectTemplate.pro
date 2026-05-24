TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG += qt
QT += core gui widgets multimedia

RESOURCES += \
    resources.qrc

FORMS += \
        ui/mainwindow.ui

SOURCES += \
        core/models/DocumentFactory.h \
        main.cpp \
        core/dashboardmanager.cpp \
        core/models/Task.cpp \
        infra/FileSystemStorage.cpp \
        core/models/Pomodoro/pomodoroInterval.cpp \
        core/models/Pomodoro/pomodorosessionmanager.cpp \
        core/models/Pomodoro/pomodorowidget.cpp \
        core/mainwindow.cpp \
        core/models/DrawingCanvas.cpp \
        core/models/TaskCalendar.cpp \
        core/models/TaskSelector.cpp


HEADERS += \
        core/models/TextDocumentFactory.h \
        core/dashboardmanager.h \
        core/models/Task.h \
        core/models/Document.h \
        core/models/TextDocument.h \
        infra/IStorage.h \
        infra/FileSystemStorage.h \
        core/models/Pomodoro/intervalRecord.h \
        core/models/Pomodoro/pomodoroInterval.h \
        core/models/Pomodoro/pomodoroSessionRecord.h \
        core/models/Pomodoro/pomodorosessionmanager.h \
        core/models/Pomodoro/pomodorowidget.h \
        core/mainwindow.h \
        core/models/DrawingCanvas.h \
        core/models/TaskCalendar.h \
        core/models/TaskSelector.h
