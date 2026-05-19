QT       += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = SportTracker
TEMPLATE = app

SOURCES += \
    main.cpp \
    database.cpp \
    loginwindow.cpp \
    registerwindow.cpp \
    mainwindow.cpp \
    recordswindow.cpp \
    clientswindow.cpp \
    workoutswindow.cpp \
    workoutexerciseswindow.cpp \
    exerciseswindow.cpp

HEADERS += \
    database.h \
    loginwindow.h \
    registerwindow.h \
    mainwindow.h \
    recordswindow.h \
    clientswindow.h \
    workoutswindow.h \
    workoutexerciseswindow.h \
    exerciseswindow.h

FORMS += \
    mainwindow.ui \
    clientswindow.ui \
    workoutswindow.ui \
    workoutexerciseswindow.ui \
    exerciseswindow.ui
