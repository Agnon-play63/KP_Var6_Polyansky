#include "workoutswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>
#include <QItemSelectionModel>
#include <QMouseEvent>

WorkoutsWindow::WorkoutsWindow(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
    , m_isTrainer(isTrainer), m_userId(userId)
{
    setWindowTitle("Тренировки");
    resize(950, 550);
    setMinimumSize(750, 400);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    QLabel *title = new QLabel("Тренировки");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

    dateEditFrom = new QDateEdit(QDate::currentDate().addMonths(-1)); dateEditFrom->setCalendarPopup(true); dateEditFrom->setDisplayFormat("dd.MM.yyyy");
    dateEditTo   = new QDateEdit(QDate::currentDate()); dateEditTo->setCalendarPopup(true); dateEditTo->setDisplayFormat("dd.MM.yyyy");
    comboClientFilter = new QComboBox();
    btnAdd= new QPushButton("➕ Добавить"); btnAdd->setStyleSheet("font-weight: bold;");
    btnEdit= new QPushButton("✏ Изменить"); btnDelete= new QPushButton("🗑 Удалить");

    tableViewWorkouts = new QTableView(); tableViewWorkouts->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewWorkouts->setSelectionMode(QAbstractItemView::SingleSelection); tableViewWorkouts->setAlternatingRowColors(true);

    modelWorkouts = new QSqlQueryModel(this);

    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(new QLabel("Дата с:")); top->addWidget(dateEditFrom);
    top->addWidget(new QLabel("по:")); top->addWidget(dateEditTo);
    if (m_isTrainer) { top->addWidget(new QLabel("Клиент:")); top->addWidget(comboClientFilter); }
    top->addStretch(); top->addWidget(btnAdd);

    QHBoxLayout *bot = new QHBoxLayout(); bot->addStretch(); bot->addWidget(btnEdit); bot->addWidget(btnDelete);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(title);
    mainLayout->addLayout(top); mainLayout->addWidget(tableViewWorkouts); mainLayout->addLayout(bot);

    connect(btnAdd,&QPushButton::clicked,this,&WorkoutsWindow::onAdd);
    connect(btnEdit,&QPushButton::clicked,this,&WorkoutsWindow::onEdit);
    connect(btnDelete,&QPushButton::clicked,this,&WorkoutsWindow::onDelete);
    connect(comboClientFilter,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WorkoutsWindow::onFilterChanged);
    connect(dateEditFrom,&QDateEdit::dateChanged,this,&WorkoutsWindow::onFilterChanged);
    connect(dateEditTo,&QDateEdit::dateChanged,this,&WorkoutsWindow::onFilterChanged);

    connect(tableViewWorkouts, &QTableView::doubleClicked, [this]() {
        int id = selectedId();
        if (id != -1) emit workoutDoubleClicked(id);
    });

    tableViewWorkouts->viewport()->installEventFilter(this);

    loadClientFilter(); loadWorkouts();
}


WorkoutsWindow::~WorkoutsWindow() {}

void WorkoutsWindow::setClientFilter(int clientId, const QString &)
{
    for (int i = 0; i < comboClientFilter->count(); i++) {
        if (comboClientFilter->itemData(i).toInt() == clientId) {
            comboClientFilter->setCurrentIndex(i);
            break;
        }
    }
    loadWorkouts();
}

void WorkoutsWindow::loadClientFilter() {
    comboClientFilter->blockSignals(true); comboClientFilter->clear(); comboClientFilter->addItem("Все",-1);
    QSqlQuery q(Database::getDb()); q.exec("SELECT id_client, name_client||' '||surname_client FROM client ORDER BY surname_client");
    while(q.next()) comboClientFilter->addItem(q.value(1).toString(), q.value(0).toInt());
    comboClientFilter->blockSignals(false);
}

int WorkoutsWindow::selectedId() {
    QModelIndex idx = tableViewWorkouts->currentIndex();
    if (!idx.isValid()) return -1;
    return modelWorkouts->data(modelWorkouts->index(idx.row(), 0)).toInt();
}

void WorkoutsWindow::loadWorkouts() {
    QString filter = "WHERE 1=1";
    if (!m_isTrainer) filter += QString(" AND ws.id_client=%1").arg(m_userId);
    else if (comboClientFilter->currentData().toInt()!=-1) filter += QString(" AND ws.id_client=%1").arg(comboClientFilter->currentData().toInt());
    filter += QString(" AND ws.start_time_sessions::date>='%1' AND ws.start_time_sessions::date<='%2'")
                  .arg(dateEditFrom->date().toString("yyyy-MM-dd")).arg(dateEditTo->date().toString("yyyy-MM-dd"));
    QString sql = QString(
                      "SELECT ws.id_sessions, "
                      "c.name_client||' '||c.surname_client AS \"Клиент\", "
                      "ws.title_sessions AS \"Название тренировки\", "
                      "ws.start_time_sessions::date AS \"Дата\", "
                      "ws.RPE AS \"RPE\", "
                      "(SELECT COUNT(*) FROM workouts_exercises WHERE id_sessions=ws.id_sessions) AS \"Упражнений\" "
                      "FROM workout_sessions ws JOIN client c ON ws.id_client=c.id_client "
                      "%1 ORDER BY ws.start_time_sessions DESC").arg(filter);
    modelWorkouts->setQuery(sql, Database::getDb());
    tableViewWorkouts->setModel(modelWorkouts);
    tableViewWorkouts->hideColumn(0);
    if (m_isTrainer) emit clientSelected(comboClientFilter->currentData().toInt());
    emit dateFilterChanged(dateEditFrom->date(), dateEditTo->date());
}

void WorkoutsWindow::onFilterChanged() { loadWorkouts(); }

void WorkoutsWindow::onAdd() {
    QDialog d(this); d.setWindowTitle("Добавить тренировку"); d.setFixedSize(350,250); QFormLayout f; QComboBox *cbo=new QComboBox();
    QSqlQuery q(Database::getDb());
    if(m_isTrainer) q.exec("SELECT id_client, name_client||' '||surname_client FROM client ORDER BY surname_client");
    else { q.prepare("SELECT id_client, name_client||' '||surname_client FROM client WHERE id_client=:id"); q.bindValue(":id",m_userId); q.exec(); }
    while(q.next()) cbo->addItem(q.value(1).toString(),q.value(0).toInt());
    QLineEdit *t=new QLineEdit(); QSpinBox *r=new QSpinBox(); r->setRange(1,10); r->setValue(7);
    f.addRow("Клиент:",cbo); f.addRow("Название:",t); f.addRow("RPE:",r);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); f.addRow(&btns); d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept); connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){QSqlQuery ins(Database::getDb());ins.prepare("INSERT INTO workout_sessions(id_client,title_sessions,RPE) VALUES(:c,:t,:r)");
        ins.bindValue(":c",cbo->currentData().toInt());ins.bindValue(":t",t->text().trimmed());ins.bindValue(":r",r->value());ins.exec();loadWorkouts();}
}

