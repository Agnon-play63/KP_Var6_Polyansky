#include "tabworkoutexercises.h"
#include "database.h"

TabWorkoutExercises::TabWorkoutExercises(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent), m_isTrainer(isTrainer), m_userId(userId)
{
    // ====== ВЕРХ: выполненные упражнения ======
    QLabel *lblSes = new QLabel("Тренировка:");
    comboSession = new QComboBox();
    QLabel *lblEx = new QLabel("Упражнение:");
    comboExercise = new QComboBox();

    btnAddExercise  = new QPushButton("➕ Добавить");
    btnAddExercise->setStyleSheet("font-weight: bold;");
    btnEditExercise = new QPushButton("✏ Изменить");
    btnDeleteExercise = new QPushButton("🗑 Удалить");

    tableViewExercises = new QTableView();
    tableViewExercises->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewExercises->setSelectionMode(QAbstractItemView::SingleSelection);
    tableViewExercises->setAlternatingRowColors(true);

    modelExercises = new QSqlQueryModel(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(lblSes); topLayout->addWidget(comboSession);
    topLayout->addWidget(lblEx);  topLayout->addWidget(comboExercise);
    topLayout->addStretch();      topLayout->addWidget(btnAddExercise);

    QHBoxLayout *topButtons = new QHBoxLayout();
    topButtons->addStretch();
    topButtons->addWidget(btnEditExercise);
    topButtons->addWidget(btnDeleteExercise);

    // ====== НИЗ: детали (подходы) ======
    groupBoxDetails = new QGroupBox("Детали выбранного упражнения");

    QLabel *lblFilterSet = new QLabel("Фильтр:");
    comboFilterSet = new QComboBox();

    btnAddSet    = new QPushButton("➕ Добавить подход");
    btnAddSet->setStyleSheet("font-weight: bold;");
    btnEditSet   = new QPushButton("✏ Изменить подход");
    btnDeleteSet = new QPushButton("🗑 Удалить подход");

    tableViewSets = new QTableView();
    tableViewSets->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewSets->setSelectionMode(QAbstractItemView::SingleSelection);
    tableViewSets->setAlternatingRowColors(true);

    modelSets = new QSqlQueryModel(this);

    QHBoxLayout *detailsFilterLayout = new QHBoxLayout();
    detailsFilterLayout->addWidget(lblFilterSet);
    detailsFilterLayout->addWidget(comboFilterSet);
    detailsFilterLayout->addStretch();
    detailsFilterLayout->addWidget(btnAddSet);

    QHBoxLayout *detailsButtons = new QHBoxLayout();
    detailsButtons->addStretch();
    detailsButtons->addWidget(btnEditSet);
    detailsButtons->addWidget(btnDeleteSet);

    QVBoxLayout *detailsLayout = new QVBoxLayout();
    detailsLayout->addLayout(detailsFilterLayout);
    detailsLayout->addWidget(tableViewSets);
    detailsLayout->addLayout(detailsButtons);
    groupBoxDetails->setLayout(detailsLayout);

    // ====== ОБЩАЯ КОМПОНОВКА ======
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableViewExercises);
    mainLayout->addLayout(topButtons);
    mainLayout->addWidget(groupBoxDetails);

    // ====== СИГНАЛЫ ======
    connect(btnAddExercise,    &QPushButton::clicked, this, &TabWorkoutExercises::onAddExercise);
    connect(btnEditExercise,   &QPushButton::clicked, this, &TabWorkoutExercises::onEditExercise);
    connect(btnDeleteExercise, &QPushButton::clicked, this, &TabWorkoutExercises::onDeleteExercise);
    connect(btnAddSet,         &QPushButton::clicked, this, &TabWorkoutExercises::onAddSet);
    connect(btnEditSet,        &QPushButton::clicked, this, &TabWorkoutExercises::onEditSet);
    connect(btnDeleteSet,      &QPushButton::clicked, this, &TabWorkoutExercises::onDeleteSet);
    connect(comboSession,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TabWorkoutExercises::onFilterChanged);
    connect(comboExercise, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TabWorkoutExercises::onFilterChanged);
    connect(comboFilterSet, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        int weId = selectedExerciseId();
        if (weId != -1) loadSets(weId);
    });
    connect(tableViewExercises, &QTableView::clicked, this, &TabWorkoutExercises::onExerciseClicked);

    loadFilters();
    loadExercises();
    loadSetsFilter();
}

