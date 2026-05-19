#include "database.h"

QSqlDatabase Database::m_db = QSqlDatabase();
int Database::currentUserId = -1;
QString Database::currentUserEmail = "";

bool Database::connect()
{
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setHostName("localhost");
    m_db.setPort(5432);
    m_db.setDatabaseName("postgres");
    m_db.setUserName("postgres");
    m_db.setPassword("Poly9654Ars");

    if (!m_db.open()) {
        qDebug() << "Ошибка подключения к БД:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Подключение к БД успешно";
    return true;
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

QSqlDatabase& Database::getDb()
{
    return m_db;
}
