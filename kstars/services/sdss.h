#ifndef SDSS_H
#define SDSS_H

#include <QUrl>

class SkyPoint;

class SDSS
{
public:
    static QUrl getSDSSUrl(const SkyPoint * const point);
};

#endif // SDSS_H
