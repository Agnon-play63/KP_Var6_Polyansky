#ifndef WORKOUTSWINDOW_H
#define WORKOUTSWINDOW_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
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

class WorkoutsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WorkoutsWindow(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~WorkoutsWindow();

    void setClientFilter(int clientId, const QString &clientName);

signals:
    void clientSelected(int clientId);
    void dateFilterChanged(QDate from, QDate to);
    void workoutDoubleClicked(int sessionId);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onFilterChanged();

private:
    QTableView  *tableViewWorkouts;
    QDateEdit   *dateEditFrom;
    QDateEdit   *dateEditTo;
    QComboBox   *comboClientFilter;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QSqlQueryModel *modelWorkouts;

    bool m_isTrainer;
    int  m_userId;

    void loadWorkouts();
    void loadClientFilter();
    int  selectedId();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // WORKOUTSWINDOW_H
