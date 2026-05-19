#include "tabworkouts.h"
#include "database.h"

TabWorkouts::TabWorkouts(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent), m_isTrainer(isTrainer), m_userId(userId)
{
    QLabel *lblFrom = new QLabel("Дата с:");
    dateEditFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
    dateEditFrom->setCalendarPopup(true);
    dateEditFrom->setDisplayFormat("dd.MM.yyyy");

    QLabel *lblTo = new QLabel("по:");
    dateEditTo = new QDateEdit(QDate::currentDate());
    dateEditTo->setCalendarPopup(true);
    dateEditTo->setDisplayFormat("dd.MM.yyyy");

    QLabel *lblClient = new QLabel("Клиент:");
    comboClientFilter = new QComboBox();

    btnAdd  = new QPushButton("➕ Добавить");
    btnAdd->setStyleSheet("font-weight: bold;");
    btnEdit = new QPushButton("✏ Изменить");
    btnDelete = new QPushButton("🗑 Удалить");

    tableViewWorkouts = new QTableView();
    tableViewWorkouts->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewWorkouts->setSelectionMode(QAbstractItemView::SingleSelection);
    tableViewWorkouts->setAlternatingRowColors(true);

    groupBoxDetails = new QGroupBox("Детали выбранной тренировки");
    tableViewDetails = new QTableView();
    tableViewDetails->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewDetails->setAlternatingRowColors(true);
    QVBoxLayout *detailsLayout = new QVBoxLayout();
    detailsLayout->addWidget(tableViewDetails);
    groupBoxDetails->setLayout(detailsLayout);

    modelWorkouts = new QSqlQueryModel(this);
    modelDetails  = new QSqlQueryModel(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(lblFrom);
    topLayout->addWidget(dateEditFrom);
    topLayout->addWidget(lblTo);
    topLayout->addWidget(dateEditTo);
    if (m_isTrainer) {
        topLayout->addWidget(lblClient);
        topLayout->addWidget(comboClientFilter);
    }
    topLayout->addStretch();
    topLayout->addWidget(btnAdd);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnEdit);
    bottomLayout->addWidget(btnDelete);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableViewWorkouts);
    mainLayout->addWidget(groupBoxDetails);
    mainLayout->addLayout(bottomLayout);

    connect(btnAdd,    &QPushButton::clicked, this, &TabWorkouts::onAdd);
    connect(btnEdit,   &QPushButton::clicked, this, &TabWorkouts::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &TabWorkouts::onDelete);
    connect(comboClientFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        onFilterChanged();
    });
    connect(dateEditFrom, &QDateEdit::dateChanged, this, &TabWorkouts::onFilterChanged);
    connect(dateEditTo,   &QDateEdit::dateChanged, this, &TabWorkouts::onFilterChanged);
    connect(tableViewWorkouts, &QTableView::clicked, this, &TabWorkouts::onWorkoutClicked);

    loadClientFilter();
    loadWorkouts();
}

TabWorkouts::~TabWorkouts() {}

void TabWorkouts::loadClientFilter()
{
    comboClientFilter->blockSignals(true);
    comboClientFilter->clear();
    comboClientFilter->addItem("Все", -1);
    QSqlQuery q(Database::getDb());
    q.exec("SELECT id_client, name_client || ' ' || surname_client FROM client "
           "WHERE email_client NOT LIKE 'trener%' ORDER BY surname_client");
    while (q.next())
        comboClientFilter->addItem(q.value(1).toString(), q.value(0).toInt());
    comboClientFilter->blockSignals(false);
}

int TabWorkouts::selectedId()
{
    QModelIndex idx = tableViewWorkouts->currentIndex();
    if (!idx.isValid()) return -1;
    return modelWorkouts->data(modelWorkouts->index(idx.row(), 0)).toInt();
}

void TabWorkouts::loadWorkouts()
{
    QString filter = "WHERE 1=1";

    if (!m_isTrainer) {
        filter += QString(" AND ws.id_client = %1").arg(m_userId);
    } else {
        int clientId = comboClientFilter->currentData().toInt();
        if (clientId != -1)
            filter += QString(" AND ws.id_client = %1").arg(clientId);
    }

    filter += QString(" AND ws.start_time_sessions::date >= '%1'").arg(dateEditFrom->date().toString("yyyy-MM-dd"));
    filter += QString(" AND ws.start_time_sessions::date <= '%1'").arg(dateEditTo->date().toString("yyyy-MM-dd"));

    QString sql = QString(
                      "SELECT ws.id_sessions, c.name_client || ' ' || c.surname_client, "
                      "ws.title_sessions, ws.start_time_sessions::date, ws.RPE, "
                      "(SELECT COUNT(*) FROM workouts_exercises WHERE id_sessions = ws.id_sessions) "
                      "FROM workout_sessions ws JOIN client c ON ws.id_client = c.id_client "
                      "%1 ORDER BY ws.start_time_sessions DESC").arg(filter);

    modelWorkouts->setQuery(sql, Database::getDb());
    modelWorkouts->setHeaderData(0, Qt::Horizontal, "ID");
    modelWorkouts->setHeaderData(1, Qt::Horizontal, "Клиент");
    modelWorkouts->setHeaderData(2, Qt::Horizontal, "Название");
    modelWorkouts->setHeaderData(3, Qt::Horizontal, "Дата");
    modelWorkouts->setHeaderData(4, Qt::Horizontal, "RPE");
    modelWorkouts->setHeaderData(5, Qt::Horizontal, "Упражнений");
    tableViewWorkouts->setModel(modelWorkouts);
    tableViewWorkouts->hideColumn(0);

    // Испустить сигнал с выбранным клиентом
    if (m_isTrainer) {
        int selectedClient = comboClientFilter->currentData().toInt();
        emit clientSelected(selectedClient);
    }

    // Испустить сигнал с датами
    emit dateFilterChanged(dateEditFrom->date(), dateEditTo->date());
}

