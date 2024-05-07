#!/usr/bin/env python3

import sys
import os
import argparse
import numpy as np
import astropy.units as u
from astropy.coordinates import SkyCoord, ICRS
from astropy.time import Time
import tqdm
import pykstars
import stardataio
import sqlite3
import io
import requests
import logging
import json
import typing
import gzip
import concurrent.futures
from functools import partial
from IPython import embed
logging.basicConfig(level=logging.INFO)

logger = logging.getLogger("Hipparcos-to-DB")

""" This script reads the data stored in the legacy namedstars.dat,
unnamedstars.dat, deepstars.dat files which was taken from the Hipparcos /
Tycho2 catalogs, converts it all into ICRS frame, cross-identifies it against
the original Tycho2 IDs and Henry Draper cross-match, and puts it all in a
database with proper IDs """

# Based on a few spot checks, I determined that whereas the Tycho2 coordinates
# in KStars (deepstars.dat) are at equinox and epoch of 2000.0, the Hipparcos
# coordinates (namedstars.dat / unnamedstars.dat) are actually referenced to an
# equinox of J2000.0, but the proper motions are referenced to the J1991.25 FK5
# frame and the proper motion correction is NOT applied. There was some
# discussion about this on the kstars-devel mailing list, where Massimiliano
# Masserelli pointed out that our positions for bright stars were J1991.25 and
# not J2000.0 -- this was corrected in the unused ASCII stars.dat, but I do not
# see an update to the binary namedstars.dat / unnamedstars.dat files
# corresponding to this change. However, the spot checks seem to show that if I
# apply reverse-precession to take the coordinates from J2000.0 to J1991.25,
# then apply the proper motion correction from epoch of J1991.25 to J2000.0,
# then apply precession from J1991.25 to J2000.0, I can recover excellent
# agreement with SIMBAD for a couple high--proper motion stars. This makes me
# believe that the original issue Massimiliano faced with star positions in
# KStars had to do with proper motion and not precession.
#
# Spot checks with deepstars.dat had some errors (false identifications of
# double stars), but some other positions seemed to correctly match J2000.0
# both in equinox and epoch.

meta = {
    'named': {'ep': 1991.25, 'eq': 2000.0},
    'unnamed': {'ep': 1991.25, 'eq': 2000.0},
    'deep': {'ep': 2000.0, 'eq': 2000.0},
}

HD_TYC_URL = 'https://vizier.cfa.harvard.edu/viz-bin/asu-tsv?-oc.form=dec&-out.max=unlimited&-c.eq=J2000&-c.r=  2&-c.u=arcmin&-c.geom=r&-source=IV/25/tyc2_hd&-order=I&-out=TYC1&-out=TYC2&-out=TYC3&-out=HD&-out=n_HD&-out=n_TYC&Simbad=Simbad&'
TYC_BASE_URL = 'https://cdsarc.cds.unistra.fr/ftp/cats/I/259/'
ATHYG_URL = 'https://www.astronexus.com/downloads/catalogs/athyg_v24.csv.gz'
TARGET_HTM_LEVEL = 6

class TABLE_NAMES:
    HD_TYC2 = 'hd_tyc2'
    TYCHO2 = 'tycho2'
    KSBIN = 'ksbin'
    KS_TYC2 = 'ks_tyc2'
    ATHYG = 'athyg'

if sqlite3.threadsafety != 3:
    raise RuntimeError(f'The version of SQLite3 used does not support multiple threads!')

def estimated_row_count(table: str, cursor: sqlite3.Cursor) -> int:
    try:
        return cursor.execute(f"SELECT COUNT() FROM {table}").fetchone()[0]
    except sqlite3.OperationalError as e: # Not completely robust but mostly should be okay
        logger.warning(f'Exception while estimating row count of table {table}: {e}')
        return 0

def download_and_cache(url: str, path: str, skip_if_exists=True):
    if os.path.isfile(path) and skip_if_exists:
        logger.info(f'File {path} already exists, skipping download.')
        return path
    logger.info(f'Downloading {url} to {path}')
    response = requests.get(url, stream=True)
    response.raise_for_status()
    with open(path, 'wb') as f:
        for chunk in response.iter_content():
            f.write(chunk)
    return path

