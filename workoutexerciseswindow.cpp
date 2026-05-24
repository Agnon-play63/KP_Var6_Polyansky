#include "workoutexerciseswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QCheckBox>

WorkoutExercisesWindow::WorkoutExercisesWindow(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
    , m_isTrainer(isTrainer), m_userId(userId)
{
    setWindowTitle("Выполненные упражнения");
    resize(950, 680);
    setMinimumSize(750, 500);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    QLabel *title = new QLabel("Выполненные упражнения");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

    comboSession=new QComboBox(); comboExercise=new QComboBox();
    btnAddExercise=new QPushButton("➕ Добавить"); btnAddExercise->setStyleSheet("font-weight: bold;");
    btnEditExercise=new QPushButton("✏ Изменить"); btnDeleteExercise=new QPushButton("🗑 Удалить");
    tableViewExercises=new QTableView(); tableViewExercises->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewExercises->setSelectionMode(QAbstractItemView::SingleSelection); tableViewExercises->setAlternatingRowColors(true);
    modelExercises=new QSqlQueryModel(this);

    QHBoxLayout *top=new QHBoxLayout(); top->addWidget(new QLabel("Тренировка:")); top->addWidget(comboSession);
    top->addWidget(new QLabel("Упражнение:")); top->addWidget(comboExercise); top->addStretch(); top->addWidget(btnAddExercise);
    QHBoxLayout *topBtns=new QHBoxLayout(); topBtns->addStretch(); topBtns->addWidget(btnEditExercise); topBtns->addWidget(btnDeleteExercise);

    groupBoxDetails=new QGroupBox("Детали выбранного упражнения");
    comboFilterSet=new QComboBox(); btnAddSet=new QPushButton("➕ Добавить подход"); btnAddSet->setStyleSheet("font-weight: bold;");
    btnEditSet=new QPushButton("✏ Изменить подход"); btnDeleteSet=new QPushButton("🗑 Удалить подход");
    tableViewSets=new QTableView(); tableViewSets->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewSets->setSelectionMode(QAbstractItemView::SingleSelection); tableViewSets->setAlternatingRowColors(true);
    modelSets=new QSqlQueryModel(this);

    QHBoxLayout *dt=new QHBoxLayout(); dt->addWidget(new QLabel("Фильтр:")); dt->addWidget(comboFilterSet); dt->addStretch(); dt->addWidget(btnAddSet);
    QHBoxLayout *db=new QHBoxLayout(); db->addStretch(); db->addWidget(btnEditSet); db->addWidget(btnDeleteSet);
    QVBoxLayout *dl=new QVBoxLayout(); dl->addLayout(dt); dl->addWidget(tableViewSets); dl->addLayout(db); groupBoxDetails->setLayout(dl);

    QVBoxLayout *mainLayout=new QVBoxLayout(this);
    mainLayout->addWidget(title);
    mainLayout->addLayout(top); mainLayout->addWidget(tableViewExercises); mainLayout->addLayout(topBtns); mainLayout->addWidget(groupBoxDetails);

    connect(btnAddExercise,&QPushButton::clicked,this,&WorkoutExercisesWindow::onAddExercise);
    connect(btnEditExercise,&QPushButton::clicked,this,&WorkoutExercisesWindow::onEditExercise);
    connect(btnDeleteExercise,&QPushButton::clicked,this,&WorkoutExercisesWindow::onDeleteExercise);
    connect(btnAddSet,&QPushButton::clicked,this,&WorkoutExercisesWindow::onAddSet);
    connect(btnEditSet,&QPushButton::clicked,this,&WorkoutExercisesWindow::onEditSet);
    connect(btnDeleteSet,&QPushButton::clicked,this,&WorkoutExercisesWindow::onDeleteSet);
    connect(comboSession,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WorkoutExercisesWindow::onFilterChanged);
    connect(comboExercise,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WorkoutExercisesWindow::onFilterChanged);
    connect(comboFilterSet,QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int){int we=selectedExerciseId(); if(we!=-1)loadSets(we);});
    connect(tableViewExercises,&QTableView::clicked,this,&WorkoutExercisesWindow::onExerciseClicked);

    tableViewExercises->viewport()->installEventFilter(this);
    tableViewSets->viewport()->installEventFilter(this);

    loadFilters(); loadExercises(); loadSetsFilter();
}

