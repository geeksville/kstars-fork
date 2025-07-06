/*
    SPDX-FileCopyrightText: 2025 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QWidget>
#include "fitsviewer/fitsview.h"
#include "ekos/capture/capturehistorynavigation.h"

class FocusFITSView : public FITSView
{
    Q_OBJECT

public:
    explicit FocusFITSView(QWidget *parent = nullptr);

    /**
     * @brief showNavigation Show the navigation bar
     */
    void showNavigation(bool show) { m_focusHistoryNavigation->setVisible(show); }

public slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    QSharedPointer<CaptureHistoryNavigation> m_focusHistoryNavigation;
};
