#ifndef DSS_H
#define DSS_H

#include "dms.h"

#include <QUrl>

class SkyPoint;

class DSS
{
public:
    /**
     *@short Create a URL to obtain a DSS image for a given SkyPoint
     *@note If SkyPoint is a DeepSkyObject, this method automatically
     *decides the image size required to fit the object.
     */
    static QUrl getDSSUrl(const SkyPoint * const point);

    /**
     *@short Create a URL to obtain a DSS image for a given RA, Dec
     *@param RA The J2000.0 Right Ascension of the point
     *@param Dec The J2000.0 Declination of the point
     *@param width The width of the image in arcminutes
     *@param height The height of the image in arcminutes
     *@note This method resets height and width to fall within the range accepted by DSS
     */
    static QUrl getDSSUrl(const dms &ra, const dms &dec, float width = 0, float height = 0);
};

#endif // DSS_H