WorkoutExercisesWindow::~WorkoutExercisesWindow() {}

void WorkoutExercisesWindow::setClientFilter(int c){m_selectedClientId=c;loadExercises();}
void WorkoutExercisesWindow::setDateFilter(QDate f,QDate t){m_dateFrom=f;m_dateTo=t;loadExercises();}
void WorkoutExercisesWindow::setWorkoutFilter(int s){m_selectedSessionId=s;loadExercises();}

void WorkoutExercisesWindow::loadFilters(){
    comboSession->blockSignals(true);comboSession->clear();comboSession->addItem("Все",-1);QSqlQuery q(Database::getDb());
    if(m_isTrainer)q.exec("SELECT id_sessions,title_sessions FROM workout_sessions ORDER BY start_time_sessions DESC");
    else{q.prepare("SELECT id_sessions,title_sessions FROM workout_sessions WHERE id_client=:id ORDER BY start_time_sessions DESC");q.bindValue(":id",m_userId);q.exec();}
    while(q.next())comboSession->addItem(q.value(1).toString(),q.value(0).toInt());comboSession->blockSignals(false);
    comboExercise->blockSignals(true);comboExercise->clear();comboExercise->addItem("Все",-1);
    q.exec("SELECT id_exercises,name_exercises FROM exercises ORDER BY name_exercises");
    while(q.next())comboExercise->addItem(q.value(1).toString(),q.value(0).toInt());comboExercise->blockSignals(false);
}

int WorkoutExercisesWindow::selectedExerciseId() {
    QModelIndex idx = tableViewExercises->currentIndex();
    if (!idx.isValid()) return -1;
    return modelExercises->data(modelExercises->index(idx.row(), 0)).toInt();
}

int WorkoutExercisesWindow::selectedSetId() {
    QModelIndex idx = tableViewSets->currentIndex();
    if (!idx.isValid()) return -1;
    return modelSets->data(modelSets->index(idx.row(), 0)).toInt();
}

void WorkoutExercisesWindow::loadExercises(){
    QString f="WHERE 1=1";
    if(!m_isTrainer)f+=QString(" AND ws.id_client=%1").arg(m_userId);else if(m_selectedClientId!=-1)f+=QString(" AND ws.id_client=%1").arg(m_selectedClientId);
    if(m_selectedSessionId!=-1)f+=QString(" AND we.id_sessions=%1").arg(m_selectedSessionId);
    if(m_dateFrom.isValid())f+=QString(" AND ws.start_time_sessions::date>='%1'").arg(m_dateFrom.toString("yyyy-MM-dd"));
    if(m_dateTo.isValid())f+=QString(" AND ws.start_time_sessions::date<='%1'").arg(m_dateTo.toString("yyyy-MM-dd"));
    if(comboSession->currentData().toInt()!=-1)f+=QString(" AND we.id_sessions=%1").arg(comboSession->currentData().toInt());
    if(comboExercise->currentData().toInt()!=-1)f+=QString(" AND we.id_exercises=%1").arg(comboExercise->currentData().toInt());
    QString sql = QString(
                      "SELECT we.id_workout_exercises, "
                      "ws.title_sessions AS \"Тренировка\", "
                      "e.name_exercises AS \"Упражнение\", "
                      "we.order_num_exercises AS \"Порядок\", "
                      "we.notes AS \"Заметки\" "
                      "FROM workouts_exercises we "
                      "JOIN workout_sessions ws ON we.id_sessions=ws.id_sessions "
                      "JOIN exercises e ON we.id_exercises=e.id_exercises "
                      "%1 ORDER BY ws.start_time_sessions DESC").arg(f);
    modelExercises->setQuery(sql, Database::getDb());
    tableViewExercises->setModel(modelExercises);
    tableViewExercises->hideColumn(0);
}

