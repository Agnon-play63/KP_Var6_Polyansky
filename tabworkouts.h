#ifndef TABWORKOUTS_H
#define TABWORKOUTS_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QLineEdit>

class TabWorkouts : public QWidget
{
    Q_OBJECT

public:
    explicit TabWorkouts(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~TabWorkouts();

signals:
    void clientSelected(int clientId);
    void dateFilterChanged(QDate from, QDate to);
    void workoutSelected(int sessionId);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onFilterChanged();
    void onWorkoutClicked();

private:
    QTableView  *tableViewWorkouts;
    QTableView  *tableViewDetails;
    QGroupBox   *groupBoxDetails;
    QDateEdit   *dateEditFrom;
    QDateEdit   *dateEditTo;
    QComboBox   *comboClientFilter;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QSqlQueryModel *modelWorkouts;
    QSqlQueryModel *modelDetails;

    bool m_isTrainer;
    int  m_userId;

    void loadWorkouts();
    void loadDetails(int sessionId);
    void loadClientFilter();
    int  selectedId();
};

#endif // TABWORKOUTS_H
