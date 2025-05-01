/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <fits_debug.h>

#include <QObject>
#include <QFileSystemWatcher>
#include <QString>
#include <QDir>

/**
 * @brief The FITSDirWatcher holds routines for monitoring a directory for new files
 * @author John Evans
 */
class FITSDirWatcher : public QObject
{
    Q_OBJECT

  public:
    explicit FITSDirWatcher(QObject *parent = nullptr);
    virtual ~FITSDirWatcher() override;

    bool watchDir(const QString &path);
    void stopWatching();
    const QStringList getCurrentFiles() const
    {
        return m_CurrentFiles;
    }

  signals:
    void newFilesDetected(const QStringList &filePaths);

  private slots:
    void onDirChanged(const QString &path);

  private:
    QSharedPointer<QFileSystemWatcher> m_Watcher;
    QString m_WatchedPath;
    QStringList m_CurrentFiles;
    QStringList m_NameFilters { "*.fits", "*.fits.fz", "*.fit", "*.fts" };
    QDir::Filters m_FilterFlags = QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks;
    QDir::SortFlags m_SortFlags = QDir::Time | QDir::Reversed;
};