void WorkoutExercisesWindow::loadSetsFilter(){
    comboFilterSet->blockSignals(true);comboFilterSet->clear();comboFilterSet->addItem("Все",-1);QSqlQuery q(Database::getDb());
    q.exec("SELECT we.id_workout_exercises, e.name_exercises||' (сессия '||we.id_sessions||')' FROM workouts_exercises we JOIN exercises e ON we.id_exercises=e.id_exercises");
    while(q.next())comboFilterSet->addItem(q.value(1).toString(),q.value(0).toInt());comboFilterSet->blockSignals(false);
}

void WorkoutExercisesWindow::loadSets(int we){
    QString f=QString("WHERE es.id_workout_exercises=%1").arg(we);
    if(comboFilterSet->currentData().toInt()!=-1)f+=QString(" AND es.id_workout_exercises=%1").arg(comboFilterSet->currentData().toInt());
    QString sql = QString(
                      "SELECT es.id_sets, "
                      "e.name_exercises AS \"Упражнение\", "
                      "es.number_set AS \"Подход №\", "
                      "es.reps AS \"Повторений\", "
                      "es.weight_kg AS \"Вес (кг)\", "
                      "CASE WHEN es.is_pr THEN '★' ELSE '' END AS \"Рекорд\" "
                      "FROM exercises_sets es "
                      "JOIN workouts_exercises we ON es.id_workout_exercises=we.id_workout_exercises "
                      "JOIN exercises e ON we.id_exercises=e.id_exercises "
                      "%1 ORDER BY es.number_set").arg(f);
    modelSets->setQuery(sql, Database::getDb());
    tableViewSets->setModel(modelSets);
    tableViewSets->hideColumn(0);
}

void WorkoutExercisesWindow::onExerciseClicked(){int we=selectedExerciseId();if(we!=-1)loadSets(we);}
void WorkoutExercisesWindow::onFilterChanged(){loadExercises();}

void WorkoutExercisesWindow::onAddExercise(){
    QDialog d(this);d.setWindowTitle("Добавить");d.setFixedSize(350,200);QFormLayout f;QComboBox *cs=new QComboBox();QComboBox *ce=new QComboBox();
    QSpinBox *o=new QSpinBox();o->setRange(1,20);o->setValue(1);QLineEdit *n=new QLineEdit();QSqlQuery q(Database::getDb());
    if(m_isTrainer)q.exec("SELECT id_sessions,title_sessions FROM workout_sessions ORDER BY start_time_sessions DESC");
    else{q.prepare("SELECT id_sessions,title_sessions FROM workout_sessions WHERE id_client=:id ORDER BY start_time_sessions DESC");q.bindValue(":id",m_userId);q.exec();}
    while(q.next())cs->addItem(q.value(1).toString(),q.value(0).toInt());
    q.exec("SELECT id_exercises,name_exercises FROM exercises ORDER BY name_exercises");while(q.next())ce->addItem(q.value(1).toString(),q.value(0).toInt());
    f.addRow("Тренировка:",cs);f.addRow("Упражнение:",ce);f.addRow("Порядок:",o);f.addRow("Заметки:",n);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&btns);d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){QSqlQuery ins(Database::getDb());ins.prepare("INSERT INTO workouts_exercises(id_sessions,id_exercises,order_num_exercises,notes) VALUES(:s,:e,:o,:n)");ins.bindValue(":s",cs->currentData().toInt());ins.bindValue(":e",ce->currentData().toInt());ins.bindValue(":o",o->value());ins.bindValue(":n",n->text().trimmed());ins.exec();loadExercises();}
}

void WorkoutExercisesWindow::onEditExercise(){
    if(selectedExerciseId()==-1){QMessageBox::warning(this,"Предупреждение","Выберите запись");return;}
    QMessageBox::information(this,"Редактирование","Функция в разработке");
}

