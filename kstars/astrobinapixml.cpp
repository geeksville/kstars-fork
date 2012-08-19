/***************************************************************************
                          astrobinapixml.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Wed Aug 8 2012
    copyright            : (C) 2012 by Lukasz Jaskiewicz and Rafal Kulaga
    email                : lucas.jaskiewicz@gmail.com, rl.kulaga@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "astrobinapixml.h"

#include <QNetworkReply>
#include <QXmlStreamReader>

void AstroBinApiXml::replyFinished(QNetworkReply *reply)
{
    m_XmlReader = new QXmlStreamReader(reply->readAll());
    bool result = false;

    while(!m_XmlReader->atEnd()) {
        // read begin element
        m_XmlReader->readNext();
        m_XmlReader->readNext();

        if(m_XmlReader->isStartElement()) {
            if(m_XmlReader->name() == "response") {
                result = readResponse();
            } else {
                skipUnknownElement();
            }
        } else {
            break;
        }
    }

    emit searchFinished(result);
}

bool AstroBinApiXml::readResponse()
{
    while(!m_XmlReader->atEnd()) {
        m_XmlReader->readNext();

        if(m_XmlReader->isEndElement()) {
            break;
        }

        if(m_XmlReader->isStartElement()) {
            if(m_XmlReader->name() == "objects") {
                readObjects();
            } else if(m_XmlReader->name() == "meta") {
                readMetadata();
            } else {
                skipUnknownElement();
            }
        }
    }

    return false;
}

bool AstroBinApiXml::readMetadata()
{
    while(!m_XmlReader->atEnd()) {
        m_XmlReader->readNext();

        if(m_XmlReader->isEndElement()) {
            break;
        }

        if(m_XmlReader->isStartElement()) {
            QStringRef elementName = m_XmlReader->name();
            if(elementName == "next") {
                m_LastSearchResult.m_Metadata.m_NextUrlPostfix = m_XmlReader->readElementText();
            } else if(elementName == "total_count") {
                m_LastSearchResult.m_Metadata.m_TotalResultCount = m_XmlReader->readElementText().toInt();
            } else if(elementName == "previous") {
                m_LastSearchResult.m_Metadata.m_PreviousUrlPostfix = m_XmlReader->readElementText();
            } else if(elementName == "limit") {
                m_LastSearchResult.m_Metadata.m_Limit = m_XmlReader->readElementText().toInt();
            } else if(elementName == "offset") {
                m_LastSearchResult.m_Metadata.m_Offset = m_XmlReader->readElementText().toInt();
            } else {
                skipUnknownElement();
            }
        }
    }

    return true;
}

bool AstroBinApiXml::readObjects()
{
    while(!m_XmlReader->atEnd()) {
        m_XmlReader->readNext();

        if(m_XmlReader->isEndElement()) {
            break;
        }

        if(m_XmlReader->isStartElement()) {
            if(m_XmlReader->name() == "object") {
                readObject();
            } else {
                skipUnknownElement();
            }
        }
    }

    return true;
}

bool AstroBinApiXml::readObject()
{
    while(!m_XmlReader->atEnd()) {
        m_XmlReader->readNext();

        if(m_XmlReader->isEndElement()) {
            break;
        }

        if(m_XmlReader->isStartElement()) {
            skipUnknownElement();
        }
    }

    return true;
}

void AstroBinApiXml::skipUnknownElement()
{
    while(!m_XmlReader->atEnd()) {
        m_XmlReader->readNext();

        if(m_XmlReader->isEndElement()) {
            break;
        }

        if(m_XmlReader->isStartElement()) {
            skipUnknownElement();
        }
    }
}
