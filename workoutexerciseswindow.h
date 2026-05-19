#ifndef WORKOUTEXERCISESWINDOW_H
#define WORKOUTEXERCISESWINDOW_H

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

class WorkoutExercisesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WorkoutExercisesWindow(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~WorkoutExercisesWindow();

    void setClientFilter(int clientId);
    void setDateFilter(QDate from, QDate to);
    void setWorkoutFilter(int sessionId);

private slots:
    void onAddExercise(); void onEditExercise(); void onDeleteExercise();
    void onExerciseClicked();
    void onAddSet(); void onEditSet(); void onDeleteSet();
    void onFilterChanged();

private:
    QTableView *tableViewExercises; QComboBox *comboSession; QComboBox *comboExercise;
    QPushButton *btnAddExercise, *btnEditExercise, *btnDeleteExercise;
    QSqlQueryModel *modelExercises;

    QGroupBox *groupBoxDetails; QTableView *tableViewSets;
    QComboBox *comboFilterSet;
    QPushButton *btnAddSet, *btnEditSet, *btnDeleteSet;
    QSqlQueryModel *modelSets;

    bool m_isTrainer; int m_userId;
    int m_selectedClientId=-1, m_selectedSessionId=-1;
    QDate m_dateFrom, m_dateTo;

    void loadExercises(); void loadFilters(); void loadSets(int weId); void loadSetsFilter();
    int selectedExerciseId(); int selectedSetId();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // WORKOUTEXERCISESWINDOW_H
