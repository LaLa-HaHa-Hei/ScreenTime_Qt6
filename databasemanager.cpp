#include "databasemanager.h"
#include <QDebug>

DatabaseManager::DatabaseManager(const QString& dbPath)
{
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(dbPath);
    if (!_db.open()) {
        qCritical() << "无法打开数据库：" << _db.lastError().text();
    }
}

DatabaseManager::~DatabaseManager()
{
    if (_db.isOpen()) {
        _db.close();
    }
}

DatabaseManager *DatabaseManager::instance(const QString& dbPath)
{
    static DatabaseManager* instance = nullptr;
    if (!instance) {
        instance = new DatabaseManager(dbPath);
    }
    return instance;
}

bool DatabaseManager::OpenDatabase()
{
    if (_db.isOpen()) {
        return true; // 避免重复打开
    }
    if (!_db.open()) {
        qCritical() << "无法打开数据库：" << _db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::CreateTable(const QString& tableName)
{
    QMutexLocker locker(&_dbMutex);
    QSqlQuery query(_db);
    QString createTableQuery = QString(
                                   "CREATE TABLE IF NOT EXISTS %1 ("
                                   "name TEXT PRIMARY KEY, "
                                   "seconds INTEGER)").arg(tableName);

    if (!query.exec(createTableQuery)) {
        qCritical() << QString("无法创建表：%1").arg(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::InsertOrUpdateAppTime(const QString& tableName, const QString& appName, int seconds)
{
    QMutexLocker locker(&_dbMutex);
    QSqlQuery query(_db);
    QString updateQuery = QString(
                              "INSERT INTO %1 (name, seconds) VALUES (:name, :seconds) "
                              "ON CONFLICT(name) DO UPDATE SET seconds = seconds + :seconds").arg(tableName);

    query.prepare(updateQuery);
    query.bindValue(":name", appName);
    query.bindValue(":seconds", seconds);

    if (!query.exec()) {
        qCritical() << QString("无法更新使用时间：%1").arg(query.lastError().text());
        return false;
    }
    return true;
}

QSqlQuery DatabaseManager::GetAppUsageData(const QString& tableName, const QString& orderBy)
{
    QMutexLocker locker(&_dbMutex);
    QSqlQuery query(_db);
    QString selectQuery = QString("SELECT name, seconds FROM %1 ORDER BY %2").arg(tableName, orderBy);

    if (!query.exec(selectQuery)) {
        qCritical() << QString("无法查询应用使用数据：%1").arg(query.lastError().text());
    }
    return query;
}

int DatabaseManager::GetTotalSeconds(const QString& tableName)
{
    QMutexLocker locker(&_dbMutex);
    QString totalSecondsQuery = QString("SELECT SUM(seconds) FROM %1").arg(tableName);
    QSqlQuery query(_db);
    if(!query.exec(totalSecondsQuery))
    {
        qCritical() << QString("无法获取时间总和：%1").arg(query.lastError().text());
        return 0;
    }
    if (query.next())
    {
        return query.value(0).toInt();
    }
    else
    {
        qCritical() << QString("没有找到时间总和：%1").arg(query.lastError().text());
        return 0;
    }
}

QStringList DatabaseManager::GetTableNameList()
{
    QMutexLocker locker(&_dbMutex);
    return _db.tables();
}

bool DatabaseManager::DeleteTable(const QString& tableName)
{
    QMutexLocker locker(&_dbMutex);
    QString deleteQuery = QString("DROP TABLE IF EXISTS %1").arg(tableName);
    QSqlQuery query(_db);
    if (!query.exec(deleteQuery)) {
        qCritical() << QString("无法删除表%1").arg(query.lastError().text());
        return false;
    }
    return true;
}
