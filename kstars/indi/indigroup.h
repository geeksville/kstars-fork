/*  INDI Group
    SPDX-FileCopyrightText: 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    SPDX-License-Identifier: GPL-2.0-or-later

    JM Changelog
    2004-16-1:	Start

*/

#pragma once

#include <indiproperty.h>

#include <QString>
#include <QList>
#include <QPointer>

class INDI_P;
class INDI_D;

class QFrame;
class QVBoxLayout;
class QSpacerItem;
class QScrollArea;

/**
 * @class INDI_G
 * INDI_G represents a collection of INDI properties that share a common group. The group is usually represented in the GUI as a separate tab with the device tab.
 *
 * @author Jasem Mutlaq
 */
class INDI_G
{
    public:
        INDI_G(INDI_D *idv, const QString &inName);
        ~INDI_G();

        bool addProperty(const INDI::Property newProperty);

        bool removeProperty(const QString &name);
        INDI_P *getProperty(const QString &name) const;
        QFrame *getContainer() const
        {
            return m_PropertiesContainer;
        }
        QScrollArea *getScrollArea() const
        {
            return m_ScrollArea;
        }
        const QString &getName() const
        {
            return name;
        }

        INDI_D *getDevice() const
        {
            return dp;
        }

        QList<INDI_P *> getProperties() const
        {
            return m_PropertiesList;
        }

        int size() const
        {
            return m_PropertiesList.count();
        }

    private:
        void resetLayout();

        // Group name
        QString name;
        // Parent device
        INDI_D *dp {nullptr};
        // Properties container
        QPointer<QFrame> m_PropertiesContainer;
        // Properties layout
        QPointer<QVBoxLayout> m_PropertiesLayout;
        // Vertical spacer
        QSpacerItem *m_VerticalSpacer {nullptr};
        QScrollArea *m_ScrollArea {nullptr};
        QList<INDI_P *> m_PropertiesList;
        bool m_Dirty { false };
};
