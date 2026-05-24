#include "recordswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>

RecordsWindow::RecordsWindow(QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
{
    setWindowTitle("Таблица рекордсменов");
    resize(850, 520);
    setMinimumSize(650, 400);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    QLabel *title = new QLabel("Рекорды");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

    tableViewRecords = new QTableView();
    tableViewRecords->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewRecords->setAlternatingRowColors(true);
    tableViewRecords->setSortingEnabled(true);

    modelRecords = new QSqlQueryModel(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(tableViewRecords);

    loadRecords();
}

RecordsWindow::~RecordsWindow() {}

void RecordsWindow::loadRecords()
{
    QString sql =
        "SELECT c.name_client||' '||c.surname_client AS \"Спортсмен\", "
        "e.name_exercises AS \"Упражнение\", MAX(es.weight_kg) AS \"Макс.вес\", "
        "MAX(es.reps) AS \"Макс.повт\", ws.start_time_sessions::date AS \"Дата\" "
        "FROM exercises_sets es JOIN workouts_exercises we ON es.id_workout_exercises=we.id_workout_exercises "
        "JOIN workout_sessions ws ON we.id_sessions=ws.id_sessions "
        "JOIN exercises e ON we.id_exercises=e.id_exercises "
        "JOIN client c ON ws.id_client=c.id_client "
        "WHERE es.is_pr=TRUE GROUP BY c.id_client,e.id_exercises,e.name_exercises,c.name_client,c.surname_client,ws.start_time_sessions::date "
        "ORDER BY e.name_exercises, \"Макс.вес\" DESC";
    modelRecords->setQuery(sql, Database::getDb());
    tableViewRecords->setModel(modelRecords);
    tableViewRecords->resizeColumnsToContents();
}
