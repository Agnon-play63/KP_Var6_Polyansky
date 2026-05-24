#ifndef TABWORKOUTEXERCISES_H
#define TABWORKOUTEXERCISES_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QDate>
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
#include <QDoubleSpinBox>
#include <QLineEdit>

class TabWorkoutExercises : public QWidget
{
    Q_OBJECT

public:
    explicit TabWorkoutExercises(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~TabWorkoutExercises();

    void setClientFilter(int clientId);
    void setDateFilter(QDate from, QDate to);
    void setWorkoutFilter(int sessionId);

private slots:
    void onAddExercise();
    void onEditExercise();
    void onDeleteExercise();
    void onExerciseClicked();
    void onAddSet();
    void onEditSet();
    void onDeleteSet();
    void onFilterChanged();

private:
    // Верх — таблица выполненных упражнений
    QTableView  *tableViewExercises;
    QComboBox   *comboSession;
    QComboBox   *comboExercise;
    QPushButton *btnAddExercise;
    QPushButton *btnEditExercise;
    QPushButton *btnDeleteExercise;
    QSqlQueryModel *modelExercises;

    // Низ — детали (подходы)
    QGroupBox   *groupBoxDetails;
    QTableView  *tableViewSets;
    QComboBox   *comboFilterSet;
    QPushButton *btnAddSet;
    QPushButton *btnEditSet;
    QPushButton *btnDeleteSet;
    QSqlQueryModel *modelSets;

    bool m_isTrainer;
    int  m_userId;
    int  m_selectedClientId = -1;
    int  m_selectedSessionId = -1;
    QDate m_dateFrom;
    QDate m_dateTo;

    void loadExercises();
    void loadFilters();
    void loadSets(int workoutExerciseId);
    void loadSetsFilter();
    int  selectedExerciseId();
    int  selectedSetId();
};

#endif // TABWORKOUTEXERCISES_H
