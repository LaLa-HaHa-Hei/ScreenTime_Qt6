#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>

class DatabaseManager
{
public:
    static DatabaseManager *instance(const QString& dbPath = QString());
    bool OpenDatabase();
    bool CreateTable(const QString& tableName);
    bool InsertOrUpdateAppTime(const QString& tableName, const QString& appName, int seconds);
    QSqlQuery GetAppUsageData(const QString& tableName, const QString& orderBy = "seconds DESC");
    int GetTotalSeconds(const QString& tableName);
    QStringList GetTableNameList();
    bool DeleteTable(const QString& tableName);

private:
    DatabaseManager(const QString& dbPath);
    ~DatabaseManager();
    QSqlDatabase _db;
    QMutex _dbMutex;  // 锁机制，防止并发访问
};

#endif // DATABASEMANAGER_H
