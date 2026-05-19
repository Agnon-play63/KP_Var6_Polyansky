#include "clientswindow.h"
#include "database.h"
#include <QGuiApplication>
#include <QScreen>
#include <QItemSelectionModel>
#include <QMouseEvent>

ClientsWindow::ClientsWindow(bool isTrainer, int userId, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
    , m_isTrainer(isTrainer), m_userId(userId)
{
    setWindowTitle("Клиенты");
    resize(900, 600);
    setMinimumSize(700, 450);
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    QLabel *title = new QLabel("Клиенты");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

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
    model->setHeaderData(1, Qt::Horizontal, "Имя");
    model->setHeaderData(2, Qt::Horizontal, "Фамилия");
    model->setHeaderData(3, Qt::Horizontal, "Телефон");
    model->setHeaderData(11, Qt::Horizontal, "Тренер");
    tableView->setModel(model);
    for (int i = 0; i < model->columnCount(); i++) tableView->hideColumn(i);
    tableView->showColumn(1); tableView->showColumn(2);
    tableView->showColumn(3); tableView->showColumn(11);

    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(labelFilter); top->addWidget(editFilter);
    top->addWidget(btnFilter); if (m_isTrainer) top->addWidget(btnAdd);

    QHBoxLayout *bot = new QHBoxLayout();
    bot->addStretch(); bot->addWidget(btnEdit); bot->addWidget(btnDelete);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(title);
    mainLayout->addLayout(top); mainLayout->addWidget(tableView); mainLayout->addLayout(bot);

    connect(btnFilter, &QPushButton::clicked, this, &ClientsWindow::onFilterByCoach);
    connect(btnAdd, &QPushButton::clicked, this, &ClientsWindow::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &ClientsWindow::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &ClientsWindow::onDelete);

    connect(tableView, &QTableView::doubleClicked, [this](const QModelIndex &index) {
        if (!m_isTrainer) return;
        int id = model->data(model->index(index.row(), 0)).toInt();
        QString name = model->data(model->index(index.row(), 1)).toString() + " " +
                       model->data(model->index(index.row(), 2)).toString();
        emit clientDoubleClicked(id, name);
    });

    tableView->viewport()->installEventFilter(this);

    if (!m_isTrainer) {
        model->setFilter(QString("id_client = %1").arg(m_userId));
        labelFilter->setVisible(false); editFilter->setVisible(false);
        btnFilter->setVisible(false); btnAdd->setVisible(false);
    }
    loadData();
}

ClientsWindow::~ClientsWindow() {}

void ClientsWindow::loadData() { model->select(); }

int ClientsWindow::selectedId() {
    QModelIndex idx = tableView->currentIndex();
    if (!idx.isValid()) return -1;
    return model->data(model->index(idx.row(), 0)).toInt();
}

void ClientsWindow::onFilterByCoach() {
    QString c = editFilter->text().trimmed();
    model->setFilter(c.isEmpty() ? "" : QString("coach_client ILIKE '%%1%'").arg(c));
    model->select();
}

void ClientsWindow::onAdd() {
    if (!m_isTrainer) return;
    QDialog d(this); d.setWindowTitle("Добавить клиента"); d.setFixedSize(380, 540);
    QFormLayout f;
    QLineEdit *n = new QLineEdit(), *s = new QLineEdit(), *p = new QLineEdit();
    QLineEdit *e = new QLineEdit(), *pw = new QLineEdit(); pw->setEchoMode(QLineEdit::Password);
    QDateEdit *b = new QDateEdit(QDate::currentDate().addYears(-20)); b->setCalendarPopup(true);
    QComboBox *g = new QComboBox(); g->addItems({"M","F"});
    QDoubleSpinBox *w = new QDoubleSpinBox(); w->setRange(20,300); w->setValue(70); w->setDecimals(1);
    QSpinBox *h = new QSpinBox(); h->setRange(100,250); h->setValue(170);
    QComboBox *tr = new QComboBox();
    QSqlQuery qt(Database::getDb()); qt.exec("SELECT id_trainer, surname_trainer FROM trainer ORDER BY surname_trainer");
    while(qt.next()) tr->addItem(qt.value(1).toString(), qt.value(0).toInt());
    f.addRow("Имя:",n); f.addRow("Фамилия:",s); f.addRow("Телефон:",p);
    f.addRow("Email:",e); f.addRow("Пароль:",pw); f.addRow("Дата рожд:",b);
    f.addRow("Пол:",g); f.addRow("Вес:",w); f.addRow("Рост:",h); f.addRow("Тренер:",tr);
    QDialogButtonBox btns(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); f.addRow(&btns); d.setLayout(&f);
    connect(&btns,&QDialogButtonBox::accepted,&d,&QDialog::accept);
    connect(&btns,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()==QDialog::Accepted){
        if(n->text().trimmed().isEmpty()||s->text().trimmed().isEmpty()||e->text().trimmed().isEmpty()||pw->text().length()<6)
        {QMessageBox::warning(this,"Ошибка","Проверьте поля и пароль (≥6)");return;}
        QSqlQuery q(Database::getDb());
        q.prepare("INSERT INTO client(name_client,surname_client,number_client,email_client,date_of_birth_client,gender_client,kg_client,cm_client,password_client,id_trainer,coach_client) "
                  "VALUES(:n,:s,:p,:e,:b,:g,:w,:h,:pw,:tid,(SELECT surname_trainer FROM trainer WHERE id_trainer=:tid))");
        q.bindValue(":n",n->text().trimmed());q.bindValue(":s",s->text().trimmed());q.bindValue(":p",p->text().trimmed());
        q.bindValue(":e",e->text().trimmed());q.bindValue(":b",b->date());q.bindValue(":g",g->currentText());
        q.bindValue(":w",w->value());q.bindValue(":h",h->value());q.bindValue(":pw",pw->text());q.bindValue(":tid",tr->currentData().toInt());
        if(!q.exec())QMessageBox::warning(this,"Ошибка",q.lastError().text());
        loadData();
    }
}

void ClientsWindow::onEdit() {
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите клиента"); return; }

    QSqlQuery q(Database::getDb());
    q.prepare("SELECT * FROM client WHERE id_client=:id"); q.bindValue(":id", id); q.exec();
    if (!q.next()) return;

    QDialog d(this);
    d.setWindowTitle("Редактировать клиента");
    d.setFixedSize(m_isTrainer ? 350 : 380, m_isTrainer ? 300 : 520);
    QFormLayout f;

    if (m_isTrainer) {
        // Тренер: только вес, рост, тренер
        f.addRow("Имя:", new QLabel(q.value(1).toString()));
        f.addRow("Фамилия:", new QLabel(q.value(2).toString()));
        QDoubleSpinBox *weight = new QDoubleSpinBox(); weight->setRange(20,300); weight->setValue(q.value(8).toDouble()); weight->setDecimals(1); weight->setSuffix(" кг");
        QSpinBox *height = new QSpinBox(); height->setRange(100,250); height->setValue(q.value(9).toInt()); height->setSuffix(" см");
        QComboBox *tr = new QComboBox();
        QSqlQuery qt(Database::getDb()); qt.exec("SELECT id_trainer, surname_trainer FROM trainer ORDER BY surname_trainer");
        while (qt.next()) tr->addItem(qt.value(1).toString(), qt.value(0).toInt());
        for (int i = 0; i < tr->count(); i++) {
            if (tr->itemData(i).toInt() == q.value(12).toInt()) { tr->setCurrentIndex(i); break; }
        }
        f.addRow("Вес:", weight);
        f.addRow("Рост:", height);
        f.addRow("Тренер:", tr);

        QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        f.addRow(&btns); d.setLayout(&f);
        connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
        if (d.exec() == QDialog::Accepted) {
            QSqlQuery up(Database::getDb());
            up.prepare("UPDATE client SET kg_client=:w, cm_client=:h, id_trainer=:tid, "
                       "coach_client=(SELECT surname_trainer FROM trainer WHERE id_trainer=:tid) WHERE id_client=:id");
            up.bindValue(":w", weight->value()); up.bindValue(":h", height->value());
            up.bindValue(":tid", tr->currentData().toInt()); up.bindValue(":id", id);
            up.exec(); loadData();
        }
    } else {
        // Клиент: всё
        QLineEdit *name    = new QLineEdit(q.value(1).toString());
        QLineEdit *surname = new QLineEdit(q.value(2).toString());
        QLineEdit *phone   = new QLineEdit(q.value(3).toString());
        QLineEdit *email   = new QLineEdit(q.value(4).toString());
        QLineEdit *password = new QLineEdit(q.value(10).toString()); password->setEchoMode(QLineEdit::Password);
        QDateEdit *birth   = new QDateEdit(q.value(6).toDate()); birth->setCalendarPopup(true);
        QComboBox *gender  = new QComboBox(); gender->addItems({"M","F"}); gender->setCurrentText(q.value(7).toString());
        QDoubleSpinBox *weight = new QDoubleSpinBox(); weight->setRange(20,300); weight->setValue(q.value(8).toDouble()); weight->setDecimals(1); weight->setSuffix(" кг");
        QSpinBox *height   = new QSpinBox(); height->setRange(100,250); height->setValue(q.value(9).toInt()); height->setSuffix(" см");
        QComboBox *tr = new QComboBox();
        QSqlQuery qt(Database::getDb()); qt.exec("SELECT id_trainer, surname_trainer FROM trainer ORDER BY surname_trainer");
        while (qt.next()) tr->addItem(qt.value(1).toString(), qt.value(0).toInt());
        for (int i = 0; i < tr->count(); i++) {
            if (tr->itemData(i).toInt() == q.value(12).toInt()) { tr->setCurrentIndex(i); break; }
        }
        f.addRow("Имя:", name); f.addRow("Фамилия:", surname); f.addRow("Телефон:", phone);
        f.addRow("Email:", email); f.addRow("Пароль:", password); f.addRow("Дата рожд:", birth);
        f.addRow("Пол:", gender); f.addRow("Вес:", weight); f.addRow("Рост:", height); f.addRow("Тренер:", tr);

        QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        f.addRow(&btns); d.setLayout(&f);
        connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);
        if (d.exec() == QDialog::Accepted) {
            if (name->text().trimmed().isEmpty() || surname->text().trimmed().isEmpty() ||
                email->text().trimmed().isEmpty() || password->text().length() < 6) {
                QMessageBox::warning(this, "Ошибка", "Проверьте поля и пароль (≥6)"); return;
            }
            QSqlQuery up(Database::getDb());
            up.prepare("UPDATE client SET name_client=:n, surname_client=:s, number_client=:p, email_client=:e, "
                       "date_of_birth_client=:b, gender_client=:g, kg_client=:w, cm_client=:h, password_client=:pw, "
                       "id_trainer=:tid, coach_client=(SELECT surname_trainer FROM trainer WHERE id_trainer=:tid) "
                       "WHERE id_client=:id");
            up.bindValue(":n", name->text().trimmed()); up.bindValue(":s", surname->text().trimmed());
            up.bindValue(":p", phone->text().trimmed()); up.bindValue(":e", email->text().trimmed());
            up.bindValue(":b", birth->date()); up.bindValue(":g", gender->currentText());
            up.bindValue(":w", weight->value()); up.bindValue(":h", height->value());
            up.bindValue(":pw", password->text()); up.bindValue(":tid", tr->currentData().toInt());
            up.bindValue(":id", id);
            up.exec(); loadData();
        }
    }
}
void ClientsWindow::onDelete() {
    int id = selectedId();
    if (id == -1) { QMessageBox::warning(this, "Предупреждение", "Выберите клиента"); return; }
    if (QMessageBox::question(this, "Подтверждение", "Удалить клиента?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(Database::getDb()); q.prepare("DELETE FROM client WHERE id_client=:id"); q.bindValue(":id", id); q.exec();
        loadData();
    }
}

bool ClientsWindow::eventFilter(QObject *obj, QEvent *event)
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