void WorkoutExercisesWindow::onDeleteExercise(){
    int id=selectedExerciseId();if(id==-1){QMessageBox::warning(this,"Предупреждение","Выберите запись");return;}
    if(QMessageBox::question(this,"Подтверждение","Удалить запись и все подходы?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    {QSqlQuery q(Database::getDb());q.prepare("DELETE FROM workouts_exercises WHERE id_workout_exercises=:id");q.bindValue(":id",id);q.exec();loadExercises();modelSets->clear();}
}

void WorkoutExercisesWindow::onAddSet(){
    int we=selectedExerciseId();if(we==-1){QMessageBox::warning(this,"Предупреждение","Выберите упражнение в верхней таблице");return;}
    QDialog d(this);d.setWindowTitle("Добавить подход");d.setFixedSize(300,250);QFormLayout f;
    QSpinBox *nm=new QSpinBox();nm->setRange(1,20);
    QSpinBox *rp=new QSpinBox();rp->setRange(1,100);
    QDoubleSpinBox *w=new QDoubleSpinBox();w->setRange(0,9999);w->setDecimals(1);
    QCheckBox *chkPR = new QCheckBox("Личный рекорд");
    f.addRow("Подход №:",nm);f.addRow("Повторений:",rp);f.addRow("Вес:",w);f.addRow("",chkPR);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&btns);d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){
        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO exercises_sets(id_workout_exercises,number_set,reps,weight_kg,is_pr) VALUES(:we,:n,:r,:w,:pr)");
        ins.bindValue(":we",we);ins.bindValue(":n",nm->value());ins.bindValue(":r",rp->value());
        ins.bindValue(":w",w->value());ins.bindValue(":pr",chkPR->isChecked());
        ins.exec();loadSets(we);
    }
}

void WorkoutExercisesWindow::onEditSet(){
    int sid=selectedSetId();if(sid==-1){QMessageBox::warning(this,"Предупреждение","Выберите подход");return;}
    QSqlQuery q(Database::getDb());q.prepare("SELECT number_set,reps,weight_kg,is_pr,id_workout_exercises FROM exercises_sets WHERE id_sets=:id");q.bindValue(":id",sid);q.exec();
    if(!q.next())return;int we=q.value(4).toInt();
    QDialog d(this);d.setWindowTitle("Редактировать подход");d.setFixedSize(300,250);QFormLayout f;
    QSpinBox *nm=new QSpinBox();nm->setRange(1,20);nm->setValue(q.value(0).toInt());
    QSpinBox *rp=new QSpinBox();rp->setRange(1,100);rp->setValue(q.value(1).toInt());
    QDoubleSpinBox *w=new QDoubleSpinBox();w->setRange(0,9999);w->setDecimals(1);w->setValue(q.value(2).toDouble());
    QCheckBox *chkPR = new QCheckBox("Личный рекорд");chkPR->setChecked(q.value(3).toBool());
    f.addRow("Подход №:",nm);f.addRow("Повторений:",rp);f.addRow("Вес:",w);f.addRow("",chkPR);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&btns);d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){
        QSqlQuery up(Database::getDb());
        up.prepare("UPDATE exercises_sets SET number_set=:n,reps=:r,weight_kg=:w,is_pr=:pr WHERE id_sets=:id");
        up.bindValue(":n",nm->value());up.bindValue(":r",rp->value());up.bindValue(":w",w->value());
        up.bindValue(":pr",chkPR->isChecked());up.bindValue(":id",sid);
        up.exec();loadSets(we);
    }
}

void WorkoutExercisesWindow::onDeleteSet(){
    int sid=selectedSetId();if(sid==-1){QMessageBox::warning(this,"Предупреждение","Выберите подход");return;}
    QSqlQuery q(Database::getDb());q.prepare("SELECT id_workout_exercises FROM exercises_sets WHERE id_sets=:id");q.bindValue(":id",sid);q.exec();if(!q.next())return;
    int we=q.value(0).toInt();
    if(QMessageBox::question(this,"Подтверждение","Удалить подход?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    {QSqlQuery del(Database::getDb());del.prepare("DELETE FROM exercises_sets WHERE id_sets=:id");del.bindValue(":id",sid);del.exec();loadSets(we);}
}

bool WorkoutExercisesWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (obj == tableViewExercises->viewport()) {
            QModelIndex idx = tableViewExercises->indexAt(mouseEvent->pos());
            if (!idx.isValid()) {
                tableViewExercises->clearSelection();
                tableViewExercises->setCurrentIndex(QModelIndex());
            }
        } else if (obj == tableViewSets->viewport()) {
            QModelIndex idx = tableViewSets->indexAt(mouseEvent->pos());
            if (!idx.isValid()) {
                tableViewSets->clearSelection();
                tableViewSets->setCurrentIndex(QModelIndex());
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
