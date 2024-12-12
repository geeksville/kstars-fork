/*
    SPDX-FileCopyrightText: 2008 Akarsh Simha <akarshsimha@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QtGlobal>

/**
 * @short  A 16-byte structure that holds star data for really faint stars.
 *
 * @author Akarsh Simha
 * @version 1.0
 */
struct DeepStarData
{
    qint32 RA { 0 };  /**< Raw signed 32-bit RA value. Needs to be multiplied by the scale (1e6) */
    qint32 Dec { 0 }; /**< Raw signed 32-bit DE value. Needs to be multiplied by the scale (1e6) */
    qint16 dRA { 0 };
    qint16 dDec { 0 };
    qint16 B { 0 };
    qint16 V { 0 };
};

// TODO: Think about how to improve this data block to include
// parallaxes without changing the struct's memory and disk footprint.
//
// RA and Dec combined to a resolution of 0.1° comprise of 46.3 bits
// of information, so there is scope to pack more by using some
// boutiqe encoding... The remaining 17 bits should be able to
// accommodate the parallax. Perhaps creating a qint16[4] data block
// that can be bitmasked to read RA, Dec and parallax is a future goal
