/***************************************************************************
                          skyguide.h  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Oct 10 2011
    copyright            : (C) 2011 by Łukasz Jaśkiewicz
    email                : lucas.jaskiewicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SKYGUIDE_H
#define SKYGUIDE_H

#include "skyguidesspace.h"

#include <QString>
#include <QList>
#include <QDate>

class SkyGuidesSpace::Guide
{
public:
    Guide(const QString &title, const QString &description, const QString &language, const QList<Author*> &authors,
          const QString &thumbnailImg, const QString &creationDate, const QString &version, const QList<Slide*> &slides)
    {
        setGuide(title, description, language, authors, thumbnailImg, creationDate, version, slides);
    }

    ~Guide();

    QString title() const { return m_Title; }
    QString description() const { return m_Description; }
    QString language() const { return m_Language; }
    QList<Author*>* authors() { return &m_Authors; }
    QString thumbnailImage() const { return m_ThumbnailImage; }
    QString creationDate() const { return m_CreationDate; }
    QString version() const { return m_Version; }
    QList<Slide*>* slides() { return &m_Slides; }

    void setGuide(const QString &title, const QString &description, const QString &language, const QList<Author*> &authors,
                  const QString &thumbnailImg, const QString &creationDate, const QString &version, const QList<Slide*> &slides);

    QList<Author*> m_Authors;
    QList<Slide*> m_Slides;
private:
    QString m_Title;
    QString m_Description;
    QString m_Language;
    QString m_ThumbnailImage;
    QString m_CreationDate;
    QString m_Version;

};

#endif // SKYGUIDE_H