TabWorkoutExercises::~TabWorkoutExercises() {}

// ======================== ФИЛЬТРЫ ========================
void TabWorkoutExercises::setClientFilter(int clientId)
{
    m_selectedClientId = clientId;
    loadExercises();
}

void TabWorkoutExercises::setDateFilter(QDate from, QDate to)
{
    m_dateFrom = from;
    m_dateTo   = to;
    loadExercises();
}

void TabWorkoutExercises::setWorkoutFilter(int sessionId)
{
    m_selectedSessionId = sessionId;
    loadExercises();
}

// ======================== ВЫПОЛНЕННЫЕ УПРАЖНЕНИЯ ========================
void TabWorkoutExercises::loadFilters()
{
    comboSession->blockSignals(true);
    comboSession->clear();
    comboSession->addItem("Все", -1);
    QSqlQuery q(Database::getDb());
    if (m_isTrainer) {
        q.exec("SELECT id_sessions, title_sessions FROM workout_sessions ORDER BY start_time_sessions DESC");
    } else {
        q.prepare("SELECT ws.id_sessions, ws.title_sessions FROM workout_sessions ws "
                  "WHERE ws.id_client = :id ORDER BY ws.start_time_sessions DESC");
        q.bindValue(":id", m_userId);
        q.exec();
    }
    while (q.next()) comboSession->addItem(q.value(1).toString(), q.value(0).toInt());
    comboSession->blockSignals(false);

    comboExercise->blockSignals(true);
    comboExercise->clear();
    comboExercise->addItem("Все", -1);
    q.exec("SELECT id_exercises, name_exercises FROM exercises ORDER BY name_exercises");
    while (q.next()) comboExercise->addItem(q.value(1).toString(), q.value(0).toInt());
    comboExercise->blockSignals(false);
}

int TabWorkoutExercises::selectedExerciseId()
{
    QModelIndex idx = tableViewExercises->currentIndex();
    if (!idx.isValid()) return -1;
    return modelExercises->data(modelExercises->index(idx.row(), 0)).toInt();
}

void TabWorkoutExercises::loadExercises()
{
    QString filter = "WHERE 1=1";

    if (!m_isTrainer) {
        filter += QString(" AND ws.id_client = %1").arg(m_userId);
    } else if (m_selectedClientId != -1) {
        filter += QString(" AND ws.id_client = %1").arg(m_selectedClientId);
    }

    if (m_selectedSessionId != -1) {
        filter += QString(" AND we.id_sessions = %1").arg(m_selectedSessionId);
    }

    if (m_dateFrom.isValid()) {
        filter += QString(" AND ws.start_time_sessions::date >= '%1'").arg(m_dateFrom.toString("yyyy-MM-dd"));
    }
    if (m_dateTo.isValid()) {
        filter += QString(" AND ws.start_time_sessions::date <= '%1'").arg(m_dateTo.toString("yyyy-MM-dd"));
    }

    if (comboSession->currentData().toInt() != -1)
        filter += QString(" AND we.id_sessions = %1").arg(comboSession->currentData().toInt());
    if (comboExercise->currentData().toInt() != -1)
        filter += QString(" AND we.id_exercises = %1").arg(comboExercise->currentData().toInt());

    QString sql = QString(
                      "SELECT we.id_workout_exercises, ws.title_sessions, e.name_exercises, we.order_num_exercises, we.notes "
                      "FROM workouts_exercises we "
                      "JOIN workout_sessions ws ON we.id_sessions = ws.id_sessions "
                      "JOIN exercises e ON we.id_exercises = e.id_exercises "
                      "%1 ORDER BY ws.start_time_sessions DESC").arg(filter);

    modelExercises->setQuery(sql, Database::getDb());
    modelExercises->setHeaderData(0, Qt::Horizontal, "ID");
    modelExercises->setHeaderData(1, Qt::Horizontal, "Тренировка");
    modelExercises->setHeaderData(2, Qt::Horizontal, "Упражнение");
    modelExercises->setHeaderData(3, Qt::Horizontal, "Порядок");
    modelExercises->setHeaderData(4, Qt::Horizontal, "Заметки");
    tableViewExercises->setModel(modelExercises);
    tableViewExercises->hideColumn(0);
}

