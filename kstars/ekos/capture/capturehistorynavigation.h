/*
    SPDX-FileCopyrightText: 2025 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "ui_capturehistorynavigation.h"

#include <QWidget>

class CaptureHistoryNavigation : public QWidget, public Ui::CaptureHistoryNavigation
{
    Q_OBJECT

public:
    explicit CaptureHistoryNavigation(QWidget *parent = nullptr);


};
