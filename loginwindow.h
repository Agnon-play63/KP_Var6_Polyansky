#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess(bool isTrainer, int userId);

private slots:
    void onLogin();
    void onRegister();

private:
    QLineEdit     *editEmail;
    QLineEdit     *editPassword;
    QRadioButton  *radioTrainer;
    QRadioButton  *radioClient;
    QButtonGroup  *roleGroup;
    QPushButton   *btnLogin;
    QPushButton   *btnRegister;
    QLabel        *labelTitle;
    QLabel        *labelStatus;
};

#endif // LOGINWINDOW_H
