#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recordswindow.h"
#include "clientswindow.h"
#include "workoutswindow.h"
#include "workoutexerciseswindow.h"
#include "exerciseswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(bool isTrainer, int userId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isTrainer(isTrainer)
    , m_userId(userId)
{
    ui->setupUi(this);

    resize(550, 400);
    setMinimumSize(450, 350);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    statusBar()->showMessage("Готово");
    QLabel *labelUser = new QLabel("  " + Database::currentUserEmail);
    statusBar()->addPermanentWidget(labelUser);

    connect(ui->btnClients, &QPushButton::clicked, this, &MainWindow::on_actionClients_triggered);
    connect(ui->btnWorkouts, &QPushButton::clicked, this, &MainWindow::on_actionWorkouts_triggered);
    connect(ui->btnWorkoutExercises, &QPushButton::clicked, this, &MainWindow::on_actionWorkoutExercises_triggered);
    connect(ui->btnExercises, &QPushButton::clicked, this, &MainWindow::on_actionExercises_triggered);
    connect(ui->btnRecords, &QPushButton::clicked, this, &MainWindow::on_actionRecords_triggered);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionSave_triggered()
{ statusBar()->showMessage("Изменения сохранены", 3000); }

void MainWindow::on_actionExit_triggered()
{ emit logout(); this->close(); }

void MainWindow::on_actionAbout_triggered()
{ QMessageBox::about(this, "О программе", "Трекер спортивных достижений\nКурсовой проект"); }

void MainWindow::on_actionRecords_triggered()
{ RecordsWindow *w = new RecordsWindow(); w->show(); }

void MainWindow::on_actionClients_triggered()
{
    ClientsWindow *w = new ClientsWindow(m_isTrainer, m_userId);
    if (m_isTrainer) {
        connect(w, &ClientsWindow::clientDoubleClicked, this, &MainWindow::onClientDoubleClicked);
    }
    w->show();
}

void MainWindow::on_actionWorkouts_triggered()
{
    WorkoutsWindow *w = new WorkoutsWindow(m_isTrainer, m_userId);
    connect(w, &WorkoutsWindow::workoutDoubleClicked, this, &MainWindow::onWorkoutDoubleClicked);
    w->show();
}

void MainWindow::on_actionWorkoutExercises_triggered()
{
    WorkoutExercisesWindow *w = new WorkoutExercisesWindow(m_isTrainer, m_userId);
    w->show();
}

void MainWindow::on_actionExercises_triggered()
{ ExercisesWindow *w = new ExercisesWindow(m_isTrainer); w->show(); }

void MainWindow::onClientDoubleClicked(int clientId, const QString &clientName)
{
    WorkoutsWindow *w = new WorkoutsWindow(m_isTrainer, m_userId);
    w->setClientFilter(clientId, clientName);
    w->setWindowTitle("Тренировки — " + clientName);
    connect(w, &WorkoutsWindow::workoutDoubleClicked, this, &MainWindow::onWorkoutDoubleClicked);
    w->show();
}

void MainWindow::onWorkoutDoubleClicked(int sessionId)
{
    WorkoutExercisesWindow *w = new WorkoutExercisesWindow(m_isTrainer, m_userId);
    w->setWorkoutFilter(sessionId);
    w->setWindowTitle("Выполненные упражнения — тренировка #" + QString::number(sessionId));
    w->show();
}
