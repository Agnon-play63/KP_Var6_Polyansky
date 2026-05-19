#ifndef EXERCISESWINDOW_H
#define EXERCISESWINDOW_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
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

class ExercisesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ExercisesWindow(bool isTrainer, QWidget *parent = nullptr);
    ~ExercisesWindow();

private slots:
    void onAdd();
    void onEdit();
    void onFilterChanged();

private:
    QTableView *tableView;
    QComboBox *comboCategory;
    QLineEdit *editSearch;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QSqlTableModel *model;
    bool m_isTrainer;
    void loadData();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // EXERCISESWINDOW_H
