#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logout();

private slots:
    void on_actionSave_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionRecords_triggered();
    void on_actionClients_triggered();
    void on_actionWorkouts_triggered();
    void on_actionWorkoutExercises_triggered();
    void on_actionExercises_triggered();

    void onClientDoubleClicked(int clientId, const QString &clientName);
    void onWorkoutDoubleClicked(int sessionId);

private:
    Ui::MainWindow *ui;
    bool m_isTrainer;
    int  m_userId;
};

#endif // MAINWINDOW_H