def ingest_hd_tyc2(cache_dir: str, cursor: sqlite3.Cursor) -> str:
    """ Read the Henry Draper/Tycho2 cross match from VizieR and ingest into DB """

    TABLE_NAME = TABLE_NAMES.HD_TYC2

    cache_file = os.path.join(cache_dir, 'hd_tyc.tsv')
    with open(download_and_cache(HD_TYC_URL, cache_file), 'r') as content:
        logger.info('Parsing and indexing HD-Tycho cross match')
        HD_TYC_raw = []
        for row in content.readlines():
            row = row.rstrip('\n\t\r').strip(' ')
            if len(row) == 0 or row.startswith('#'):
                continue
            HD_TYC_raw.append(row.split('\t'))

    HD_TYC = []
    HD_TYC_conversions = {
        'TYC1': int,
        'TYC2': int,
        'TYC3': int,
        'HD': int,
        'n_HD': int,
        'n_TYC': int,
    }
    for entry in HD_TYC_raw[3:]:
        HD_TYC.append(dict(zip(HD_TYC_raw[0], entry)))
    for entry in HD_TYC:
        for key, func in HD_TYC_conversions.items():
            entry[key] = func(entry[key])
        entry["TYC"] = f'{entry["TYC1"]}-{entry["TYC2"]}-{entry["TYC3"]}'

    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": HD_TYC_URL,
                       "local_file": cache_file,
                       })))
    cursor.execute(f"CREATE TABLE IF NOT EXISTS {TABLE_NAME} ("
                   "    id INTEGER PRIMARY KEY,"
                   "    HD INTEGER NOT NULL,"           # Henry Draper number
                   "    TYC TEXT NOT NULL,"             # Combined Tycho2 designation as string
                   "    n_HD INTEGER NOT NULL,"         # Number of HD entries corresponding to TYC entry
                   "    n_TYC INTEGER NOT NULL)"        # Number of Tycho2 entries corresponding to HD entry
                   )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__HD ON {TABLE_NAME}(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__HD ON {TABLE_NAME}(TYC)")
    cursor.execute(f"DELETE FROM `{TABLE_NAME}`")
    cursor.executemany(f"INSERT INTO `{TABLE_NAME}` (HD, TYC, n_HD, n_TYC) VALUES (:HD, :TYC, :n_HD, :n_TYC)",
                       HD_TYC)

    logger.info(f'HD-Tycho2 cross-match parsed and ingested into table {TABLE_NAME}')
    return TABLE_NAME

def ingest_tycho2_chunk(path: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor, TABLE_NAME: str):
    logger.info(f'Processing Tycho2 data from {path}')
    data = []
    f = gzip.open(path, 'rb')
    ras, decs = [], []
    for line in f.readlines():
        line = line.decode('utf-8').strip('\r\n\t ').split('|')
        TYC_1, TYC_2, TYC_3 = map(int, line[0].split(' '))
        TYC = f'{TYC_1}-{TYC_2}-{TYC_3}'
        try:
            jra, jdec = map(lambda q: float(q.strip(' ')), (line[2], line[3]))
        except ValueError:
            jra, jdec = None, None
        epra, epdec = map(lambda q: float(q.strip(' ')), (line[24], line[25]))
        if jra is not None:
            assert jdec is not None
            ras.append(jra)
            decs.append(jdec)
        else:
            ras.append(epra)
            decs.append(epdec)

        data.append(
            (TYC, jra, jdec, epra, epdec))

    f.close()
    trixels = indexer.get_trixel(np.asarray(ras), np.asarray(decs), False)
    logger.info(f'Ingesting Tycho2 data from {path}')
    cursor.executemany(
        f"INSERT INTO `{TABLE_NAME}` (TYC, jra, jdec, epra, epdec, tgt_trixel) VALUES (?, ?, ?, ?, ?, ?)",
        [datum + (int(trixel),) for datum, trixel in zip(data, trixels)])

def ingest_tycho2_supplement(path: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor, TABLE_NAME: str):
    CC = pykstars.CoordinateConversion
    logger.info(f'Processing Tycho2 supplement data from {path}')
    data = []
    f = gzip.open(path, 'rb')
    ras, decs = [], []
    for line in f.readlines():
        line = line.decode('utf-8').strip('\r\n\t ').split('|')
        TYC_1, TYC_2, TYC_3 = map(int, line[0].split(' '))
        TYC = f'{TYC_1}-{TYC_2}-{TYC_3}'
        epra, epdec = map(lambda q: float(q.strip(' ')), (line[2], line[3])) # ICRS @ J1991.25
        try:
            pmra, pmdec = map(lambda q: float(q.strip(' ')), (line[4], line[5])) # PM is in the ICRF
            # Spot checked the following on one star, TYC 17-1272-1, against SIMBAD
            jra, jdec = CC.proper_motion(epra, epdec, pmra, pmdec, 1991.25, 2000.0)
        except ValueError:
            jra, jdec = None, None # No proper motion
        if jra is not None:
            assert jdec is not None
            ras.append(jra)
            decs.append(jdec)
        else:
            ras.append(epra)
            decs.append(epdec)

        data.append(
            (TYC, jra, jdec, epra, epdec))

    f.close()
    trixels = indexer.get_trixel(np.asarray(ras), np.asarray(decs), False)
    logger.info(f'Ingesting Tycho2 supplement data from {path}')
    cursor.executemany(
        f"INSERT INTO `{TABLE_NAME}` (TYC, jra, jdec, epra, epdec, tgt_trixel) VALUES (?, ?, ?, ?, ?, ?)",
        [datum + (int(trixel),) for datum, trixel in zip(data, trixels)])

def ingest_athyg(indexer: pykstars.Indexer, cache_dir: str, cursor: sqlite3.Cursor) -> str:
    """ Download the AT-HYG dataset, index it with `indexer`, and ingest it into the DB """

    TABLE_NAME = TABLE_NAMES.ATHYG
    download_and_cache(ATHYG_URL, os.path.join(cache_dir, os.path.basename(ATHYG_URL)))

def ingest_tycho2(indexer: pykstars.Indexer, cache_dir: str, cursor: sqlite3.Cursor) -> str:
    """ Download the Tycho2 dataset, index it with `indexer`, and ingest it into the DB """

    TABLE_NAME = TABLE_NAMES.TYCHO2

    paths = []
    for part in range(20):
        paths.append(
            download_and_cache(TYC_BASE_URL + f'tyc2.dat.{part:02}.gz', os.path.join(cache_dir, f'tyc2.dat.{part:02}.gz'))
        )

    cursor.execute(f"CREATE TABLE IF NOT EXISTS `{TABLE_NAME}` ("
                   "    id INTEGER PRIMARY KEY,"        # Primary key integer
                   "    TYC TEXT NOT NULL, "            # Tycho2 3-part identifier
                   "    jra REAL,"                      # ICRS RA converted to J2000.0 in degrees
                   "    jdec REAL,"                     # ICRS Dec converted to J2000.0 in degrees
                   "    epra REAL NOT NULL, "           # ICRS RA at observation epoch 
                   "    epdec REAL NOT NULL, "          # ICRS RA at observation epoch
                   "    tgt_trixel INTEGER NOT NULL)"   # Trixel number (calculated using J2000.0 RA/Dec if available, else Epoch RA/Dec)
                   )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__TYC ON {TABLE_NAME}(TYC)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__tgt_trixel ON {TABLE_NAME}(tgt_trixel)")
    cursor.execute(f"DELETE FROM `{TABLE_NAME}`")

    ingester = partial(ingest_tycho2_chunk, indexer=indexer, TABLE_NAME=TABLE_NAME, cursor=cursor)
    _ = list(tqdm.tqdm(map(ingester, paths), total=len(paths)))

    # Ingest the Tycho2 Supplement 1 (Tycho1 / Hipparcos stars)
    supplement_path = download_and_cache(TYC_BASE_URL + f'suppl_1.dat.gz', os.path.join(cache_dir, f'suppl_1.dat.gz'))
    ingest_tycho2_supplement(supplement_path, indexer, cursor, TABLE_NAME)
    

    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": TYC_BASE_URL,
                       "local_dir": cache_dir,
                       "local_paths": paths,
                       "htm_level": indexer.level,
                       })))
    return TABLE_NAME

