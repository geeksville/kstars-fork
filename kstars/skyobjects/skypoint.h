/***************************************************************************
                          skypoint.h  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001-2005 by Jason Harris
    email                : jharris@30doradus.org
    copyright            : (C) 2004-2005 by Pablo de Vicente
    email                : p.devicente@wanadoo.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SKYPOINT_H_
#define SKYPOINT_H_


#include <QList>

#include "kstars/dms.h"
#include "kstars/kstarsdatetime.h"

class KSNumbers;
class KSSun;

/**@class SkyPoint
	*
	*The sky coordinates of a point in the sky.  The
	*coordinates are stored in both Equatorial (Right Ascension,
	*Declination) and Horizontal (Azimuth, Altitude) coordinate systems.
	*Provides set/get functions for each coordinate angle, and functions
	*to convert between the Equatorial and Horizon coordinate systems.
	*
	*Because the coordinate values change slowly over time (due to
	*precession, nutation), the "catalog coordinates" are stored
	*(RA0, Dec0), which were the true coordinates on Jan 1, 2000.
	*The true coordinates (RA, Dec) at any other epoch can be found
	*from the catalog coordinates using updateCoords().
	*@short Stores dms coordinates for a point in the sky.
	*for converting between coordinate systems.
	*@author Jason Harris
	*@version 1.0
	*/
class SkyPoint {
public:
    /**Default constructor: Sets RA, Dec and RA0, Dec0 according
    	*to arguments.  Does not set Altitude or Azimuth.
    	*@param r Right Ascension
    	*@param d Declination
    	*/
    SkyPoint( const dms& r, const dms& d ) :
    lastPrecessJD( J2000 ), RA0(r), Dec0(d),
            RA(r),  Dec(d)
    {}

    
    /**Alternate constructor using double arguments, for convenience.
     *It behaves essentially like the default constructor.
     *@param r Right Ascension, expressed as a double
     *@param d Declination, expressed as a double
     *@note This also sets RA0 and Dec0
     */
    //FIXME: this (*15.0) thing is somewhat hacky.
    explicit SkyPoint( double r, double d ) :
    lastPrecessJD( J2000 ), RA0(r*15.0), Dec0(d), RA(r*15.0),  Dec(d)
    {}
    
    /**
     *@short Default constructor. Sets nonsense values for RA, Dec etc
     */
    SkyPoint();

    /** Empty destructor. */
    virtual ~SkyPoint();

    ////
    //// 1.  Setting Coordinates
    //// =======================

    /**Sets RA, Dec and RA0, Dec0 according to arguments.
     * Does not set Altitude or Azimuth.
     * @param r Right Ascension
     * @param d Declination
     * @note This function also sets RA0 and Dec0 to the same values, so call at your own peril!
     */
    void set( const dms& r, const dms& d );

