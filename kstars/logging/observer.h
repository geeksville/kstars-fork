/***************************************************************************
                    observer.h - K Desktop Planetarium
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


#ifndef OBSERVER_H
#define OBSERVER_H

#include "optional.h"
#include "QString"
#include "QStringList"

namespace Logging
{

class Observer
{
public:
    class Account
    {
    public:
        Account(const QString &account, const QString &service) :
            m_Account(account), m_Service(service)
        { }

        QString account() const
        {
            return m_Account;
        }

        QString service() const
        {
            return m_Service;
        }

        void setAccount(const QString &account)
        {
            m_Account = account;
        }

        void setService(const QString &service)
        {
            m_Service = service;
        }

    private:
        QString m_Account;
        QString m_Service;
    };


    Observer(const int id, const QString &name, const QString &surname);

    Observer(const int id, const QString &name, const QString &surname,
             const double fstOffset, const QStringList &contacts,
             const QList<Account> &accounts);

    int id() const
    {
        return m_Id;
    }

    QString name() const
    {
        return m_Name;
    }

    QString surname() const
    {
        return m_Surname;
    }

    QStringList contacts() const
    {
        return m_Contacts;
    }

    QList<Account> accounts() const
    {
        return m_Accounts;
    }

    Optional<double> fstOffset() const
    {
        return m_FstOffset;
    }

    void setName(const QString &name)
    {
        m_Name = name;
    }

    void setSurname(const QString &surname)
    {
        m_Surname = surname;
    }

    void setContacts(const QStringList &contacts)
    {
        m_Contacts = contacts;
    }

    void setAccounts(const QList<Account> &accounts)
    {
        m_Accounts = accounts;
    }

    void setFstOffset(const Optional<double> fstOffset)
    {
        m_FstOffset = fstOffset;
    }

private:
    int m_Id;
    QString m_Name;
    QString m_Surname;
    QStringList m_Contacts;
    QList<Account> m_Accounts;
    Optional<double> m_FstOffset;
};

}

#endif // OBSERVER_H