def ingest_kstars_binary_files(indexer: pykstars.Indexer, meta: dict, cursor: sqlite3.Cursor) -> str:
    TABLE_NAME = TABLE_NAMES.KSBIN
    cursor.execute(f"CREATE TABLE IF NOT EXISTS `{TABLE_NAME}` ("
                   "    id INTEGER PRIMARY KEY,"        # Primary key integer
                   "    ra REAL NOT NULL,"              # ICRS RA in degrees
                   "    dec REAL NOT NULL,"             # ICRS Dec in degrees
                   "    file TEXT NOT NULL,"            # Filename (deep / named / unnamed)
                   "    trixel INTEGER NOT NULL,"       # Trixel number in source binary file
                   "    tr_index INTEGER NOT NULL,"     # Index of star record within trixel
                   "    offset INTEGER NOT NULL,"       # Offset within binary file
                   "    HD INTEGER,"                    # Primary Henry Draper identification (can be null)
                   "    tgt_trixel INTEGER NOT NULL)"   # Trixel ID in target HTMesh
                   )
    # cursor.execute("CREATE INDEX IF NOT EXISTS idx_hd ON HenryDraper(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_file ON {TABLE_NAME}(file)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_offset ON {TABLE_NAME}(offset)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_hd_{TABLE_NAME} ON {TABLE_NAME}(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_tgt_trixel ON {TABLE_NAME}(tgt_trixel)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_trixel ON {TABLE_NAME}(trixel)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_tr_index ON {TABLE_NAME}(tr_index)")

    CC = pykstars.CoordinateConversion
    ##
    # Correction to apply for Hipparcos (namedstars.dat / unnamedstars.dat):
    #
    # r, d = CC.precess(s['RA'] * 15.0, s['Dec'], 2000.0, 1991.25)
    # r, d = CC.proper_motion(r, d, s['dRA'], s['dDec'], 1991.25, 2000.0)
    # r, d = CC.precess(r, d, 1991.25, 2000.0);
    #
    ##


    # cursor.execute("DELETE FROM HenryDraper") # Truncate the table
    # cursor.execute("DELETE FROM metadata")
    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "files": meta,
                       "target_htm_level": TARGET_HTM_LEVEL})))
    cursor.execute(f"DELETE FROM {TABLE_NAME}")

    for key, info in meta.items():
        logger.info(f'Processing {key} stars from {info["path"]}')
        reader = stardataio.KSStarDataReader(info['path'])
        ep, eq = info['ep'], info['eq']
        for trixel in tqdm.tqdm(reader, total=len(reader)):
            for tr_index, star in enumerate(trixel):
                r, d = star['RA'] * 15.0, star['Dec']   # degrees
                pmra, pmdec = star['dRA'], star['dDec'] # mas/yr
                HD = int(star['HD']*10.0001)
                if HD == 0:
                    HD = None

                # Convert epoch / equinox to ICRS
                if ep != eq:
                    r, d = CC.precess(r, d, eq, ep)
                if ep != 2000.0:
                    r, d = CC.proper_motion(r, d, pmra, pmdec, ep, 2000.0)
                    r, d = CC.precess(r, d, ep, 2000.0)

                # Index
                tgt_trixel = indexer.get_trixel(r, d)

                # # Cross-match with Henry Draper
                # candidates = []
                # HDs = []
                # match_dist = None
                # for tr in indexer.get_trixels(r, d, HD_XMATCH_RADIUS * 1.20): # 20% buffer factor
                #     for hd_entry in HenryDraper.get(tr, []):
                #         hdra, hddec = hd_entry['_RAJ2000'], hd_entry['_DEJ2000']
                #         dist = CC.angular_distance(r, d, hdra, hddec)
                #         if dist <= HD_XMATCH_RADIUS:
                #             candidates.append((dist, hd_entry))
                # if len(candidates) > 0:
                #     candidates = sorted(candidates, key=lambda t: (t[0], t[1]['HD']))
                #     N_HD = candidates[0][1]['n_HD']
                #     HDs = [entry[1]['HD'] for entry in candidates[:N_HD]]
                #     match_dist = candidates[0][0]
                # for HD in HDs:
                #     N_HD_Matched += 1
                #     cursor.execute("INSERT INTO HenryDraper (HD, file, trixel, tr_index, offset, match_dist) VALUES (?, ?, ?, ?, ?, ?)",
                #                    (HD, key, trixel.id(), tr_index, star.offset, match_dist))

                cursor.execute(
                    f"INSERT INTO {TABLE_NAME} (ra, dec, file, trixel, tr_index, offset, HD, tgt_trixel) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                    (r, d, key, trixel.id(), tr_index, star.offset, HD, tgt_trixel)
                )

    return TABLE_NAME

