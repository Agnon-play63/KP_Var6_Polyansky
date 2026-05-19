#include "loginwindow.h"
#include "registerwindow.h"
#include "database.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Авторизация");
    setFixedSize(380, 320);

    labelTitle = new QLabel("Трекер спортивных достижений");
    labelTitle->setAlignment(Qt::AlignCenter);
    labelTitle->setStyleSheet("font-size: 16px; font-weight: bold;");

    editEmail    = new QLineEdit();
    editEmail->setPlaceholderText("Email");

    editPassword = new QLineEdit();
    editPassword->setPlaceholderText("Пароль");
    editPassword->setEchoMode(QLineEdit::Password);

    // Выбор роли
    QLabel *labelRole = new QLabel("Кто вы?");
    labelRole->setAlignment(Qt::AlignCenter);

    radioClient  = new QRadioButton("Клиент");
    radioTrainer = new QRadioButton("Тренер");
    radioClient->setChecked(true);

    roleGroup = new QButtonGroup(this);
    roleGroup->addButton(radioClient, 0);
    roleGroup->addButton(radioTrainer, 1);

    QHBoxLayout *roleLayout = new QHBoxLayout();
    roleLayout->addStretch();
    roleLayout->addWidget(radioClient);
    roleLayout->addWidget(radioTrainer);
    roleLayout->addStretch();

    btnLogin    = new QPushButton("Войти");
    btnRegister = new QPushButton("Регистрация");

    btnLogin->setStyleSheet("font-weight: bold; padding: 6px;");
    btnRegister->setStyleSheet("padding: 6px;");

    labelStatus = new QLabel("");
    labelStatus->setAlignment(Qt::AlignCenter);
    labelStatus->setStyleSheet("color: red;");

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(labelTitle);
    verticalLayout->addSpacing(10);
    verticalLayout->addWidget(editEmail);
    verticalLayout->addWidget(editPassword);
    verticalLayout->addSpacing(5);
    verticalLayout->addWidget(labelRole);
    verticalLayout->addLayout(roleLayout);
    verticalLayout->addSpacing(5);
    verticalLayout->addWidget(labelStatus);
    verticalLayout->addSpacing(5);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(btnLogin);
    buttonLayout->addWidget(btnRegister);
    verticalLayout->addLayout(buttonLayout);

    verticalLayout->addStretch();

    connect(btnLogin,     &QPushButton::clicked, this, &LoginWindow::onLogin);
    connect(btnRegister,  &QPushButton::clicked, this, &LoginWindow::onRegister);
    connect(editPassword, &QLineEdit::returnPressed, this, &LoginWindow::onLogin);
}

LoginWindow::~LoginWindow() {}

void LoginWindow::onLogin()
{
    QString email    = editEmail->text().trimmed();
    QString password = editPassword->text();
    bool isTrainer   = radioTrainer->isChecked();

    if (email.isEmpty() || password.isEmpty()) {
        labelStatus->setText("Заполните все поля");
        return;
    }

    QSqlQuery query(Database::getDb());

    if (isTrainer) {
        // Проверка в таблице trainer
        query.prepare("SELECT id_trainer, email_trainer FROM trainer "
                      "WHERE email_trainer = :email AND password_trainer = :password");
    } else {
        // Проверка в таблице client
        query.prepare("SELECT id_client, email_client FROM client "
                      "WHERE email_client = :email AND password_client = :password");
    }

    query.bindValue(":email",    email);
    query.bindValue(":password", password);

    if (!query.exec() || !query.next()) {
        labelStatus->setText("Неверный email или пароль");
        return;
    }

    Database::currentUserId    = query.value(0).toInt();
    Database::currentUserEmail = query.value(1).toString();

    emit loginSuccess(isTrainer, Database::currentUserId);
    this->close();
}

void LoginWindow::onRegister()
{
    RegisterWindow *regWindow = new RegisterWindow();
    regWindow->show();
    this->close();
}
