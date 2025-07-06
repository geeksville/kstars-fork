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
