/*
    opssupernovae.cpp  -  K Desktop Planetarium
    -------------------
    begin                : Thu, 25 Aug 2011
    SPDX-FileCopyrightText: 2011 Samikshan Bairagya <samikshan@gmail.com>
*/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "opssupernovae.h"

#include "Options.h"
#include "kstars.h"
#include "kstarsdata.h"
#include "skymapcomposite.h"

OpsSupernovae::OpsSupernovae() : QFrame(KStars::Instance())
{
    setupUi(this);
}
