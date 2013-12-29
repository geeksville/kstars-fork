/***************************************************************************
                    optional.h - K Desktop Planetarium
                             -------------------
    begin                : Mon Dec 23 2013
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


#ifndef OPTIONAL_H
#define OPTIONAL_H

template <class T>
class Optional
{
public:
    Optional(T value) :
        m_Value(value), m_Defined(true)
    { }

    Optional(T value, bool defined) :
        m_Value(value), m_Defined(defined)
    { }

    Optional() :
        m_Value(T()), m_Defined(false)
    { }

    T value() const
    {
        return m_Value;
    }

    bool defined() const
    {
        return m_Defined;
    }

    void setValue(const T &value)
    {
        m_Value = value;
    }

    void setDefined(const bool defined)
    {
        m_Defined = defined;
    }

    operator T() const
    {
        return m_Value;
    }

private:
    T m_Value;
    bool m_Defined;
};

#endif // OPTIONAL_H