void TabWorkouts::loadDetails(int sessionId)
{
    QString sql = QString(
                      "SELECT es.id_sets, e.name_exercises, es.number_set, es.reps, es.weight_kg, "
                      "CASE WHEN es.is_pr THEN '★' ELSE '' END "
                      "FROM exercises_sets es "
                      "JOIN workouts_exercises we ON es.id_workout_exercises = we.id_workout_exercises "
                      "JOIN exercises e ON we.id_exercises = e.id_exercises "
                      "WHERE we.id_sessions = %1 ORDER BY we.order_num_exercises, es.number_set").arg(sessionId);

    modelDetails->setQuery(sql, Database::getDb());
    modelDetails->setHeaderData(0, Qt::Horizontal, "ID");
    modelDetails->setHeaderData(1, Qt::Horizontal, "Упражнение");
    modelDetails->setHeaderData(2, Qt::Horizontal, "Подход");
    modelDetails->setHeaderData(3, Qt::Horizontal, "Повторений");
    modelDetails->setHeaderData(4, Qt::Horizontal, "Вес");
    modelDetails->setHeaderData(5, Qt::Horizontal, "PR");
    tableViewDetails->setModel(modelDetails);
    tableViewDetails->hideColumn(0);
}

void TabWorkouts::onFilterChanged() { loadWorkouts(); }

void TabWorkouts::onWorkoutClicked()
{
    int id = selectedId();
    if (id != -1) {
        loadDetails(id);
        emit workoutSelected(id);
    }
}

void TabWorkouts::onAdd()
{
    QDialog d(this);
    d.setWindowTitle("Добавить тренировку");
    d.setFixedSize(350, 250);
    QFormLayout form;

    QComboBox *cbo = new QComboBox();
    QSqlQuery q(Database::getDb());

    if (m_isTrainer) {
        q.exec("SELECT id_client, name_client || ' ' || surname_client FROM client "
               "WHERE email_client NOT LIKE 'trener%' ORDER BY surname_client");
    } else {
        q.prepare("SELECT id_client, name_client || ' ' || surname_client FROM client WHERE id_client = :id");
        q.bindValue(":id", m_userId);
        q.exec();
    }
    while (q.next()) cbo->addItem(q.value(1).toString(), q.value(0).toInt());

    QLineEdit *title = new QLineEdit();
    QSpinBox *rpe = new QSpinBox(); rpe->setRange(1, 10); rpe->setValue(7);
    form.addRow("Клиент:", cbo);
    form.addRow("Название:", title);
    form.addRow("RPE:", rpe);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO workout_sessions (id_client, title_sessions, RPE) VALUES (:c, :t, :r)");
        ins.bindValue(":c", cbo->currentData().toInt());
        ins.bindValue(":t", title->text().trimmed());
        ins.bindValue(":r", rpe->value());
        ins.exec();
        loadWorkouts();
    }
}

void TabWorkouts::onEdit()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите тренировку"); return; }
    QSqlQuery q(Database::getDb());
    q.prepare("SELECT title_sessions, RPE FROM workout_sessions WHERE id_sessions = :id");
    q.bindValue(":id", id); q.exec();
    if (!q.next()) return;
    QDialog d(this);
    d.setWindowTitle("Редактировать"); d.setFixedSize(300, 150);
    QFormLayout form;
    QLineEdit *title = new QLineEdit(q.value(0).toString());
    QSpinBox *rpe = new QSpinBox(); rpe->setRange(1, 10); rpe->setValue(q.value(1).toInt());
    form.addRow("Название:", title); form.addRow("RPE:", rpe);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery upd(Database::getDb());
        upd.prepare("UPDATE workout_sessions SET title_sessions=:t, RPE=:r WHERE id_sessions=:id");
        upd.bindValue(":t", title->text().trimmed()); upd.bindValue(":r", rpe->value()); upd.bindValue(":id", id);
        upd.exec(); loadWorkouts();
    }
}

void TabWorkouts::onDelete()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите тренировку"); return; }
    if (QMessageBox::question(this, "Подтверждение", "Удалить тренировку и все её данные?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(Database::getDb());
        q.prepare("DELETE FROM workout_sessions WHERE id_sessions = :id");
        q.bindValue(":id", id); q.exec();
        loadWorkouts();
    }
}