void WorkoutsWindow::onEdit() {
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите тренировку"); return; }
    QSqlQuery q(Database::getDb()); q.prepare("SELECT title_sessions,RPE FROM workout_sessions WHERE id_sessions=:id"); q.bindValue(":id",id); q.exec();
    if(!q.next()) return;
    QDialog d(this); d.setWindowTitle("Редактировать"); d.setFixedSize(300,150); QFormLayout f;
    QLineEdit *t=new QLineEdit(q.value(0).toString()); QSpinBox *r=new QSpinBox(); r->setRange(1,10); r->setValue(q.value(1).toInt());
    f.addRow("Название:",t); f.addRow("RPE:",r);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); f.addRow(&btns); d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept); connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){QSqlQuery up(Database::getDb());up.prepare("UPDATE workout_sessions SET title_sessions=:t,RPE=:r WHERE id_sessions=:id");
        up.bindValue(":t",t->text().trimmed());up.bindValue(":r",r->value());up.bindValue(":id",id);up.exec();loadWorkouts();}
}

void WorkoutsWindow::onDelete() {
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите тренировку"); return; }
    if(QMessageBox::question(this,"Подтверждение","Удалить тренировку?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    {QSqlQuery q(Database::getDb());q.prepare("DELETE FROM workout_sessions WHERE id_sessions=:id");q.bindValue(":id",id);q.exec();loadWorkouts();}
}

bool WorkoutsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tableViewWorkouts->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex idx = tableViewWorkouts->indexAt(mouseEvent->pos());
        if (!idx.isValid()) {
            tableViewWorkouts->clearSelection();
            tableViewWorkouts->setCurrentIndex(QModelIndex());
        }
    }
    return QWidget::eventFilter(obj, event);
}
