#ifndef TABSETS_H
#define TABSETS_H

#include "qdatetime.h"
#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
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
#include <QDoubleSpinBox>
#include <QDate>

class TabSets : public QWidget
{
    Q_OBJECT

public:
    explicit TabSets(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~TabSets();
    void setClientFilter(int clientId);
    void setDateFilter(QDate from, QDate to);
    void setWorkoutFilter(int sessionId);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onFilterChanged();

private:
    QTableView  *tableView;
    QComboBox   *comboFilter;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QSqlQueryModel *model;

    bool m_isTrainer;
    int  m_userId;

    void loadData();
    void loadFilter();
    int  selectedId();
    int m_selectedClientId = -1;
    int m_selectedSessionId = -1;
    QDate m_dateFrom;
    QDate m_dateTo;
};

#endif // TABSETS_H