void TabWorkoutExercises::onExerciseClicked()
{
    int weId = selectedExerciseId();
    if (weId != -1) {
        loadSets(weId);
    }
}

void TabWorkoutExercises::onFilterChanged() { loadExercises(); }

void TabWorkoutExercises::onAddExercise()
{
    QDialog d(this);
    d.setWindowTitle("Добавить выполненное упражнение");
    d.setFixedSize(350, 200);
    QFormLayout form;
    QComboBox *cboSes = new QComboBox();
    QComboBox *cboEx  = new QComboBox();
    QSpinBox *order   = new QSpinBox(); order->setRange(1, 20); order->setValue(1);
    QLineEdit *notes  = new QLineEdit();

    QSqlQuery q(Database::getDb());
    if (m_isTrainer) {
        q.exec("SELECT id_sessions, title_sessions FROM workout_sessions ORDER BY start_time_sessions DESC");
    } else {
        q.prepare("SELECT id_sessions, title_sessions FROM workout_sessions WHERE id_client = :id ORDER BY start_time_sessions DESC");
        q.bindValue(":id", m_userId);
        q.exec();
    }
    while (q.next()) cboSes->addItem(q.value(1).toString(), q.value(0).toInt());

    q.exec("SELECT id_exercises, name_exercises FROM exercises ORDER BY name_exercises");
    while (q.next()) cboEx->addItem(q.value(1).toString(), q.value(0).toInt());

    form.addRow("Тренировка:", cboSes); form.addRow("Упражнение:", cboEx);
    form.addRow("Порядок:", order); form.addRow("Заметки:", notes);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect (&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO workouts_exercises (id_sessions, id_exercises, order_num_exercises, notes) "
                    "VALUES (:s, :e, :o, :n)");
        ins.bindValue(":s", cboSes->currentData().toInt());
        ins.bindValue(":e", cboEx->currentData().toInt());
        ins.bindValue(":o", order->value());
        ins.bindValue(":n", notes->text().trimmed());
        ins.exec();
        loadExercises();
    }
}

void TabWorkoutExercises::onEditExercise()
{
    if (selectedExerciseId() == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите запись"); return; }
    QMessageBox::information(this, "Редактирование", "Функция в разработке");
}

void TabWorkoutExercises::onDeleteExercise()
{
    int id = selectedExerciseId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите запись"); return; }
    if (QMessageBox::question(this, "Подтверждение", "Удалить запись и все подходы?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(Database::getDb());
        q.prepare("DELETE FROM workouts_exercises WHERE id_workout_exercises = :id");
        q.bindValue(":id", id); q.exec();
        loadExercises();
        modelSets->clear();
    }
}

// ======================== ПОДХОДЫ ========================
void TabWorkoutExercises::loadSetsFilter()
{
    comboFilterSet->blockSignals(true);
    comboFilterSet->clear();
    comboFilterSet->addItem("Все", -1);
    QSqlQuery q(Database::getDb());
    q.exec("SELECT we.id_workout_exercises, e.name_exercises || ' (сессия ' || we.id_sessions || ')' "
           "FROM workouts_exercises we JOIN exercises e ON we.id_exercises = e.id_exercises");
    while (q.next()) comboFilterSet->addItem(q.value(1).toString(), q.value(0).toInt());
    comboFilterSet->blockSignals(false);
}

int TabWorkoutExercises::selectedSetId()
{
    QModelIndex idx = tableViewSets->currentIndex();
    if (!idx.isValid()) return -1;
    return modelSets->data(modelSets->index(idx.row(), 0)).toInt();
}

void TabWorkoutExercises::loadSets(int workoutExerciseId)
{
    QString filter = QString("WHERE es.id_workout_exercises = %1").arg(workoutExerciseId);

    // Поверх — фильтр из комбо
    if (comboFilterSet->currentData().toInt() != -1)
        filter += QString(" AND es.id_workout_exercises = %1").arg(comboFilterSet->currentData().toInt());

    QString sql = QString(
                      "SELECT es.id_sets, e.name_exercises, es.number_set, es.reps, es.weight_kg, "
                      "CASE WHEN es.is_pr THEN '★' ELSE '' END "
                      "FROM exercises_sets es "
                      "JOIN workouts_exercises we ON es.id_workout_exercises = we.id_workout_exercises "
                      "JOIN exercises e ON we.id_exercises = e.id_exercises "
                      "%1 ORDER BY es.number_set").arg(filter);

    modelSets->setQuery(sql, Database::getDb());
    modelSets->setHeaderData(0, Qt::Horizontal, "ID");
    modelSets->setHeaderData(1, Qt::Horizontal, "Упражнение");
    modelSets->setHeaderData(2, Qt::Horizontal, "Подход");
    modelSets->setHeaderData(3, Qt::Horizontal, "Повторений");
    modelSets->setHeaderData(4, Qt::Horizontal, "Вес");
    modelSets->setHeaderData(5, Qt::Horizontal, "PR");
    tableViewSets->setModel(modelSets);
    tableViewSets->hideColumn(0);
}
void TabWorkoutExercises::onAddSet()
{
    int weId = selectedExerciseId();
    if (weId == -1) { QMessageBox::warning(this, "Предупреждение", "Сначала выберите упражнение в верхней таблице"); return; }

    QDialog d(this);
    d.setWindowTitle("Добавить подход"); d.setFixedSize(300, 200);
    QFormLayout form;
    QSpinBox *num = new QSpinBox(); num->setRange(1, 20);
    QSpinBox *reps = new QSpinBox(); reps->setRange(1, 100);
    QDoubleSpinBox *weight = new QDoubleSpinBox(); weight->setRange(0, 9999); weight->setDecimals(1);
    form.addRow("Подход №:", num); form.addRow("Повторений:", reps); form.addRow("Вес:", weight);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO exercises_sets (id_workout_exercises, number_set, reps, weight_kg) "
                    "VALUES (:we, :n, :r, :w)");
        ins.bindValue(":we", weId);
        ins.bindValue(":n",  num->value());
        ins.bindValue(":r",  reps->value());
        ins.bindValue(":w",  weight->value());
        ins.exec();
        loadSets(weId);
    }
}

