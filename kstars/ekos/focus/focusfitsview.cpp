/*
    SPDX-FileCopyrightText: 2025 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "focusfitsview.h"
#include "QGraphicsOpacityEffect"

FocusFITSView::FocusFITSView(QWidget *parent): FITSView(parent, FITS_FOCUS)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    createFloatingToolBar();

    m_focusHistoryNavigation.reset(new CaptureHistoryNavigation(this));
    auto * eff = new QGraphicsOpacityEffect(this);
    eff->setOpacity(0.5);
    m_focusHistoryNavigation->setGraphicsEffect(eff);
    m_focusHistoryNavigation->resize(width(), m_focusHistoryNavigation->height());
    m_focusHistoryNavigation->move(0, height() - m_focusHistoryNavigation->height());

    m_focusHistoryNavigation->filenameValue->setVisible(false);

    showNavigation(false);
    m_focusHistoryNavigation->raise();
}

void FocusFITSView::resizeEvent(QResizeEvent *event)
{
    FITSView::resizeEvent(event);
    m_focusHistoryNavigation->resize(width(), m_focusHistoryNavigation->height());
    m_focusHistoryNavigation->move(0, height() - m_focusHistoryNavigation->height());
}
