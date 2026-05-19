#include <QApplication>
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"

// Указатели
static LoginWindow *loginWindow = nullptr;
static MainWindow  *mainWindow  = nullptr;

static void showLogin();
static void onLoginSuccess(bool isTrainer, int userId);
static void onLogout();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    if (!Database::connect()) {
        return -1;
    }

    showLogin();

    int result = a.exec();
    Database::close();
    return result;
}

void showLogin()
{
    if (mainWindow) {
        mainWindow->deleteLater();
        mainWindow = nullptr;
    }

    loginWindow = new LoginWindow();

    QObject::connect(loginWindow, &LoginWindow::loginSuccess, &onLoginSuccess);

    loginWindow->show();
}

void onLoginSuccess(bool isTrainer, int userId)
{
    mainWindow = new MainWindow(isTrainer, userId);

    QObject::connect(mainWindow, &MainWindow::logout, &onLogout);

    mainWindow->show();
    loginWindow->close();
}

void onLogout()
{
    if (mainWindow) {
        mainWindow->deleteLater();
        mainWindow = nullptr;
    }

    loginWindow = new LoginWindow();

    QObject::connect(loginWindow, &LoginWindow::loginSuccess, &onLoginSuccess);

    loginWindow->show();
}