    /**Sets RA0, the catalog Right Ascension.
    	*@param r catalog Right Ascension.
    	*/
    inline void setRA0( dms r ) { RA0 = r; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param r Right Ascension, expressed as a double.
    	*/
    inline void setRA0( double r ) { RA0.setH( r ); }

    /**Sets Dec0, the catalog Declination.
    	*@param d catalog Declination.
    	*/
    inline void setDec0( dms d ) { Dec0 = d; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param d Declination, expressed as a double.
    	*/
    inline void setDec0( double d ) { Dec0.setD( d ); }

    /**Sets RA, the current Right Ascension.
    	*@param r Right Ascension.
    	*/
    inline void setRA( dms r ) { RA = r; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param r Right Ascension, expressed as a double.
    	*/
    inline void setRA( double r ) { RA.setH( r ); }

    /**Sets Dec, the current Declination
    	*@param d Declination.
    	*/
    inline void setDec( dms d ) { Dec = d; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param d Declination, expressed as a double.
    	*/
    inline void setDec( double d ) { Dec.setD( d ); }

    /**Sets Alt, the Altitude.
    	*@param alt Altitude.
    	*/
    inline void setAlt( dms alt ) { Alt = alt; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param alt Altitude, expressed as a double.
    	*/
    inline void setAlt( double alt ) { Alt.setD( alt ); }

    /**Sets Az, the Azimuth.
    	*@param az Azimuth.
    	*/
    inline void setAz( dms az ) { Az = az; }

    /**Overloaded member function, provided for convenience.
    	*It behaves essentially like the above function.
    	*@param az Azimuth, expressed as a double.
    	*/
    inline void setAz( double az ) { Az.setD( az ); }

    ////
    //// 2. Returning coordinates.
    //// =========================

    /**@return a pointer to the catalog Right Ascension. */
    inline const dms& ra0() const { return RA0; }

    /**@return a pointer to the catalog Declination. */
    inline const dms& dec0() const { return Dec0; }

    /**@returns a pointer to the current Right Ascension. */
    inline const dms& ra() const { return RA; }

    /**@return a pointer to the current Declination. */
    inline const dms& dec() const { return Dec; }

    /**@return a pointer to the current Azimuth. */
    inline const dms& az() const { return Az; }

    /**@return a pointer to the current Altitude. */
    inline const dms& alt() const { return Alt; }

    ////
    //// 3. Coordinate conversions.
    //// ==========================

    /**Determine the (Altitude, Azimuth) coordinates of the
    	*SkyPoint from its (RA, Dec) coordinates, given the local
    	*sidereal time and the observer's latitude.
    	*@param LST pointer to the local sidereal time
    	*@param lat pointer to the geographic latitude
    	*/
    void EquatorialToHorizontal( const dms* LST, const dms* lat );

    /**Determine the (RA, Dec) coordinates of the
    	*SkyPoint from its (Altitude, Azimuth) coordinates, given the local
    	*sidereal time and the observer's latitude.
    	*@param LST pointer to the local sidereal time
    	*@param lat pointer to the geographic latitude
    	*/
    void HorizontalToEquatorial( const dms* LST, const dms* lat );

    /**Determine the Ecliptic coordinates of the SkyPoint, given the Julian Date.
    	*The ecliptic coordinates are returned as reference arguments (since
    	*they are not stored internally)
    	*/
    void findEcliptic( const dms *Obliquity, dms &EcLong, dms &EcLat );

    /**Set the current (RA, Dec) coordinates of the
    	*SkyPoint, given pointers to its Ecliptic (Long, Lat) coordinates, and
    	*to the current obliquity angle (the angle between the equator and ecliptic).
    	*/
    void setFromEcliptic( const dms *Obliquity, const dms& EcLong, const dms& EcLat );

    /** Computes galactic coordinates from equatorial coordinates referred to
    	* epoch 1950. RA and Dec are, therefore assumed to be B1950
    	* coordinates.
    	*/
    void Equatorial1950ToGalactic(dms &galLong, dms &galLat);

    /** Computes equatorial coordinates referred to 1950 from galactic ones referred to
    	* epoch B1950. RA and Dec are, therefore assumed to be B1950
    	* coordinates.
    	*/
    void GalacticToEquatorial1950(const dms* galLong, const dms* galLat);

    ////
    //// 4. Coordinate update/corrections.
    //// =================================

    /**Determine the current coordinates (RA, Dec) from the catalog
    	*coordinates (RA0, Dec0), accounting for both precession and nutation.
    	*@param num pointer to KSNumbers object containing current values of
    	*time-dependent variables.
    	*@param includePlanets does nothing in this implementation (see KSPlanetBase::updateCoords()).
    	*@param lat does nothing in this implementation (see KSPlanetBase::updateCoords()).
    	*@param LST does nothing in this implementation (see KSPlanetBase::updateCoords()).
        *@param forceRecompute reapplies precession, nutation and aberration even if the time passed since the last computation is not significant.
    	*/
    virtual void updateCoords( KSNumbers *num, bool includePlanets=true, const dms *lat=0, const dms *LST=0, bool forceRecompute = false );

    /** Computes the angular distance between two SkyObjects. The algorithm
     *  to compute this distance is:
     *  cos(distance) = sin(d1)*sin(d2) + cos(d1)*cos(d2)*cos(a1-a2)
     *  where a1,d1 are the coordinates of the first object and a2,d2 are
     *  the coordinates of the second object.
     *  However this algorithm is not accurate when the angular separation
     *  is small.
     *  Meeus provides a different algorithm in page 111 which we 
     *  implement here.
     *  @param sp SkyPoint to which distance is to be calculated
     *  @param positionAngle if a non-null pointer is passed, the position angle from this SkyPoint to sp is computed and stored at the location
     *  @return dms angle representing angular separation.
     **/
    dms angularDistanceTo(const SkyPoint *sp, double * const positionAngle = 0) const;

    inline bool operator == ( SkyPoint &p ) { return ( ra() == p.ra() && dec() == p.dec() ); }

    /** Find the SkyPoint obtained by moving distance dist
     * (arcseconds) away from the givenSkyPoint 
     *
     * @param dist Distance to move through in arcseconds
     * @param from The SkyPoint to move away from
     * @return a SkyPoint that is at the dist away from this SkyPoint in the direction away from from
     */
    SkyPoint moveAway( const SkyPoint &from, double dist );

    /**
     * @short Check if this point is circumpolar at the given geographic latitude
     */
    bool checkCircumpolar( const dms *gLat );

    long double   lastPrecessJD; // JD at which the last coordinate update (see updateCoords) for this SkyPoint was done

private:
    dms RA0, Dec0; //catalog coordinates
    dms RA, Dec; //current true sky coordinates
    dms Alt, Az;
};

#endif
