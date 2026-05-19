#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

private slots:
    void onRegister();
    void onBack();
    void onRoleChanged(int id);

private:
    // Выбор роли
    QRadioButton  *radioClient;
    QRadioButton  *radioTrainer;
    QButtonGroup  *roleGroup;

    // Общие поля
    QLineEdit     *editEmail;
    QLineEdit     *editPassword;

    // Поля клиента
    QStackedWidget *stackedFields;
    QWidget        *clientFields;
    QWidget        *trainerFields;
    QLineEdit      *editName;
    QLineEdit      *editSurname;
    QLineEdit      *editPhone;
    QDateEdit      *editBirthDate;
    QComboBox      *comboGender;
    QDoubleSpinBox *spinWeight;
    QSpinBox       *spinHeight;
    QComboBox      *comboTrainer;

    // Поля тренера
    QLineEdit      *editTrainerName;
    QLineEdit      *editTrainerSurname;

    QPushButton    *btnRegister;
    QPushButton    *btnBack;
    QLabel         *labelStatus;
};

#endif // REGISTERWINDOW_H
