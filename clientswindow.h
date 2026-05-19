#ifndef CLIENTSWINDOW_H
#define CLIENTSWINDOW_H

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

class ClientsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClientsWindow(bool isTrainer, int userId, QWidget *parent = nullptr);
    ~ClientsWindow();

signals:
    void clientDoubleClicked(int clientId, const QString &clientName);

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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // CLIENTSWINDOW_H
