/***************************************************************************
            kshelplabel.cpp - Help label used to document astronomical terms
                             -------------------
    begin                : Wed 1 Dec 2010
    copyright            : (C) 2010 by Valery Kharitonov
    email                : kharvd@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kshelplabel.h"
#include <ktoolinvocation.h>
#include <QtGui/QMessageBox>

KSHelpLabel::KSHelpLabel(const QString& text, const QString& anchor,
			 QWidget *parent) : QLabel(parent), m_anchor(anchor)
{
    setText(text);
    updateText();
    connect(this, SIGNAL(linkActivated(QString)), SLOT(slotShowDefinition(QString)));
}

KSHelpLabel::KSHelpLabel(QWidget *parent) : QLabel(parent)
{
    connect(this, SIGNAL(linkActivated(QString)), SLOT(slotShowDefinition(QString)));
}

void KSHelpLabel::setAnchor(const QString& anchor) {
    m_anchor = anchor;
    updateText();
}

void KSHelpLabel::updateText() {
    QLabel::setText("<a href=\"ai-" + m_anchor + "\">" + text() + "</a>");
}

void KSHelpLabel::slotShowDefinition(const QString & term) {
    KToolInvocation::invokeHelp(term);
}

void KSHelpLabel::setText(const QString& txt) {
    m_cleanText = txt;
    QLabel::setText("<a href=\"ai-" + m_anchor + "\">" + m_cleanText + "</a>");
}

