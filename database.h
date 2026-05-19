#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDebug>

class Database
{
public:
    static bool connect();
    static void close();
    static QSqlDatabase& getDb();
    static int currentUserId;
    static QString currentUserEmail;

private:
    static QSqlDatabase m_db;
};

#endif // DATABASE_H
