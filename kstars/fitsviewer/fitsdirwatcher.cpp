/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fitsdirwatcher.h"
#include <fits_debug.h>

FITSDirWatcher::FITSDirWatcher(QObject *parent) : QObject()
{
    m_Watcher.reset(new QFileSystemWatcher(this));

    // Connect the directory changed signal to our slot
    connect(m_Watcher.get(), &QFileSystemWatcher::directoryChanged, this, &FITSDirWatcher::onDirChanged);
}

FITSDirWatcher::~FITSDirWatcher()
{
}

// Start watching the specified directory
bool FITSDirWatcher::watchDir(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
    {
        qCDebug(KSTARS_FITS) << QString("Directory %1 does not exist").arg(path);
        return false;
    }

    // Store the current files in the directory
    QStringList files = dir.entryList(m_NameFilters, m_FilterFlags, m_SortFlags);
    for (const QString &file : files)
        m_CurrentFiles.push_back(dir.absoluteFilePath(file));

    m_WatchedPath = path;

    // Add the path to the watcher
    return m_Watcher->addPath(path);
}

// Stop watching the current directory
void FITSDirWatcher::stopWatching()
{
    if (!m_WatchedPath.isEmpty())
    {
        m_Watcher->removePath(m_WatchedPath);
        m_WatchedPath.clear();
        m_CurrentFiles.clear();
    }
}

void FITSDirWatcher::onDirChanged(const QString &path)
{
    if (path != m_WatchedPath)
        return;

    QDir dir(path);
    QStringList newFileList;
    QStringList files = dir.entryList(m_NameFilters, m_FilterFlags, m_SortFlags);
    for (const QString &file : files)
        newFileList.push_back(dir.absoluteFilePath(file));

    // Find files that are in newFileList but not in m_currentFiles
    QStringList newFiles;
    for (const QString &file : newFileList)
    {
        if (!m_CurrentFiles.contains(file))
            newFiles.push_back(file);
    }

    // If we found new files, update our list and emit the signal
    if (!newFiles.isEmpty())
    {
        m_CurrentFiles = newFileList;
        emit newFilesDetected(newFiles);
    }
}