void TabWorkoutExercises::onEditSet()
{
    int setId = selectedSetId();
    if (setId == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите подход в нижней таблице"); return; }

    QSqlQuery q(Database::getDb());
    q.prepare("SELECT number_set, reps, weight_kg, id_workout_exercises FROM exercises_sets WHERE id_sets = :id");
    q.bindValue(":id", setId); q.exec();
    if (!q.next()) return;
    int weId = q.value(3).toInt();

    QDialog d(this); d.setWindowTitle("Редактировать подход"); d.setFixedSize(300, 200);
    QFormLayout form;
    QSpinBox *num = new QSpinBox(); num->setRange(1, 20); num->setValue(q.value(0).toInt());
    QSpinBox *reps = new QSpinBox(); reps->setRange(1, 100); reps->setValue(q.value(1).toInt());
    QDoubleSpinBox *weight = new QDoubleSpinBox(); weight->setRange(0, 9999); weight->setDecimals(1);
    weight->setValue(q.value(2).toDouble());
    form.addRow("Подход №:", num); form.addRow("Повторений:", reps); form.addRow("Вес:", weight);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery upd(Database::getDb());
        upd.prepare("UPDATE exercises_sets SET number_set=:n, reps=:r, weight_kg=:w WHERE id_sets=:id");
        upd.bindValue(":n", num->value()); upd.bindValue(":r", reps->value());
        upd.bindValue(":w", weight->value()); upd.bindValue(":id", setId);
        upd.exec();
        loadSets(weId);
    }
}

void TabWorkoutExercises::onDeleteSet()
{
    int setId = selectedSetId();
    if (setId == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите подход в нижней таблице"); return; }

    // Узнать weId для обновления
    QSqlQuery q(Database::getDb());
    q.prepare("SELECT id_workout_exercises FROM exercises_sets WHERE id_sets = :id");
    q.bindValue(":id", setId); q.exec();
    if (!q.next()) return;
    int weId = q.value(0).toInt();

    if (QMessageBox::question(this, "Подтверждение", "Удалить подход?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery del(Database::getDb());
        del.prepare("DELETE FROM exercises_sets WHERE id_sets = :id");
        del.bindValue(":id", setId); del.exec();
        loadSets(weId);
    }
}
