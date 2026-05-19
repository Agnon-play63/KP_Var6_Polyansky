#include "tabexercises.h"
#include "database.h"

TabExercises::TabExercises(bool isTrainer, QWidget *parent)
    : QWidget(parent), m_isTrainer(isTrainer)
{
    QLabel *lblCat = new QLabel("Категория:");
    comboCategory = new QComboBox();
    comboCategory->addItems({"Все", "Силовые", "Кардио", "Растяжка", "Кроссфит"});

    QLabel *lblSearch = new QLabel("Поиск:");
    editSearch = new QLineEdit();
    editSearch->setPlaceholderText("Введите название упражнения...");

    btnAdd = new QPushButton("➕ Добавить");
    btnAdd->setStyleSheet("font-weight: bold;");
    if (!m_isTrainer) {
        btnAdd->setVisible(false);
    }

    tableView = new QTableView();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);

    model = new QSqlTableModel(this, Database::getDb());
    model->setTable("exercises");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Название");
    model->setHeaderData(2, Qt::Horizontal, "Описание");
    model->setHeaderData(3, Qt::Horizontal, "Ед. изм.");
    model->setHeaderData(4, Qt::Horizontal, "Категория");
    model->setHeaderData(5, Qt::Horizontal, "Дата добавл.");

    tableView->setModel(model);
    tableView->hideColumn(0);
    tableView->hideColumn(2);
    tableView->hideColumn(5);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(lblCat); topLayout->addWidget(comboCategory);
    topLayout->addWidget(lblSearch); topLayout->addWidget(editSearch);
    topLayout->addStretch(); topLayout->addWidget(btnAdd);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableView);

    connect(btnAdd, &QPushButton::clicked, this, &TabExercises::onAdd);
    connect(comboCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TabExercises::onFilterChanged);
    connect(editSearch, &QLineEdit::textChanged, this, &TabExercises::onFilterChanged);

    loadData();
}

TabExercises::~TabExercises() {}

void TabExercises::saveChanges()
{
    if (model->isDirty()) model->submitAll();
}

void TabExercises::loadData()
{
    QString filter;
    QString cat = comboCategory->currentText();
    if (cat != "Все") filter = QString("category_exercises = '%1'").arg(cat);

    QString s = editSearch->text().trimmed();
    if (!s.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("name_exercises ILIKE '%%1%'").arg(s);
    }

    model->setFilter(filter);
    model->select();
}

void TabExercises::onFilterChanged() { loadData(); }

void TabExercises::onAdd()
{
    if (!m_isTrainer) return;

    QDialog d(this);
    d.setWindowTitle("Добавить упражнение");
    d.setFixedSize(380, 320);

    QFormLayout form;

    QLineEdit *name = new QLineEdit();

    QComboBox *unit = new QComboBox();
    unit->addItems({"kg", "sec", "min", "meters", "reps"});

    QComboBox *cat = new QComboBox();
    cat->addItems({"Силовые", "Кардио", "Растяжка", "Кроссфит", "Другое"});

    QLineEdit *customCat = new QLineEdit();
    customCat->setPlaceholderText("Введите название новой категории...");
    customCat->setVisible(false);

    // Показать/скрыть поле ввода при выборе "Другое"
    connect(cat, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        customCat->setVisible(cat->itemText(index) == "Другое");
    });

    QLineEdit *desc = new QLineEdit();

    form.addRow("Название:", name);
    form.addRow("Ед. изм.:", unit);
    form.addRow("Категория:", cat);
    form.addRow("Своя категория:", customCat);
    form.addRow("Описание:", desc);

    QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&btns);
    d.setLayout(&form);

    connect(&btns, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &d, &QDialog::reject);

    if (d.exec() == QDialog::Accepted) {
        if (name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите название упражнения");
            return;
        }

        QString category = cat->currentText();
        if (category == "Другое") {
            category = customCat->text().trimmed();
            if (category.isEmpty()) {
                QMessageBox::warning(this, "Ошибка", "Введите название новой категории");
                return;
            }
        }

        QSqlQuery ins(Database::getDb());
        ins.prepare("INSERT INTO exercises (name_exercises, unit_exercises, category_exercises, description_exercises) "
                    "VALUES (:n, :u, :c, :d)");
        ins.bindValue(":n", name->text().trimmed());
        ins.bindValue(":u", unit->currentText());
        ins.bindValue(":c", category);
        ins.bindValue(":d", desc->text().trimmed());
        ins.exec();
        loadData();

        // Обновить комбо-фильтр, если добавили новую категорию
        if (comboCategory->findText(category) == -1) {
            comboCategory->addItem(category);
        }
    }
}