def generate_xmatch(indexer: pykstars.Indexer, radius: float, cursor: sqlite3.Cursor, src_htm_level = 3) -> str:
    """ Cross match KStars star catalogs gainst Tycho2 using (RA, Dec)
    radius: in arcseconds
    """
    N_trixel = (4 ** indexer.level) * 8

    src_indexer = pykstars.Indexer(src_htm_level)
    CC = pykstars.CoordinateConversion

    TABLE_NAME = TABLE_NAMES.KS_TYC2
    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{TABLE_NAME}` ("
        "    id INTEGER PRIMARY KEY,"        # Primary key integer
        "    ksbin_id INTEGER NOT NULL,"
        "    tycho2_id INTEGER NOT NULL,"
        "    match_dist REAL NOT NULL)"
    )
    # cursor.execute("CREATE INDEX IF NOT EXISTS idx_hd ON HenryDraper(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__ksbin ON {TABLE_NAME}(ksbin_id)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__tycho2 ON {TABLE_NAME}(tycho2_id)")
    cursor.execute(f"DELETE FROM {TABLE_NAME}")
    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `pm_duplicates` ("
        "    dup_id INTEGER PRIMARY KEY,"
        "    orig_id INTEGER NOT NULL)"
    )
    cursor.execute(f"DELETE FROM pm_duplicates")

    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `ksbin_unmatched` ("
        "    ksbin_id INTEGER PRIMARY KEY,"
        "    tycho2_closest_id INTEGER,"
        "    distance_arcsec REAL)"
    )
    cursor.execute(f"DELETE FROM ksbin_unmatched")

    total_unmatched = 0
    total_pm_duplicates = 0
    for trixel in tqdm.tqdm(range(N_trixel)):
        ksbin_rows = cursor.execute(
            f"SELECT `id`, `trixel`, `ra`, `dec` FROM {TABLE_NAMES.KSBIN} WHERE `tgt_trixel` = {trixel}").fetchall()
        query_trixels = set()
        ksbin = []
        for i, src_trixel, ra, dec in ksbin_rows:
            expected_src_trixel = src_indexer.get_trixel(ra, dec)
            if src_trixel != expected_src_trixel:
                found = False
                info_dump = []
                for dup_id, dup_ra, dup_dec in cursor.execute(f"SELECT `id`, `ra`, `dec` FROM {TABLE_NAMES.KSBIN} WHERE `trixel` = {expected_src_trixel}"):
                    if abs(ra - dup_ra) < 1e-7 and abs(dec - dup_dec) < 1e-7:
                        cursor.execute("INSERT INTO pm_duplicates (dup_id, orig_id) VALUES (?, ?)",
                                       (i, dup_id))
                        # logger.info(f'Found original id={dup_id} for proper motion duplicate {i}')
                        found = True
                        break
                if found:
                    total_pm_duplicates += 1
                    continue # Skip, PM duplicate

                logger.info(f'Star in {TABLE_NAMES.KSBIN} with id = {i} was expected to have source trixel {expected_src_trixel} but has {src_trixel} instead. So it must be a proper motion duplicate, but mining for the original did not produce any candidates!')

                if expected_src_trixel in set(src_indexer.get_trixels(ra, dec, 1e-4)):
                    # Not a duplicate, not an error, don't skip
                    logger.info(f'Fuzzy match with radius of 1e-4 does return {expected_src_trixel}, so should be a computation error')
                else:
                    embed(header=f'Failed to even fuzzy match the source trixel!')
                        
            for t in indexer.get_trixels(ra, dec, radius/3600.0):
                query_trixels.add(t)
            ksbin.append((i, src_trixel, ra, dec))

        cursor.execute(
            f"SELECT `id`, `tgt_trixel`, COALESCE(`jra`, `epra`), COALESCE(`jdec`, `epdec`) FROM {TABLE_NAMES.TYCHO2} WHERE `tgt_trixel` IN ("
            + ", ".join(map(str, query_trixels)) + ")")
        tycho2 = cursor.fetchall()

        # Calculate pair-wise angular distance
        ksbin_radec = np.asarray(ksbin)[:, 2:]
        tycho2_radec = np.asarray(tycho2)[:, 2:]
        
        radec_pairs = np.concatenate([ # This incantation produces (ra1, dec1, ra2, dec2)
            np.broadcast_to(ksbin_radec, (tycho2_radec.shape[0], ksbin_radec.shape[0], 2)).transpose(2, 1, 0),
            np.broadcast_to(tycho2_radec, (ksbin_radec.shape[0], tycho2_radec.shape[0], 2)).transpose(2, 0, 1)]
        ).transpose(1, 2, 0).reshape(-1, 4) # The first index is a flattened combination of (ksbin_i, tycho2_j)

        distances = CC.angular_distance(
            radec_pairs[:, 0],
            radec_pairs[:, 1],
            radec_pairs[:, 2],
            radec_pairs[:, 3],
        ).reshape(len(ksbin_radec), len(tycho2_radec))

        argmin = distances.argmin(axis=1) # Tycho2 indices with minimum distance to KSBin indices
        unmatched, = np.where(distances.min(axis=1) * 3600.0 > radius)
        matched, = np.where(distances.min(axis=1) * 3600.0 <= radius)
        total_unmatched += len(unmatched)
        if len(unmatched) > 0:
            unmatched_ids = [ksbin[j][0] for j in unmatched]
            # logger.warning(f'Failed to find Tycho2 matches for {len(unmatched)} stars within {radius}" tolerance, with ids=[{", ".join(map(str, unmatched_ids))}] in the {TABLE_NAMES.KSBIN} table.')
            for j in unmatched:
                _, _, umra, umdec = ksbin[j]
                xmatch_err = distances[j, argmin[j]] * 3600.0
                cursor.execute("INSERT INTO `ksbin_unmatched` (ksbin_id, tycho2_closest_id, distance_arcsec) VALUES (?, ?, ?)",
                               (ksbin[j][0], tycho2[argmin[j]][0], xmatch_err))

        cursor.executemany(
            f"INSERT INTO {TABLE_NAME} (ksbin_id, tycho2_id, match_dist) VALUES (?, ?, ?)",
            [(ksbin[match][0], tycho2[argmin[match]][0], distances[match, argmin[match]]) for match in matched])

        # if distances.min(axis=1).max() * 3600.0 > 2.0: # FIXME:
        #     ksbin_idx = distances.min(axis=1).argmax()
        #     tycho2_idx = distances[ksbin_idx].argmin()
        #     embed(header=f'Large match distance of {distances.min(axis=1).max() * 3600.0:0.3f}" at ({ksbin_idx},{tycho2_idx}) in the distance matrix. The database ids are ksbin={ksbin[ksbin_idx][0]}, tycho2={tycho2[tycho2_idx][0]}. Inspect!')

    logger.info(f'Found {total_pm_duplicates} proper-motion duplicates')
    logger.warning(f'Found {total_unmatched} entries in KStars binary files that could not be matched to Tycho2!')

    return TABLE_NAME
        

def main(argv):
    parser = argparse.ArgumentParser(description='Uniformize Hipparcos/Tycho2 binary data into SQLite DB')
    parser.add_argument(
        '--cache-dir', '-d',
        type=str,
        required=True,
        help='Specify a directory where the Tycho2 and Henry Draper will be downloaded. It should have maybe 1GB of free space.'
    )
    parser.add_argument(
        '--output', '-o',
        type=str,
        required=False,
        default=None,
        help='Path to SQLite database. Will use stars.db in the --cache-dir by default.',
    )
    parser.add_argument(
        '--namedstars', '-n',
        type=str,
        required=True,
        help='Path to namedstars.dat file',
    )
    parser.add_argument(
        '--unnamedstars', '-u',
        type=str,
        required=True,
        help='Path to unnamedstars.dat file',
    )
    parser.add_argument(
        '--deepstars', '-D',
        type=str,
        required=True,
        help='Path to deepstars.dat file',
    )
    parser.add_argument(
        '--force-hd-tyc2', '-fhd',
        action='store_true',
        help='Force ingestion of HD-Tycho2 even if it seems like it has been performed.'
    )
    parser.add_argument(
        '--force-tyc2', '-ft',
        action='store_true',
        help='Force ingestion of Tycho2 even if it seems like it has been performed.'
    )
    parser.add_argument(
        '--force-ks', '-fk',
        action='store_true',
        help='Force ingestion of KStars binary files even if it seems like it has been performed.'
    )
    parser.add_argument(
        '--force-xm', '-fx',
        action='store_true',
        help='Force recomputation of cross-match between KStars binary files and Tycho2 even if it seems like it has been performed.'
    )
    parser.add_argument(
        '--xmatch-radius', '-r',
        type=float,
        required=False,
        default=8.0, # Start seeing false positives of double stars at > 8", so relaxed to 8". Histogram suggests < 8.57"
        help='Maximum distance in arcseconds to consider for cross-matching between KStars binary files and Tycho2.'
    )
    args = parser.parse_args(sys.argv[1:])
    if not os.path.isdir(args.cache_dir):
        os.makedirs(args.cache_dir)
    if args.output is None:
        args.output = os.path.join(args.cache_dir, 'stars.db')

    indexer = pykstars.Indexer(TARGET_HTM_LEVEL)

    for key, path in [('named', args.namedstars), ('unnamed', args.unnamedstars), ('deep', args.deepstars)]:
        meta[key]['path'] = path

    connection = sqlite3.connect(args.output)
    cursor = connection.cursor()
    cursor.execute("CREATE TABLE IF NOT EXISTS metadata ("
                   "    table_name TEXT PRIMARY KEY,"   # Table name
                   "    info TEXT)")                    # JSON metadata

    if args.force_hd_tyc2 or estimated_row_count(TABLE_NAMES.HD_TYC2, cursor) < 1:
        logger.info('Ingested HD-Tycho2 cross-match into ' + ingest_hd_tyc2(args.cache_dir, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of HD-Tycho2 cross-match because it seems to be done already.')

    if args.force_tyc2 or estimated_row_count(TABLE_NAMES.TYCHO2, cursor) < 1:
        logger.info('Indexed and ingested Tycho2 into ' + ingest_tycho2(indexer, args.cache_dir, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of Tycho2 data because it seems to be done already.')

    if args.force_ks or estimated_row_count(TABLE_NAMES.KSBIN, cursor) < 1:
        logger.info('Re-indexed and ingested KStars binary files into ' + ingest_kstars_binary_files(indexer, meta, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of KStars binary files because it seems to be done already.')

    if args.force_xm or estimated_row_count(TABLE_NAMES.KS_TYC2, cursor) < 1:
        logger.info('Generate KStars-Tycho2 cross-match into ' + generate_xmatch(indexer, args.xmatch_radius, cursor))
        connection.commit()
    else:
        logger.warning('Skipping generation of KStars-Tycho2 cross-match because it seems to be done already.')

    connection.commit()
    connection.close()

    logger.info("Done")


if __name__ == "__main__":
    main(sys.argv)
