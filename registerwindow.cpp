#include "registerwindow.h"
#include "loginwindow.h"
#include "database.h"

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Регистрация");
    setFixedSize(420, 560);

    // Выбор роли
    QLabel *labelRole = new QLabel("Кто вы?");
    labelRole->setAlignment(Qt::AlignCenter);
    labelRole->setStyleSheet("font-weight: bold;");

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

    // Общие поля
    editEmail    = new QLineEdit();
    editPassword = new QLineEdit();
    editPassword->setEchoMode(QLineEdit::Password);

    // === ПОЛЯ КЛИЕНТА ===
    clientFields = new QWidget();
    QFormLayout *clientForm = new QFormLayout(clientFields);
    editName    = new QLineEdit();
    editSurname = new QLineEdit();
    editPhone   = new QLineEdit();
    editBirthDate = new QDateEdit(QDate::currentDate().addYears(-20));
    editBirthDate->setCalendarPopup(true);
    editBirthDate->setDisplayFormat("dd.MM.yyyy");
    comboGender = new QComboBox();
    comboGender->addItems({"M", "F"});
    spinWeight  = new QDoubleSpinBox();
    spinWeight->setRange(20, 300); spinWeight->setValue(70); spinWeight->setDecimals(1);
    spinWeight->setSuffix(" кг");
    spinHeight  = new QSpinBox();
    spinHeight->setRange(100, 250); spinHeight->setValue(170);
    spinHeight->setSuffix(" см");
    comboTrainer = new QComboBox();
    QSqlQuery qTrainer(Database::getDb());
    qTrainer.exec("SELECT id_trainer, surname_trainer FROM trainer ORDER BY surname_trainer");
    while (qTrainer.next()) comboTrainer->addItem(qTrainer.value(1).toString(), qTrainer.value(0).toInt());

    clientForm->addRow("Имя:", editName);
    clientForm->addRow("Фамилия:", editSurname);
    clientForm->addRow("Телефон:", editPhone);
    clientForm->addRow("Дата рождения:", editBirthDate);
    clientForm->addRow("Пол:", comboGender);
    clientForm->addRow("Вес:", spinWeight);
    clientForm->addRow("Рост:", spinHeight);
    clientForm->addRow("Тренер:", comboTrainer);

    // === ПОЛЯ ТРЕНЕРА ===
    trainerFields = new QWidget();
    QFormLayout *trainerForm = new QFormLayout(trainerFields);
    editTrainerName    = new QLineEdit();
    editTrainerSurname = new QLineEdit();
    trainerForm->addRow("Имя:", editTrainerName);
    trainerForm->addRow("Фамилия:", editTrainerSurname);

    // Стек для переключения полей
    stackedFields = new QStackedWidget();
    stackedFields->addWidget(clientFields);
    stackedFields->addWidget(trainerFields);

    // Кнопки
    btnRegister = new QPushButton("Зарегистрироваться");
    btnRegister->setStyleSheet("font-weight: bold; padding: 8px;");
    btnBack = new QPushButton("← Назад к входу");
    labelStatus = new QLabel("");
    labelStatus->setAlignment(Qt::AlignCenter);
    labelStatus->setStyleSheet("color: red;");

    // Компоновка
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(labelRole);
    mainLayout->addLayout(roleLayout);
    mainLayout->addSpacing(5);

    QFormLayout *commonForm = new QFormLayout();
    commonForm->addRow("Email:", editEmail);
    commonForm->addRow("Пароль:", editPassword);
    mainLayout->addLayout(commonForm);
    mainLayout->addWidget(stackedFields);
    mainLayout->addWidget(labelStatus);
    mainLayout->addWidget(btnRegister);
    mainLayout->addWidget(btnBack);
    mainLayout->addStretch();

    // Сигналы
    connect(roleGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &RegisterWindow::onRoleChanged);
    connect(btnRegister, &QPushButton::clicked, this, &RegisterWindow::onRegister);
    connect(btnBack,     &QPushButton::clicked, this, &RegisterWindow::onBack);
}

RegisterWindow::~RegisterWindow() {}

void RegisterWindow::onRoleChanged(int id)
{
    stackedFields->setCurrentIndex(id); // 0 — клиент, 1 — тренер
}

void RegisterWindow::onRegister()
{
    QString email    = editEmail->text().trimmed();
    QString password = editPassword->text();
    bool isTrainer   = radioTrainer->isChecked();

    if (email.isEmpty() || password.isEmpty()) {
        labelStatus->setText("Заполните email и пароль");
        return;
    }

    if (password.length() < 6) {
        labelStatus->setText("Пароль должен быть не менее 6 символов");
        return;
    }

    QSqlQuery query(Database::getDb());

    if (isTrainer) {
        // Регистрация тренера
        QString name = editTrainerName->text().trimmed();
        QString surname = editTrainerSurname->text().trimmed();

        if (name.isEmpty() || surname.isEmpty()) {
            labelStatus->setText("Заполните имя и фамилию");
            return;
        }

        // Проверка уникальности email
        QSqlQuery check(Database::getDb());
        check.prepare("SELECT id_trainer FROM trainer WHERE email_trainer = :email");
        check.bindValue(":email", email);
        check.exec();
        if (check.next()) {
            labelStatus->setText("Тренер с таким email уже существует");
            return;
        }

        query.prepare("INSERT INTO trainer (name_trainer, surname_trainer, email_trainer, password_trainer) "
                      "VALUES (:n, :s, :e, :p)");
        query.bindValue(":n", name);
        query.bindValue(":s", surname);
        query.bindValue(":e", email);
        query.bindValue(":p", password);
    } else {
        // Регистрация клиента
        QString name    = editName->text().trimmed();
        QString surname = editSurname->text().trimmed();
        QString phone   = editPhone->text().trimmed();

        if (name.isEmpty() || surname.isEmpty() || phone.isEmpty()) {
            labelStatus->setText("Заполните все обязательные поля");
            return;
        }

        // Проверка уникальности email
        QSqlQuery check(Database::getDb());
        check.prepare("SELECT id_client FROM client WHERE email_client = :email");
        check.bindValue(":email", email);
        check.exec();
        if (check.next()) {
            labelStatus->setText("Клиент с таким email уже существует");
            return;
        }

        query.prepare("INSERT INTO client (name_client, surname_client, number_client, email_client, "
                      "date_of_birth_client, gender_client, kg_client, cm_client, password_client, id_trainer, coach_client) "
                      "VALUES (:n, :s, :p, :e, :b, :g, :w, :h, :pw, :tid, "
                      "(SELECT surname_trainer FROM trainer WHERE id_trainer = :tid))");
        query.bindValue(":n",   name);
        query.bindValue(":s",   surname);
        query.bindValue(":p",   phone);
        query.bindValue(":e",   email);
        query.bindValue(":b",   editBirthDate->date());
        query.bindValue(":g",   comboGender->currentText());
        query.bindValue(":w",   spinWeight->value());
        query.bindValue(":h",   spinHeight->value());
        query.bindValue(":pw",  password);
        query.bindValue(":tid", comboTrainer->currentData().toInt());
    }

    if (!query.exec()) {
        labelStatus->setText("Ошибка: " + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Успех", "Регистрация прошла успешно!\nТеперь войдите под своим email.");
    onBack();
}

void RegisterWindow::onBack()
{
    LoginWindow *login = new LoginWindow();
    login->show();
    this->close();
}
