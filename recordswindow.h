#ifndef RECORDSWINDOW_H
#define RECORDSWINDOW_H

#include <QWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlQuery>
#include <QHeaderView>
#include <QLabel>

class RecordsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RecordsWindow(QWidget *parent = nullptr);
    ~RecordsWindow();

private:
    QTableView      *tableViewRecords;
    QSqlQueryModel  *modelRecords;
    QLabel          *labelTitle;

    void loadRecords();
};

#endif // RECORDSWINDOW_H
