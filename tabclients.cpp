#include "tabclients.h"
#include "database.h"

TabClients::TabClients(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent), m_isTrainer(isTrainer), m_userId(userId)
{
    QLabel *labelFilter = new QLabel("Поиск по тренеру:");

    editFilter = new QLineEdit();
    editFilter->setPlaceholderText("Введите фамилию тренера...");

    btnFilter = new QPushButton("🔍 Найти");
    btnAdd    = new QPushButton("➕ Добавить");
    btnAdd->setStyleSheet("font-weight: bold;");
    btnEdit   = new QPushButton("✏ Изменить");
    btnDelete = new QPushButton("🗑 Удалить");

    tableView = new QTableView();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);

    model = new QSqlTableModel(this, Database::getDb());
    model->setTable("client");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Имя");
    model->setHeaderData(2, Qt::Horizontal, "Фамилия");
    model->setHeaderData(3, Qt::Horizontal, "Телефон");
    model->setHeaderData(4, Qt::Horizontal, "Email");
    model->setHeaderData(5, Qt::Horizontal, "Дата рег.");
    model->setHeaderData(6, Qt::Horizontal, "Дата рожд.");
    model->setHeaderData(7, Qt::Horizontal, "Пол");
    model->setHeaderData(8, Qt::Horizontal, "Вес");
    model->setHeaderData(9, Qt::Horizontal, "Рост");
    model->setHeaderData(10, Qt::Horizontal, "Пароль");
    model->setHeaderData(11, Qt::Horizontal, "Тренер");
    model->setHeaderData(12, Qt::Horizontal, "id_trainer");

    tableView->setModel(model);
    // Скрыть все, кроме: Имя(1), Фамилия(2), Телефон(3), Тренер(11)
    tableView->hideColumn(0);   // id
    tableView->hideColumn(4);   // email
    tableView->hideColumn(5);   // дата рег
    tableView->hideColumn(6);   // дата рожд
    tableView->hideColumn(7);   // пол
    tableView->hideColumn(8);   // вес
    tableView->hideColumn(9);   // рост
    tableView->hideColumn(10);  // пароль
    tableView->hideColumn(12);  // id_trainer

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(labelFilter);
    topLayout->addWidget(editFilter);
    topLayout->addWidget(btnFilter);
    if (m_isTrainer) {
        topLayout->addWidget(btnAdd);
    }

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnEdit);
    bottomLayout->addWidget(btnDelete);

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->addLayout(topLayout);
    verticalLayout->addWidget(tableView);
    verticalLayout->addLayout(bottomLayout);

    connect(btnFilter, &QPushButton::clicked, this, &TabClients::onFilterByCoach);
    connect(btnAdd,    &QPushButton::clicked, this, &TabClients::onAdd);
    connect(btnEdit,   &QPushButton::clicked, this, &TabClients::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &TabClients::onDelete);

    if (!m_isTrainer) {
        model->setFilter(QString("id_client = %1").arg(m_userId));
        labelFilter->setVisible(false);
        editFilter->setVisible(false);
        btnFilter->setVisible(false);
        btnAdd->setVisible(false);
    }

    loadData();
}

TabClients::~TabClients() {}

void TabClients::saveChanges()
{
    if (model->isDirty()) {
        model->submitAll();
    }
}

int TabClients::selectedId()
{
    QModelIndex idx = tableView->currentIndex();
    if (!idx.isValid()) return -1;
    return model->data(model->index(idx.row(), 0)).toInt();
}

void TabClients::loadData()
{
    model->select();
}

void TabClients::onFilterByCoach()
{
    QString coach = editFilter->text().trimmed();
    if (coach.isEmpty()) {
        model->setFilter("");
    } else {
        model->setFilter(QString("coach_client ILIKE '%%1%'").arg(coach));
    }
    model->select();
}

