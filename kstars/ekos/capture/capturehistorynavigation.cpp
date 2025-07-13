/*
    SPDX-FileCopyrightText: 2025 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "capturehistorynavigation.h"

CaptureHistoryNavigation::CaptureHistoryNavigation(QWidget *parent)
    : QWidget{parent}
{
    setupUi(this);
    // use white as text color
    setStyleSheet("color: rgb(255, 255, 255);");
}

void CaptureHistoryNavigation::addRun()
{
    m_lastRun++;
    m_currentRun = m_lastRun;

    refreshNavigation();
}

void CaptureHistoryNavigation::removeRun(int run)
{
    if (run <= m_lastRun)
    {
        m_captureHistory.removeAt(run);
        m_lastRun--;
        if (run <= m_currentRun && (run > 1 || m_captureHistory.size() <= 1))
            m_currentRun--;

        refreshNavigation();
    }
}

void CaptureHistoryNavigation::refreshHistory()
{
    for (QList<CaptureHistory>::iterator history = m_captureHistory.begin(); history != m_captureHistory.end(); history++)
        history->updateTargetStatistics(false);
}



CaptureHistory &CaptureHistoryNavigation::captureHistory(int run)
{
    // ensure that the history contains enough entries
    while (m_captureHistory.size() <= run)
        m_captureHistory.append(CaptureHistory());

    return m_captureHistory[run];
}

bool CaptureHistoryNavigation::showFirstFrame()
{
    if (captureHistory(m_currentRun).first())
    {
        refreshNavigation();
        return true;
    }
    else
        return false;
}

bool CaptureHistoryNavigation::showLastFrame()
{
    if (captureHistory(m_currentRun).last())
    {
        refreshNavigation();
        return true;
    }
    else
        return false;
}

bool CaptureHistoryNavigation::showPreviousFrame()
{
    if (captureHistory(m_currentRun).backward())
    {
        refreshNavigation();
        return true;
    }
    else
        return false;
}

bool CaptureHistoryNavigation::showNextFrame()
{
    if (captureHistory(m_currentRun).forward())
    {
        refreshNavigation();
        return true;
    }
    else
        return false;
}

bool CaptureHistoryNavigation::showPreviousRun()
{
    if (m_currentRun > 1)
    {
        m_currentRun--;
        refreshNavigation();
        return true;
    }
    else
        return false;
}

bool CaptureHistoryNavigation::showNextRun()
{
    if (m_currentRun < m_lastRun)
    {
        m_currentRun++;
        refreshNavigation();
        return true;
    }
    else
        return false;
}

void CaptureHistoryNavigation::refreshNavigation()
{
    bool backward = (captureHistory(m_currentRun).position() > 0);
    bool forward  = (captureHistory(m_currentRun).size() > 0
                     && captureHistory(m_currentRun).position() + 1 < captureHistory(m_currentRun).size());
    historyFirstB->setEnabled(backward);
    historyBackwardB->setEnabled(backward);
    historyForwardB->setEnabled(forward);
    historyLastB->setEnabled(forward);
    historyPreviousRunB->setEnabled(m_currentRun > 1);
    historyNextRunB->setEnabled(m_currentRun < m_lastRun);
}
