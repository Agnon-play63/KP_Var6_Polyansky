#ifndef TABCLIENTS_H
#define TABCLIENTS_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>

class TabClients : public QWidget
{
    Q_OBJECT

public:
    explicit TabClients(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~TabClients();

    void saveChanges();

private slots:
    void onFilterByCoach();
    void onAdd();
    void onEdit();
    void onDelete();

private:
    QTableView  *tableView;
    QLineEdit   *editFilter;
    QPushButton *btnFilter;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QSqlTableModel *model;

    bool m_isTrainer;
    int  m_userId;

    void loadData();
    int  selectedId();
};

#endif // TABCLIENTS_H
