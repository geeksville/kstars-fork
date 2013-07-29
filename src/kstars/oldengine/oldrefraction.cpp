/***************************************************************************
     engine/oldrefraction.cpp - deprecated functions for atmo refraction.
                             -------------------
    begin                : 2013-06-12
    copyright            : (C) 2013 by Henry de Valence
    email                : hdevalence@hdevalence.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "oldrefraction.h"

#include "skypoint.h"
#include "ksnumbers.h"
#include "Options.h"

namespace KSEngine {
namespace OldRefraction {
    
dms altRefracted(const SkyPoint *p) {
    if( Options::useRefraction() )
        return refract(p->alt());
    else
        return p->alt();
}

double refractionCorr(const double alt)
{
    return 1.02 / tan(dms::DegToRad * ( alt + 10.3/(alt + 5.11) )) / 60;
}


double refract(const double alt) {
    static double corrCrit = refractionCorr( altCrit );

    if( alt > altCrit ) {
        return ( alt + refractionCorr(alt) );
    } else {
        // Linear extrapolation from corrCrit at altCrit to 0 at -90 degrees
        return ( alt + corrCrit * (alt + 90) / (altCrit + 90) );
    }
}

// Found uncorrected value by solving equation. This is OK since
// unrefract is never called in loops.
//
// Convergence is quite fast just a few iterations.
double unrefract(const double alt) {
    double h0 = alt;
    // It's probably okay to add h0 in refract() and subtract it 
    // here, since refract() is called way more frequently.
    double h1 = alt - (refract( h0 ) - h0);

    while( fabs(h1 - h0) > 1e-4 ) {
        h0 = h1;
        h1 = alt - (refract( h0 ) - h0);
    }
    return h1;
}

} //NS OldRefraction
} //NS KSEngine