void TabClients::onAdd()
{
    if (!m_isTrainer) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Добавить клиента");
    dialog.setFixedSize(350, 520);

    QFormLayout form;
    QLineEdit *name     = new QLineEdit();
    QLineEdit *surname  = new QLineEdit();
    QLineEdit *phone    = new QLineEdit();
    QLineEdit *email    = new QLineEdit();
    QLineEdit *password = new QLineEdit();
    password->setEchoMode(QLineEdit::Password);
    QDateEdit *birth    = new QDateEdit(QDate::currentDate().addYears(-20));
    birth->setCalendarPopup(true);
    QComboBox *gender   = new QComboBox();
    gender->addItems({"M", "F"});
    QDoubleSpinBox *weight = new QDoubleSpinBox();
    weight->setRange(20, 300); weight->setValue(70); weight->setDecimals(1);
    QSpinBox *height = new QSpinBox();
    height->setRange(100, 250); height->setValue(170);

    QComboBox *comboTrainer = new QComboBox();
    QSqlQuery qTrainer(Database::getDb());
    qTrainer.exec("SELECT id_trainer, surname_trainer FROM trainer ORDER BY surname_trainer");
    while (qTrainer.next())
        comboTrainer->addItem(qTrainer.value(1).toString(), qTrainer.value(0).toInt());

    form.addRow("Имя:", name);
    form.addRow("Фамилия:", surname);
    form.addRow("Телефон:", phone);
    form.addRow("Email:", email);
    form.addRow("Пароль:", password);
    form.addRow("Дата рождения:", birth);
    form.addRow("Пол:", gender);
    form.addRow("Вес:", weight);
    form.addRow("Рост:", height);
    form.addRow("Тренер:", comboTrainer);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    dialog.setLayout(&form);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (name->text().trimmed().isEmpty() || surname->text().trimmed().isEmpty() ||
            email->text().trimmed().isEmpty() || password->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля");
            return;
        }

        if (password->text().length() < 6) {
            QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов");
            return;
        }

        QSqlQuery check(Database::getDb());
        check.prepare("SELECT id_client FROM client WHERE email_client = :email");
        check.bindValue(":email", email->text().trimmed());
        check.exec();
        if (check.next()) {
            QMessageBox::warning(this, "Ошибка", "Клиент с таким email уже существует");
            return;
        }

        QSqlQuery q(Database::getDb());
        q.prepare("INSERT INTO client (name_client, surname_client, number_client, email_client, "
                  "date_of_birth_client, gender_client, kg_client, cm_client, password_client, id_trainer, coach_client) "
                  "VALUES (:n, :s, :p, :e, :b, :g, :w, :h, :pw, :tid, "
                  "(SELECT surname_trainer FROM trainer WHERE id_trainer = :tid))");
        q.bindValue(":n",   name->text().trimmed());
        q.bindValue(":s",   surname->text().trimmed());
        q.bindValue(":p",   phone->text().trimmed());
        q.bindValue(":e",   email->text().trimmed());
        q.bindValue(":b",   birth->date());
        q.bindValue(":g",   gender->currentText());
        q.bindValue(":w",   weight->value());
        q.bindValue(":h",   height->value());
        q.bindValue(":pw",  password->text());
        q.bindValue(":tid", comboTrainer->currentData().toInt());

        if (!q.exec())
            QMessageBox::warning(this, "Ошибка", q.lastError().text());
        loadData();
    }
}

void TabClients::onEdit()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите клиента"); return; }

    QSqlQuery q(Database::getDb());
    q.prepare("SELECT * FROM client WHERE id_client = :id");
    q.bindValue(":id", id); q.exec();
    if (!q.next()) return;

    // Индексы: 0=id, 1=name, 2=surname, 3=phone, 4=email, 5=regdate, 6=birth, 7=gender,
    // 8=kg, 9=cm, 10=password, 11=coach, 12=id_trainer

    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать клиента");
    dialog.setFixedSize(350, 400);
    QFormLayout form;

    QLabel *labelName    = new QLabel(q.value(1).toString());
    QLabel *labelSurname = new QLabel(q.value(2).toString());
    QLabel *labelPhone   = new QLabel(q.value(3).toString());
    QLabel *labelEmail   = new QLabel(q.value(4).toString());
    QLabel *labelCoach   = new QLabel(q.value(11).toString());
    QLabel *labelBirth   = new QLabel(q.value(6).toDate().toString("dd.MM.yyyy"));
    QLabel *labelGender  = new QLabel(q.value(7).toString());

    QDoubleSpinBox *weight = new QDoubleSpinBox();
    weight->setRange(20, 300); weight->setValue(q.value(8).toDouble()); weight->setDecimals(1);
    weight->setSuffix(" кг");

    QSpinBox *height = new QSpinBox();
    height->setRange(100, 250); height->setValue(q.value(9).toInt());
    height->setSuffix(" см");

    form.addRow("Имя:",       labelName);
    form.addRow("Фамилия:",   labelSurname);
    form.addRow("Телефон:",   labelPhone);
    form.addRow("Email:",     labelEmail);
    form.addRow("Тренер:",    labelCoach);
    form.addRow("Дата рожд.:", labelBirth);
    form.addRow("Пол:",       labelGender);
    form.addRow("Вес:",       weight);
    form.addRow("Рост:",      height);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    dialog.setLayout(&form);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery upd(Database::getDb());
        upd.prepare("UPDATE client SET kg_client=:w, cm_client=:h WHERE id_client=:id");
        upd.bindValue(":w",  weight->value());
        upd.bindValue(":h",  height->value());
        upd.bindValue(":id", id);
        upd.exec();
        loadData();
    }
}

void TabClients::onDelete()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите клиента"); return; }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                                                              "Удалить клиента и все его тренировки?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery q(Database::getDb());
        q.prepare("DELETE FROM client WHERE id_client = :id");
        q.bindValue(":id", id); q.exec();
        loadData();
    }
}
