/***************************************************************************
                    observer.cpp - K Desktop Planetarium
                             -------------------
    begin                : Tue Nov 12 2013
    copyright            : (C) 2013 by Rafal Kulaga
    email                : rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "observer.h"

using namespace Logging;

Observer::Observer(const int id, const QString &name, const QString &surname,
                   const double fstOffset, const QStringList &contacts,
                   const QList<Account> &accounts) :
    m_Id(id), m_Name(name), m_Surname(surname), m_FstOffset(fstOffset),
    m_Contacts(contacts), m_Accounts(accounts)
{ }
