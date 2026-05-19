#include "tabsets.h"
#include "database.h"

TabSets::TabSets(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent), m_isTrainer(isTrainer), m_userId(userId)
{
    QLabel *lbl = new QLabel("Выполненное упражнение:");
    comboFilter = new QComboBox();

    btnAdd  = new QPushButton("➕ Добавить"); btnAdd->setStyleSheet("font-weight: bold;");
    btnEdit = new QPushButton("✏ Изменить");
    btnDelete = new QPushButton("🗑 Удалить");

    tableView = new QTableView();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);

    model = new QSqlQueryModel(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(lbl); topLayout->addWidget(comboFilter);
    topLayout->addStretch();   topLayout->addWidget(btnAdd);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnEdit);
    bottomLayout->addWidget(btnDelete);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(bottomLayout);

    connect(btnAdd,  &QPushButton::clicked, this, &TabSets::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &TabSets::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &TabSets::onDelete);
    connect(comboFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TabSets::onFilterChanged);

    loadFilter();
    loadData();
}

TabSets::~TabSets() {}

void TabSets::loadFilter()
{
    comboFilter->blockSignals(true);
    comboFilter->clear();
    comboFilter->addItem("Все", -1);
    QSqlQuery q(Database::getDb());

    if (m_isTrainer) {
        q.exec("SELECT we.id_workout_exercises, e.name_exercises || ' (сессия ' || we.id_sessions || ')' "
               "FROM workouts_exercises we JOIN exercises e ON we.id_exercises = e.id_exercises");
    } else {
        q.prepare("SELECT we.id_workout_exercises, e.name_exercises || ' (сессия ' || we.id_sessions || ')' "
                  "FROM workouts_exercises we JOIN exercises e ON we.id_exercises = e.id_exercises "
                  "JOIN workout_sessions ws ON we.id_sessions = ws.id_sessions "
                  "WHERE ws.id_client = :id");
        q.bindValue(":id", m_userId);
        q.exec();
    }
    while (q.next()) comboFilter->addItem(q.value(1).toString(), q.value(0).toInt());
    comboFilter->blockSignals(false);
}

int TabSets::selectedId()
{
    QModelIndex idx = tableView->currentIndex();
    if (!idx.isValid()) return -1;
    return model->data(model->index(idx.row(), 0)).toInt();
}

void TabSets::loadData()
{
    QString filter = "WHERE 1=1";

    if (!m_isTrainer) {
        filter += QString(" AND ws.id_client = %1").arg(m_userId);
    } else if (m_selectedClientId != -1) {
        filter += QString(" AND ws.id_client = %1").arg(m_selectedClientId);
    }

    // Фильтр по конкретной тренировке
    if (m_selectedSessionId != -1) {
        filter += QString(" AND we.id_sessions = %1").arg(m_selectedSessionId);
    }

    // Фильтр по датам
    if (m_dateFrom.isValid()) {
        filter += QString(" AND ws.start_time_sessions::date >= '%1'").arg(m_dateFrom.toString("yyyy-MM-dd"));
    }
    if (m_dateTo.isValid()) {
        filter += QString(" AND ws.start_time_sessions::date <= '%1'").arg(m_dateTo.toString("yyyy-MM-dd"));
    }

    // Фильтр из выпадающего списка (работает поверх)
    if (comboFilter->currentData().toInt() != -1)
        filter += QString(" AND es.id_workout_exercises = %1").arg(comboFilter->currentData().toInt());

    QString sql = QString(
                      "SELECT es.id_sets, e.name_exercises, es.number_set, es.reps, es.weight_kg, "
                      "CASE WHEN es.is_pr THEN '★' ELSE '' END "
                      "FROM exercises_sets es "
                      "JOIN workouts_exercises we ON es.id_workout_exercises = we.id_workout_exercises "
                      "JOIN exercises e ON we.id_exercises = e.id_exercises "
                      "JOIN workout_sessions ws ON we.id_sessions = ws.id_sessions "
                      "%1 ORDER BY es.id_sets").arg(filter);

    model->setQuery(sql, Database::getDb());
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Упражнение");
    model->setHeaderData(2, Qt::Horizontal, "Подход");
    model->setHeaderData(3, Qt::Horizontal, "Повторений");
    model->setHeaderData(4, Qt::Horizontal, "Вес");
    model->setHeaderData(5, Qt::Horizontal, "PR");
    tableView->setModel(model);
    tableView->hideColumn(0);
}

void TabSets::onFilterChanged() { loadData(); }

void TabSets::onAdd()
{
    QDialog d(this);
    d.setWindowTitle("Добавить подход"); d.setFixedSize(350, 250);
    QFormLayout form;
    QComboBox *cbo = new QComboBox();
    QSqlQuery q(Database::getDb());

    if (m_isTrainer) {
        q.exec("SELECT we.id_workout_exercises, e.name_exercises || ' (сессия ' || we.id_sessions || ')' "
               "FROM workouts_exercises we JOIN exercises e ON we.id_exercises = e.id_exercises");
    } else {
        q.prepare("SELECT we.id_workout_exercises, e.name_exercises || ' (сессия ' || we.id_sessions || ')' "
                  "FROM workouts_exercises we JOIN exercises e ON we.id_exercises = e.id_exercises "
                  "JOIN workout_sessions ws ON we.id_sessions = ws.id_sessions "
                  "WHERE ws.id_client = :id");
        q.bindValue(":id", m_userId);
        q.exec();
    }
    while (q.next()) cbo->addItem(q.value(1).toString(), q.value(0).toInt());

    QSpinBox *num = new QSpinBox(); num->setRange(1, 20);
    QSpinBox *reps = new QSpinBox(); reps->setRange(1, 100);
    QDoubleSpinBox *weight = new QDoubleSpinBox(); weight->setRange(0, 9999); weight->setDecimals(1);
    form.addRow("Упражнение:", cbo); form.addRow("Подход №:", num);
    form.addRow("Повторений:", reps); form.addRow("Вес:", weight);
    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns); d.setLayout(&form);
    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO exercises_sets (id_workout_exercises, number_set, reps, weight_kg) "
                    "VALUES (:we, :n, :r, :w)");
        ins.bindValue(":we", cbo->currentData().toInt()); ins.bindValue(":n", num->value());
        ins.bindValue(":r", reps->value()); ins.bindValue(":w", weight->value());
        ins.exec(); loadData();
    }
}

void TabSets::onEdit()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите подход"); return; }
    QSqlQuery q(Database::getDb());
    q.prepare("SELECT number_set, reps, weight_kg FROM exercises_sets WHERE id_sets = :id");
    q.bindValue(":id", id); q.exec();
    if (!q.next()) return;
    QDialog d(this); d.setWindowTitle("Редактировать"); d.setFixedSize(300, 200);
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
        upd.bindValue(":w", weight->value()); upd.bindValue(":id", id);
        upd.exec(); loadData();
    }
}

void TabSets::onDelete()
{
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите подход"); return; }
    if (QMessageBox::question(this, "Подтверждение", "Удалить подход?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(Database::getDb());
        q.prepare("DELETE FROM exercises_sets WHERE id_sets = :id");
        q.bindValue(":id", id); q.exec();
        loadData();
    }
}

void TabSets::setClientFilter(int clientId)
{
    m_selectedClientId = clientId;
    loadData();
}

void TabSets::setDateFilter(QDate from, QDate to)
{
    m_dateFrom = from;
    m_dateTo   = to;
    loadData();
}

void TabSets::setWorkoutFilter(int sessionId)
{
    m_selectedSessionId = sessionId;
    loadData();
}
