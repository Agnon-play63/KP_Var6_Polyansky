#include "exerciseswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>
#include <QItemSelectionModel>
#include <QMouseEvent>

ExercisesWindow::ExercisesWindow(bool isTrainer, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
    , m_isTrainer(isTrainer)
{
    setWindowTitle("Упражнения");
    resize(850, 580);
    setMinimumSize(650, 450);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    QLabel *title = new QLabel("Упражнения");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

    comboCategory=new QComboBox();comboCategory->addItems({"Все","Силовые","Кардио","Растяжка","Кроссфит"});
    editSearch=new QLineEdit();editSearch->setPlaceholderText("Введите название...");
    btnAdd=new QPushButton("➕ Добавить");btnAdd->setStyleSheet("font-weight: bold;");
    btnEdit=new QPushButton("✏ Изменить");
    if(!m_isTrainer){btnAdd->setVisible(false);btnEdit->setVisible(false);}
    tableView=new QTableView();tableView->setSelectionBehavior(QAbstractItemView::SelectRows);tableView->setAlternatingRowColors(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    model=new QSqlTableModel(this,Database::getDb());model->setTable("exercises");model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setHeaderData(1, Qt::Horizontal, "Название");
    model->setHeaderData(3, Qt::Horizontal, "Ед. изм.");
    model->setHeaderData(4, Qt::Horizontal, "Категория");
    tableView->setModel(model);tableView->hideColumn(0);tableView->hideColumn(2);tableView->hideColumn(5);

    QHBoxLayout *top=new QHBoxLayout();top->addWidget(new QLabel("Категория:"));top->addWidget(comboCategory);
    top->addWidget(new QLabel("Поиск:"));top->addWidget(editSearch);top->addStretch();top->addWidget(btnEdit);top->addWidget(btnAdd);
    QVBoxLayout *mainLayout=new QVBoxLayout(this);
    mainLayout->addWidget(title);
    mainLayout->addLayout(top);mainLayout->addWidget(tableView);

    tableView->viewport()->installEventFilter(this);

    connect(btnAdd,&QPushButton::clicked,this,&ExercisesWindow::onAdd);
    connect(btnEdit,&QPushButton::clicked,this,&ExercisesWindow::onEdit);
    connect(comboCategory,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&ExercisesWindow::onFilterChanged);
    connect(editSearch,&QLineEdit::textChanged,this,&ExercisesWindow::onFilterChanged);
    loadData();
}

ExercisesWindow::~ExercisesWindow(){}

void ExercisesWindow::loadData(){
    QString f;QString c=comboCategory->currentText();if(c!="Все")f=QString("category_exercises='%1'").arg(c);
    QString s=editSearch->text().trimmed();if(!s.isEmpty()){if(!f.isEmpty())f+=" AND ";f+=QString("name_exercises ILIKE '%%1%'").arg(s);}
    model->setFilter(f);model->select();
}

void ExercisesWindow::onFilterChanged(){loadData();}

void ExercisesWindow::onAdd(){
    if(!m_isTrainer)return;
    QDialog d(this);d.setWindowTitle("Добавить упражнение");d.setFixedSize(380,320);QFormLayout f;
    QLineEdit *n=new QLineEdit();QComboBox *u=new QComboBox();u->addItems({"kg","sec","min","meters","reps"});
    QComboBox *c=new QComboBox();c->addItems({"Силовые","Кардио","Растяжка","Кроссфит","Другое"});
    QLineEdit *cc=new QLineEdit();cc->setPlaceholderText("Новая категория...");cc->setVisible(false);
    connect(c,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int i){cc->setVisible(c->itemText(i)=="Другое");});
    QLineEdit *desc=new QLineEdit();
    f.addRow("Название:",n);f.addRow("Ед.изм.:",u);f.addRow("Категория:",c);f.addRow("Своя категория:",cc);f.addRow("Описание:",desc);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&btns);d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){
        if(n->text().trimmed().isEmpty()){QMessageBox::warning(this,"Ошибка","Введите название");return;}
        QString cat=c->currentText();if(cat=="Другое"){cat=cc->text().trimmed();if(cat.isEmpty()){QMessageBox::warning(this,"Ошибка","Введите категорию");return;}}
        QSqlQuery ins(Database::getDb());ins.prepare("INSERT INTO exercises(name_exercises,unit_exercises,category_exercises,description_exercises) VALUES(:n,:u,:c,:d)");
        ins.bindValue(":n",n->text().trimmed());ins.bindValue(":u",u->currentText());ins.bindValue(":c",cat);ins.bindValue(":d",desc->text().trimmed());
        ins.exec();loadData();if(comboCategory->findText(cat)==-1)comboCategory->addItem(cat);
    }
}

void ExercisesWindow::onEdit(){
    QModelIndex idx = tableView->currentIndex();
    if (!idx.isValid() || !tableView->selectionModel()->isSelected(idx)) {
        QMessageBox::warning(this, "Предупреждение", "Выберите упражнение"); return;
    }
    int row = idx.row();
    QString name = model->data(model->index(row, 1)).toString();
    QString unit = model->data(model->index(row, 3)).toString();
    QString cat  = model->data(model->index(row, 4)).toString();
    QString desc = model->data(model->index(row, 2)).toString();
    int id = model->data(model->index(row, 0)).toInt();

    QDialog d(this); d.setWindowTitle("Редактировать упражнение"); d.setFixedSize(380, 320);
    QFormLayout f;
    QLineEdit *n = new QLineEdit(name);
    QComboBox *u = new QComboBox(); u->addItems({"kg","sec","min","meters","reps"}); u->setCurrentText(unit);
    QComboBox *c = new QComboBox(); c->addItems({"Силовые","Кардио","Растяжка","Кроссфит"}); c->setCurrentText(cat);
    QLineEdit *descEdit = new QLineEdit(desc);
    f.addRow("Название:",n); f.addRow("Ед.изм.:",u); f.addRow("Категория:",c); f.addRow("Описание:",descEdit);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); f.addRow(&btns); d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);
    connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){
        QSqlQuery up(Database::getDb());
        up.prepare("UPDATE exercises SET name_exercises=:n, unit_exercises=:u, category_exercises=:c, description_exercises=:d WHERE id_exercises=:id");
        up.bindValue(":n",n->text().trimmed()); up.bindValue(":u",u->currentText());
        up.bindValue(":c",c->currentText()); up.bindValue(":d",descEdit->text().trimmed()); up.bindValue(":id",id);
        up.exec(); loadData();
    }
}

bool ExercisesWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tableView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex idx = tableView->indexAt(mouseEvent->pos());
        if (!idx.isValid()) {
            tableView->clearSelection();
            tableView->setCurrentIndex(QModelIndex());
        }
    }
    return QWidget::eventFilter(obj, event);
}
