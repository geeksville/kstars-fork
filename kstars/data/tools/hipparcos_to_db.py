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
from typing import IO, Union, Dict, Tuple, Callable, Any
import gzip
import concurrent.futures
from functools import partial
from IPython import embed
import collections
logging.basicConfig(level=logging.INFO)
CC = pykstars.CoordinateConversion

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
HIP_URL = 'https://cdsarc.cds.unistra.fr/ftp/I/239/hip_main.dat'
TYC1_URL = 'https://cdsarc.cds.unistra.fr/ftp/I/239/tyc_main.dat'
TARGET_HTM_LEVEL = 6
SOURCE_HTM_LEVEL = 3


CrossMatchParams = collections.namedtuple('CrossMatchParams', [
    'mag_tolerance',         # Acceptable error in magnitude while matching
    'free_radius',           # Radius in arcsec for free-matching
    'mag_constrained_radius' # Radius in arcsec for magnitude-checked matching
])

CROSS_MATCH_PARAMS = {
    'tycho2': CrossMatchParams(mag_tolerance=0.3, free_radius=0.5, mag_constrained_radius=8.5),
    'athyg': CrossMatchParams(mag_tolerance=0.3, free_radius=0.5, mag_constrained_radius=8.5),
    'gaia': CrossMatchParams(mag_tolerance=0.4, free_radius=0.5, mag_constrained_radius=7),
}
SEARCH_RADIUS = 100.0 # Maximum search radius, must be larger than any above, in arcseconds

class TABLE_NAMES:
    HD_TYC2 = 'hd_tyc2'
    TYCHO2 = 'tycho2'
    KSBIN = 'ksbin'
    KSBIN_NODUPS = 'ksbin_nodups'
    KS_TYC2 = 'ks_tyc2'
    KS_TYC1 = 'ks_tyc1'
    KS_HIP = 'ks_hip'
    ATHYG = 'athyg'
    PM_DUPLICATES = 'pm_duplicates'
    STARNAMES = 'starnames'
    KS_ATHYG = 'ks_athyg'
    HIPPARCOS = 'hipparcos'
    TYCHO1 = 'tycho1'

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

def xsv_parser(f: IO, table_name: str, conversions: Dict[str, Tuple[int, Callable[[str], Any]]],
               cursor: sqlite3.Cursor, separator=',',
               post_processor=Callable[[Dict[str,Any]], None],
               skip_lines=0, comment='#', flush_interval=10000,
               ):
    """ Convenience function to parse CSV / PSV / TSV (XSV) files

    f: IO object that can be read to either produce `bytes` or `str`
    table_name: Table to write data to
    conversions: A dictionary mapping db table column-names to a tuple (index in XSV file, conversion function to apply)
    cursor: SQLite3 cursor
    separator: Field separator used in the file (e.g. ',' '|' '\t' etc.)
    post_processor: A function that gets the final row dictionary and may add / remove columns as needed for the database row
    skip_lines: Skip these many lines in the beginning
    comment: Comment line syntax (None for none)
    flush_interval: Write to DB after parsing these many rows
    """
    
    data = []
    skip = skip_lines

    assert len(conversions) > 0, 'noop'
    SQL = None


    rows = 0
    for linenum, line in enumerate(tqdm.tqdm(f.readlines())):
        if isinstance(line, bytes):
            line = line.decode('utf-8')
        line = line.strip('\r\n')
        if separator != '\t':
            line = line.strip('\t ')
        else:
            line = line.strip(' ')
        if len(line) == 0:
            continue
        if comment is not None and line.startswith(comment):
            continue
        if skip > 0:
            skip -= 1
            continue
        fields = line.split(separator)
        datum = {}
        for field, metadata in conversions.items():
            index, converter = metadata
            value = fields[index].strip(' \t')
            if len(value) == 0:
                datum[field] = None
            else:
                try:
                    datum[field] = converter(value)
                except Exception as e:
                    raise ValueError(f'Error converting value {value} for field {field} on line {linenum}:\n{e}')
        if post_processor:
            post_processor(datum)
        if SQL is None:
            SQL = (
                f"INSERT INTO `{table_name}` (" + " ,".join(datum.keys()) + ") VALUES (:"
                + ", :".join(datum.keys()) + ")"
            )
        data.append(datum)
        if linenum % flush_interval == 0:
            cursor.executemany(SQL, data)
            rows += len(data)
            data = []
    cursor.executemany(SQL, data)
    rows += len(data)

    return rows


def ingest_hd_tyc2(cache_dir: str, cursor: sqlite3.Cursor) -> str:
    """ Read the Henry Draper/Tycho2 cross match from VizieR and ingest into DB """

    TABLE_NAME = TABLE_NAMES.HD_TYC2

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

    cache_file = os.path.join(cache_dir, 'hd_tyc.tsv')
    HD_TYC_conversions = {
        'TYC1': (0, int),
        'TYC2': (1, int),
        'TYC3': (2, int),
        'HD': (3, int),
        'n_HD': (4, int),
        'n_TYC': (5, int),
    }
    def post_proc(entry):
        TYC1 = entry.pop("TYC1")
        TYC2 = entry.pop("TYC2")
        TYC3 = entry.pop("TYC3")
        entry["TYC"] = f'{TYC1}-{TYC2}-{TYC3}'
        
    with open(download_and_cache(HD_TYC_URL, cache_file), 'r') as content:
        logger.info('Parsing and indexing HD-Tycho cross match')
        xsv_parser(
            content, TABLE_NAME, HD_TYC_conversions,
            cursor, separator='\t',
            skip_lines=3, comment='#',
            post_processor=post_proc,
        )

    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": HD_TYC_URL,
                       "local_file": cache_file,
                       })))

    logger.info(f'HD-Tycho2 cross-match parsed and ingested into table {TABLE_NAME}')
    return TABLE_NAME

def hip_tyc1_post_proc(indexer, entry):
    epra, epdec = entry['epra'], entry['epdec']
    sra = entry.pop('sra')
    sdec = entry.pop('sdec')
    if epra is None or epdec is None:
        # Weirdly, sometimes the sexagesimal RA/Dec is given but not decimal
        epra = sum(x/(60**i) for i, x in enumerate(map(float, sra.split(' ')))) * 15.
        mult = 1
        if sdec.startswith('-'):
            sdec = sdec[1:]
            mult = -1
        epdec = sum(x/(60**i) for i, x in enumerate(map(float, sdec.split(' ')))) * mult
        entry['epra'] = epra
        entry['epdec'] = epdec
    pmra, pmdec = entry['pmra'], entry['pmdec']
    if pmra is not None and pmdec is not None:
        jra, jdec = CC.proper_motion(epra, epdec, pmra, pmdec, 1991.25, 2000.0)
        entry['jra'], entry['jdec'] = jra, jdec
    else:
        jra, jdec = epra, epdec # Use Epoch RA/Dec for indexing
        entry['jra'] = None
        entry['jdec'] = None
    entry['tgt_trixel'] = indexer.get_trixel(jra, jdec)


def ingest_hipparcos(cache_dir: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor):
    TABLE_NAME = TABLE_NAMES.HIPPARCOS

    cursor.execute(f"CREATE TABLE IF NOT EXISTS {TABLE_NAME} ("
                   "    HIP INTEGER PRIMARY KEY,"
                   "    epra REAL NOT NULL,"
                   "    epdec REAL NOT NULL,"
                   "    V REAL,"
                   "    parallax REAL,"
                   "    pmra REAL,"
                   "    pmdec REAL,"
                   "    bv_index REAL,"
                   "    HD INTEGER,"
                   "    BD TEXT,"
                   "    sp_type TEXT,"
                   "    jra REAL,"
                   "    jdec REAL,"
                   "    tgt_trixel INTEGER NOT NULL)"
                   )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__trixel ON {TABLE_NAME}(tgt_trixel)")
    cursor.execute(f"DELETE FROM `{TABLE_NAME}`")

    cache_file = os.path.join(cache_dir, 'hip_main.dat')
    conversions = {
        'HIP': (1, int),
        'sra': (3, str),
        'sdec': (4, str),
        'epra': (8, float),   # Frame is ICRS
        'epdec': (9, float),  # Frame is ICRS
        'V': (5, float),
        'parallax': (11, float),
        'pmra': (12, float),  # Frame is ICRS
        'pmdec': (13, float), # Frame is ICRS
        'bv_index': (37, float),
        'HD': (71, int),
        'BD': (72, str),
        'sp_type': (76, str),
    }

    with open(download_and_cache(HIP_URL, cache_file), 'r') as content:
        logger.info('Parsing and indexing Hipparcos catalog')
        xsv_parser(
            content, TABLE_NAME, conversions,
            cursor, separator='|',
            post_processor=partial(hip_tyc1_post_proc, indexer),
        )

    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": HIP_URL,
                       "local_file": cache_file,
                       })))

    logger.info(f'Hipparcos parsed and ingested into table {TABLE_NAME}')

    return TABLE_NAME

def ingest_tycho1(cache_dir: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor):
    TABLE_NAME = TABLE_NAMES.TYCHO1

    cursor.execute(f"CREATE TABLE IF NOT EXISTS {TABLE_NAME} ("
                   "    TYC TEXT PRIMARY KEY,"
                   "    epra REAL NOT NULL,"
                   "    epdec REAL NOT NULL,"
                   "    pmra REAL,"
                   "    pmdec REAL,"
                   "    V REAL,"
                   "    HIP INTEGER,"
                   "    BT REAL,"
                   "    VT REAL,"
                   "    bv_index REAL,"
                   "    jra REAL,"
                   "    jdec REAL,"
                   "    tgt_trixel INTEGER NOT NULL)"
    )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__trixel ON {TABLE_NAME}(tgt_trixel)")
    cursor.execute(f"DELETE FROM `{TABLE_NAME}`")

    cache_file = os.path.join(cache_dir, 'tyc_main.dat')
    conversions = {
        'TYC': (1, lambda q: '-'.join(str(int(num)) for num in q.strip(' ').split(' ') if len(num) != 0)),
        'sra': (3, str),
        'sdec': (4, str),
        'epra': (8, float),
        'epdec': (9, float),
        'pmra': (12, float),
        'pmdec': (13, float),
        'V': (5, float),
        'HIP': (31, int),
        'BT': (32, float),
        'VT': (34, float),
        'bv_index': (37, float),
    }

    with open(download_and_cache(TYC1_URL, cache_file), 'r') as content:
        logger.info('Parsing and indexing Tycho1 catalog')
        xsv_parser(
            content, TABLE_NAME, conversions,
            cursor, separator='|',
            post_processor=partial(hip_tyc1_post_proc, indexer),
        )

    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": TYC1_URL,
                       "local_file": cache_file,
                       })))

    logger.info(f'Tycho1 parsed and ingested into table {TABLE_NAME}')

    return TABLE_NAME


def ingest_tycho2_chunk(path: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor, TABLE_NAME: str):
    logger.info(f'Processing Tycho2 data from {path}')
    data = []
    f = gzip.open(path, 'rb')
    ras, decs = [], []
    floatify = lambda q: float(q.strip(' ')) if len(q.strip(' ')) > 0 else None
    for line in f.readlines():
        line = line.decode('utf-8').strip('\r\n\t ').split('|')
        TYC_1, TYC_2, TYC_3 = map(int, line[0].split(' '))
        TYC = f'{TYC_1}-{TYC_2}-{TYC_3}'
        BT, VT = map(floatify, (line[17], line[19]))
        jra, jdec = map(floatify, (line[2], line[3]))
        epra, epdec = map(floatify, (line[24], line[25]))
        assert epra is not None
        assert epdec is not None
        if jra is not None:
            assert jdec is not None
            ras.append(jra)
            decs.append(jdec)
        else:
            ras.append(epra)
            decs.append(epdec)

        data.append(
            (TYC, jra, jdec, epra, epdec, BT, VT))

    f.close()
    trixels = indexer.get_trixel(np.asarray(ras), np.asarray(decs), False)
    logger.info(f'Ingesting Tycho2 data from {path}')
    cursor.executemany(
        f"INSERT INTO `{TABLE_NAME}` (TYC, jra, jdec, epra, epdec, BT, VT, tgt_trixel) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        [datum + (int(trixel),) for datum, trixel in zip(data, trixels)])

def ingest_tycho2_supplement(path: str, indexer: pykstars.Indexer, cursor: sqlite3.Cursor, TABLE_NAME: str):
    logger.info(f'Processing Tycho2 supplement data from {path}')
    data = []
    f = gzip.open(path, 'rb')
    ras, decs = [], []
    floatify = lambda q: float(q.strip(' ')) if len(q.strip(' ')) > 0 else None
    for line in f.readlines():
        line = line.decode('utf-8').strip('\r\n\t ').split('|')
        TYC_1, TYC_2, TYC_3 = map(int, line[0].split(' '))
        TYC = f'{TYC_1}-{TYC_2}-{TYC_3}'
        epra, epdec = map(floatify, (line[2], line[3])) # ICRS @ J1991.25
        pmra, pmdec = map(floatify, (line[4], line[5])) # PM is in the ICRF
        BT, VT_or_Hp = map(floatify, (line[11], line[13])) # We will assume Hp is a V-like mag
        if pmra is None or pmdec is None:
            assert pmra is None and pmdec is None, 'Eh?'
            jra, jdec = None, None
        else:
            # Spot checked the following on one star, TYC 17-1272-1, against SIMBAD
            jra, jdec = CC.proper_motion(epra, epdec, pmra, pmdec, 1991.25, 2000.0)

        if jra is not None:
            assert jdec is not None
            ras.append(jra)
            decs.append(jdec)
        else:
            ras.append(epra)
            decs.append(epdec)

        data.append(
            (TYC, jra, jdec, epra, epdec, BT, VT_or_Hp))

    f.close()
    trixels = indexer.get_trixel(np.asarray(ras), np.asarray(decs), False)
    logger.info(f'Ingesting Tycho2 supplement data from {path}')
    cursor.executemany(
        f"INSERT INTO `{TABLE_NAME}` (TYC, jra, jdec, epra, epdec, BT, VT, tgt_trixel) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        [datum + (int(trixel),) for datum, trixel in zip(data, trixels)])

def ingest_athyg(indexer: pykstars.Indexer, cache_dir: str, cursor: sqlite3.Cursor) -> str:
    """ Download the AT-HYG dataset, index it with `indexer`, and ingest it into the DB """

    TABLE_NAME = TABLE_NAMES.ATHYG
    path = download_and_cache(ATHYG_URL, os.path.join(cache_dir, os.path.basename(ATHYG_URL)))

    cursor.execute(f"CREATE TABLE IF NOT EXISTS `{TABLE_NAME}` ("
                   "    id INTEGER PRIMARY KEY,"        # Primary key integer
                   "    TYC TEXT, "                     # Tycho2 3-part identifier
                   "    gaia INTEGER,"                  # Gaia identifier (DR not certain)
                   "    HD INTEGER,"                    # Henry-Draper identifier
                   "    bayer TEXT,"                    # Bayer designation
                   "    flam INT,"                      # Flamsteed designation
                   "    const TEXT,"                    # Constellation (rho Aql marked in Aql so this can be used for designation)
                   "    proper TEXT,"                   # Proper name if the star has one
                   "    parallax  REAL,"                # mas (1000/Distance (pc))
                   "    dist_src TEXT,"                 # Distance source catalog
                   "    mag   REAL,"                    # V or V_Tycho magnitude
                   "    ci    REAL,"                    # B - V color index (or BT - VT for Tycho2)
                   "    mag_src TEXT,"                  # Photometry source catalog
                   "    spect TEXT,"                    # Spectral type when known
                   "    spect_src TEXT,"                # Spectral type source dataset
                   "    ra    REAL, "                   # α in °
                   "    dec   REAL, "                   # δ in °
                   "    pos_src TEXT,"                  # Position source dataset
                   "    pmra  REAL, "                   # µ_α cos(δ) in mas/yr
                   "    pmdec REAL, "                   # µ_δ in mas/yr
                   "    pm_src TEXT,"                   # Proper motion source dataset
                   "    tgt_trixel INTEGER NOT NULL)"   # Trixel number (calculated using provided ICRS/J2000 RA/Dec)
                   )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__TYC ON {TABLE_NAME}(TYC)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__gaia ON {TABLE_NAME}(gaia)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__tgt_trixel ON {TABLE_NAME}(tgt_trixel)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__HD ON {TABLE_NAME}(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAME}__mag ON {TABLE_NAME}(mag)")
    cursor.execute(f"DELETE FROM `{TABLE_NAME}`")

    conversions = {
        'id': (0, int),
        'tyc': (1, str),
        'gaia': (2, int),
        # Skip hyg, hip
        'hd': (5, int),
        # Skip hr, gl
        'bayer': (8, str),
        'flam': (9, int),
        'const': (10, str),
        'proper': (11, str),
        'ra': (12, lambda q: float(q) * 15),
        'dec': (13, float),
        'pos_src': (14, str),
        'parallax': (15, lambda q: (1000/float(q)) if q != '' else None),
        # Skip x0, y0, z0
        'dist_src': (19, str),
        'mag': (20, float),
        # Skip absmag
        'ci': (22, float),
        'mag_src': (23, str),
        # Skip rv, rv_src
        'pmra': (26, float),
        'pmdec': (27, float),
        'pm_src': (28, str),
        # Skip vx, vy, vz
        'spect': (32, str),
        'spect_src': (33, str),
    }

    def post_proc(entry: dict):
        entry['tgt_trixel'] = indexer.get_trixel(entry['ra'], entry['dec'])

    f = gzip.open(path, 'rb')
    xsv_parser(
        f, TABLE_NAME, conversions,
        cursor, separator=',',
        skip_lines=2, # Ignore header and line corresponding to Sun
        post_processor=post_proc
    )
    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (TABLE_NAME, json.dumps({
                       "url": TYC1_URL,
                       "local_file": path,
                       })))

    f.close()
    logger.info(f'AT-HYG parsed and ingested into table {TABLE_NAME}')

    return TABLE_NAME

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
                   "    BT REAL,"                       # B magnitude
                   "    VT REAL,"                       # V magnitude
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

def ingest_kstars_binary_files(indexer: pykstars.Indexer, meta: dict, starnames_path: str, cursor: sqlite3.Cursor) -> str:
    for key, value in meta.items():
        if value['path'] is None:
            raise ValueError(f'Have not supplied path for {key} stars binary file')
    if starnames_path is None:
        raise ValueError(f'Ingesting KStars binary data, but starnames.dat path is not provided!')
    KSBIN_TABLE = TABLE_NAMES.KSBIN
    STARNAMES_TABLE = TABLE_NAMES.STARNAMES
    cursor.execute(f"CREATE TABLE IF NOT EXISTS `{KSBIN_TABLE}` ("
                   "    id INTEGER PRIMARY KEY,"        # Primary key integer
                   "    ra REAL NOT NULL,"              # ICRS RA in degrees, either J1991.25 or J2000.0 :-(
                   "    dec REAL NOT NULL,"             # ICRS Dec in degrees, either J1991.25 or J2000.0 :-(
                   "    file TEXT NOT NULL,"            # Filename (deep / named / unnamed)
                   "    trixel INTEGER NOT NULL,"       # Trixel number in source binary file
                   "    tr_index INTEGER NOT NULL,"     # Index of star record within trixel
                   "    offset INTEGER NOT NULL,"       # Offset within binary file
                   "    HD INTEGER,"                    # Primary Henry Draper identification (can be null)
                   "    B REAL,"                        # B magnitude
                   "    V REAL,"                        # V magnitude
                   # "    epra REAL,"                     # RA at the J1991.25 epoch if applicable
                   # "    epdec REAL,"                    # Dec at the J1991.25 epoch if applicable
                   "    pmra REAL,"                     # Proper motion in RA (which frame?)
                   "    pmdec REAL,"                    # Proper motion in Dec (which frame?)
                   "    tgt_trixel INTEGER NOT NULL)"   # Trixel ID in target HTMesh
                   )
    # cursor.execute("CREATE INDEX IF NOT EXISTS idx_hd ON HenryDraper(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_file ON {KSBIN_TABLE}(file)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_offset ON {KSBIN_TABLE}(offset)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_hd_{KSBIN_TABLE} ON {KSBIN_TABLE}(HD)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_tgt_trixel ON {KSBIN_TABLE}(tgt_trixel)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_trixel ON {KSBIN_TABLE}(trixel)")
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx_tr_index ON {KSBIN_TABLE}(tr_index)")

    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{STARNAMES_TABLE}` ("
        "    ksbin_id INTEGER PRIMARY KEY,"  # id in the ksbin table
        "    longname TEXT,"                 # Full name of the star
        "    bayername TEXT)"                # Bayer designation
    )
    ##
    # NOTE: WE DO NOT APPLY THE BELOW CORRECTION TO MATCH PRECISELY!
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
                   (KSBIN_TABLE, json.dumps({
                       "files": meta,
                       "target_htm_level": TARGET_HTM_LEVEL})))
    cursor.execute("INSERT OR REPLACE INTO metadata (table_name, info) VALUES (?, ?)",
                   (STARNAMES_TABLE, json.dumps({
                       "file": starnames_path
     })))
    cursor.execute(f"DELETE FROM {KSBIN_TABLE}")
    cursor.execute(f"DELETE FROM {STARNAMES_TABLE}")

    for key, info in meta.items():
        logger.info(f'Processing {key} stars from {info["path"]}')
        reader = stardataio.KSStarDataReader(info['path'])
        if key == 'named':
            # Also open starnames.dat and ingest star names
            starnames = stardataio.KSBinFileReader(starnames_path)[0] # All data in trixel 0
            starname_index = 0

        ep, eq = info['ep'], info['eq']
        for trixel in tqdm.tqdm(reader, total=len(reader)):
            for tr_index, star in enumerate(trixel):
                r, d = star['RA'] * 15.0, star['Dec']   # degrees
                pmra, pmdec = star['dRA'], star['dDec'] # mas/yr
                HD = star.get_unscaled('HD')
                V = star['mag']
                B = V + star['bv_index']
                if HD == 0:
                    HD = None
                named = False
                flags = int.from_bytes(bytes(star['flags'], encoding='utf-8'), signed=False)
                if flags & 0x01:
                    if key != 'named':
                        raise RuntimeError(f'Star with flags {hex(flags)}, indicative of being named, found in {key} stars file')
                    bayerName, longName = starnames[starname_index]['bayerName'].strip(' '), starnames[starname_index]['longName'].strip(' ')
                    if len(bayerName) == 0:
                        bayerName = None
                    if len(longName) == 0:
                        longName = None
                    starname_index += 1
                    named = True

                # # Convert epoch / equinox to ICRS
                # epra, epdec = None, None
                # if ep != eq:
                #     epra, epdec = r, d
                #     r, d = CC.precess(r, d, eq, ep)
                # if ep != 2000.0:
                #     r, d = CC.proper_motion(r, d, pmra, pmdec, ep, 2000.0)
                #     r, d = CC.precess(r, d, ep, 2000.0)

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

                if pmra == 0. and pmdec == 0.:
                    pmra, pmdec = None, None
                cursor.execute(
                    f"INSERT INTO {KSBIN_TABLE} (ra, dec, file, trixel, tr_index, offset, HD, B, V, pmra, pmdec, tgt_trixel) "
                    f"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    (r, d, key, trixel.id(), tr_index, star.offset, HD, B, V, pmra, pmdec, tgt_trixel)
                )
                if named:
                    ksbin_id = cursor.lastrowid
                    cursor.execute(
                        f"INSERT INTO {STARNAMES_TABLE} (ksbin_id, longname, bayername) VALUES (?, ?, ?)",
                        (ksbin_id, longName, bayerName)
                    )


    return KSBIN_TABLE

def find_pm_duplicates(indexer: pykstars.Indexer, cursor: sqlite3.Cursor) -> str:
    N_trixel = (4 ** indexer.level) * 8
    src_indexer = pykstars.Indexer(SOURCE_HTM_LEVEL)
    TABLE_NAME = TABLE_NAMES.PM_DUPLICATES
    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{TABLE_NAME}` ("
        "    dup_id INTEGER PRIMARY KEY," # ID of duplicate in the kstars binary file table
        "    orig_id INTEGER NOT NULL)"   # ID of original in the kstars binary file table
    )
    cursor.execute(f"DELETE FROM {TABLE_NAME}")

    total_pm_duplicates = 0
    for tgt_trixel in tqdm.tqdm(range(N_trixel)): # We query one trixel at a time to avoid loading the entire table in memory
        ksbin_rows = cursor.execute(
            f"SELECT `id`, `trixel`, `ra`, `dec` FROM {TABLE_NAMES.KSBIN} WHERE `tgt_trixel` = {tgt_trixel}").fetchall()
        for i, src_trixel, ra, dec in ksbin_rows:
            expected_src_trixel = src_indexer.get_trixel(ra, dec)
            if src_trixel != expected_src_trixel:
                found = False
                cursor.execute(f"SELECT `id`, `ra`, `dec` FROM {TABLE_NAMES.KSBIN} WHERE `trixel` = {expected_src_trixel}")
                for dup_id, dup_ra, dup_dec in cursor:
                    if abs(ra - dup_ra) < 1e-7 and abs(dec - dup_dec) < 1e-7:
                        cursor.execute(f"INSERT INTO {TABLE_NAME} (dup_id, orig_id) VALUES (?, ?)",
                                       (i, dup_id))
                        found = True
                        break

                if found:
                    total_pm_duplicates += 1
                    continue # We've found the duplicate, so go on to the next row

                logger.info(f'Star in {TABLE_NAMES.KSBIN} with id = {i} was expected to have source trixel {expected_src_trixel} but has {src_trixel} instead. So it must be a proper motion duplicate, but mining for the original did not produce any candidates!')

                if expected_src_trixel in set(src_indexer.get_trixels(ra, dec, 1e-4)):
                    # Not a duplicate, not an error, don't skip
                    logger.info(f'Fuzzy match with radius of 1e-4 does return {expected_src_trixel}, so should be a computation error')
                else:
                    embed(header=f'Failed to even fuzzy match the source trixel! Please inspect!')

    logger.info(f'Found {total_pm_duplicates} proper motion duplicates and wrote them to table {TABLE_NAME}')
    cursor.execute(f"CREATE VIEW IF NOT EXISTS {TABLE_NAMES.KSBIN_NODUPS} AS "
                   f"SELECT {TABLE_NAMES.KSBIN}.* FROM {TABLE_NAMES.KSBIN} "
                   f"LEFT JOIN {TABLE_NAMES.PM_DUPLICATES} "
                   f"ON {TABLE_NAMES.KSBIN}.id = {TABLE_NAMES.PM_DUPLICATES}.dup_id "
                   f"WHERE {TABLE_NAMES.PM_DUPLICATES}.dup_id IS NULL")

    return TABLE_NAME

def prepare_xmatch_tables(table_name: str, cursor: sqlite3.Cursor):
    # Match table
    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{table_name}` ("
        "    ks_id INTEGER PRIMARY KEY,"
        "    xm_id INTEGER NOT NULL,"
        "    match_dist REAL NOT NULL,"
        "    delta_mag REAL)"
    )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{table_name}__xm ON {table_name}(xm_id)")
    cursor.execute(f"DELETE FROM {table_name}")

    # Nearest-Neighbor table
    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{table_name}_nn` ("
        "    ks_id INTEGER PRIMARY_KEY,"
        "    xm_id INTEGER NOT NULL,"
        "    dist REAL NOT NULL)"
    )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{table_name}_nn__xm ON {table_name}_nn(xm_id)")
    cursor.execute(f"DELETE FROM {table_name}_nn")

def read_ksbin_from_trixel(trixel: int, cursor: sqlite3.Cursor):
    return cursor.execute(
        f"SELECT `id`, `ra`, `dec`, `V` FROM {TABLE_NAMES.KSBIN_NODUPS} "
        f"WHERE {TABLE_NAMES.KSBIN_NODUPS}.tgt_trixel = {trixel} "
    ).fetchall()

def distance_grid(queries: np.ndarray, candidates: np.ndarray) -> np.ndarray:
    """
    Computes distances between arrays of celestial points

    queries: N x 2 array of (RA, Dec) of query points
    candidates: M x 2 array of (RA, Dec) of candidates (M >= N usu.)

    Returns an array of size N x M with distances
    """
    radec_pairs = np.concatenate([ # This incantation produces (ra1, dec1, ra2, dec2)
        np.broadcast_to(queries, (candidates.shape[0], queries.shape[0], 2)).transpose(2, 1, 0),
        np.broadcast_to(candidates, (queries.shape[0], candidates.shape[0], 2)).transpose(2, 0, 1)]
    ).transpose(1, 2, 0).reshape(-1, 4) # The first index is a flattened combination of (queries[i], candidates[j])

    distances = pykstars.CoordinateConversion.angular_distance(
        radec_pairs[:, 0],
        radec_pairs[:, 1],
        radec_pairs[:, 2],
        radec_pairs[:, 3],
    ).reshape(len(queries), len(candidates))

    return distances


def difference_grid(queries: np.ndarray, candidates: np.ndarray) -> np.ndarray:
    """ Computes differences between scalar quantities

    Takes an M-long array of query values and N-long array of candidate values

    Returns a M x N array of their differences

    delta[i, j] = queries[i] - candidates[j]
    """
    M = len(queries)
    N = len(candidates)
    return (np.broadcast_to(queries, (N, M)) - np.broadcast_to(candidates, (M, N)).T).T

def cross_match(distances: np.ndarray, delta_mag_grid: np.ndarray, params: CrossMatchParams):
    """ Cross-matches and returns nearest neighbors and matches

    distances: M X N array of angular distances in degrees
    delta_mag_grid: M x N array of magnitude differences
    params: cross-matching criterion parameters

    Returns: A tuple (Nearest Neighbor Indices, Match Dictionary mapping Query indices to Match indices)
    """

    nn_inds = distances.argmin(axis=1)
    ABSURDLY_LARGE_DISTANCE = 181
    
    candidate_inds = np.where((distances <= params.free_radius/3600.) | (delta_mag_grid <= params.mag_tolerance),
                        distances, ABSURDLY_LARGE_DISTANCE).argmin(axis=1) # Candidate indices
    matched_query_inds, = np.where(distances[np.arange(candidate_inds.shape[0]), candidate_inds] <= params.mag_constrained_radius)

    return nn_inds, dict(zip(matched_query_inds, candidate_inds[matched_query_inds]))
    

def match_kstars_tycho2(indexer: pykstars.Indexer, cursor: sqlite3.Cursor) -> str:
    """ Cross match KStars star catalogs gainst Tycho2 using (RA, Dec, VT)
    """
    N_trixel = (4 ** indexer.level) * 8

    logger.info(f'Cross-matching Tycho2 with KStars binary files')
    prepare_xmatch_tables(TABLE_NAMES.KS_TYC2, cursor)

    for trixel in tqdm.tqdm(range(N_trixel)):
        ksbin = read_ksbin_from_trixel(trixel, cursor)

        query_trixels = set()
        for _, ra, dec, _ in ksbin:
            for t in indexer.get_trixels(ra, dec, SEARCH_RADIUS/3600.0):
                query_trixels.add(t)

        cursor.execute(
            f"SELECT `id`, COALESCE(`jra`, `epra`), COALESCE(`jdec`, `epdec`), `VT` "
            f"FROM {TABLE_NAMES.TYCHO2} WHERE `tgt_trixel` IN (" + ", ".join(map(str, query_trixels)) + ")"
        )
        tycho2 = cursor.fetchall()

        # Calculate pair-wise angular distance
        ksbin_data = np.asarray(ksbin)[:, 1:4].astype(np.float64)
        tycho2_data = np.asarray(tycho2)[:, 1:].astype(np.float64)

        distances = distance_grid(ksbin_data[:, :2], tycho2_data[:, :2])
        delta_mag_grid = difference_grid(ksbin_data[:, 2:].squeeze(), tycho2_data[:, 2:].squeeze())

        nn_inds, matches = cross_match(distances, delta_mag_grid, CROSS_MATCH_PARAMS['tycho2'])

        try:
            cursor.executemany(
                f"INSERT INTO {TABLE_NAMES.KS_TYC2} (ks_id, xm_id, match_dist, delta_mag) VALUES (?, ?, ?, ?)",
                [(ksbin[i][0], tycho2[j][0], distances[i, j], delta_mag_grid[i, j]) for i, j in matches.items()])
            cursor.executemany(
                f"INSERT INTO {TABLE_NAMES.KS_TYC2}_nn (ks_id, xm_id, dist) VALUES (?, ?, ?)",
                [(ksbin[i][0], tycho2[j][0], distances[i, j]) for i, j in enumerate(nn_inds)])
        except Exception as e:
            embed(header=str(e))

    return TABLE_NAMES.KS_TYC2

def match_kstars_athyg(indexer: pykstars.Indexer, cursor: sqlite3.Cursor) -> str:
    """ Cross match KStars star catalogs against AT-HYG using (RA, Dec)
    """
    N_trixel = (4 ** indexer.level) * 8

    logger.info(f'Cross-matching AT-HYG with KStars binary files')
    prepare_xmatch_tables(TABLE_NAMES.KS_ATHYG, cursor)

    for trixel in tqdm.tqdm(range(N_trixel)):
        ksbin = read_ksbin_from_trixel(trixel, cursor)
        query_trixels = set()
        for _, ra, dec, _ in ksbin:
            for t in indexer.get_trixels(ra, dec, SEARCH_RADIUS/3600.0):
                query_trixels.add(t)

        cursor.execute(
            f"SELECT `id`, `ra`, `dec`, `mag` "
            f"FROM {TABLE_NAMES.ATHYG} WHERE `tgt_trixel` IN (" + ", ".join(map(str, query_trixels)) + ")"
        )
        athyg = cursor.fetchall()

        # Calculate pair-wise angular distance
        try:
            ksbin_data = np.asarray(ksbin)[:, 1:4].astype(np.float64)
            athyg_data = np.asarray(athyg)[:, 1:].astype(np.float64)
        except:
            embed()

        distances = distance_grid(ksbin_data[:, :2], athyg_data[:, :2])
        delta_mag_grid = difference_grid(ksbin_data[:, 2:].squeeze(), athyg_data[:, 2:].squeeze())

        nn_inds, matches = cross_match(distances, delta_mag_grid, CROSS_MATCH_PARAMS['athyg'])

        cursor.executemany(
            f"INSERT INTO {TABLE_NAMES.KS_ATHYG} (ks_id, xm_id, match_dist, delta_mag) VALUES (?, ?, ?, ?)",
            [(ksbin[i][0], athyg[j][0], distances[i, j], delta_mag_grid[i, j]) for i, j in matches.items()])
        cursor.executemany(
            f"INSERT INTO {TABLE_NAMES.KS_ATHYG}_nn (ks_id, xm_id, dist) VALUES (?, ?, ?)",
            [(ksbin[i][0], athyg[j][0], distances[i, j]) for i, j in enumerate(nn_inds)])

    return TABLE_NAMES.KS_ATHYG

def match_kstars_hip_tyc1(indexer: pykstars.Indexer, cursor: sqlite3.Cursor):
    """
    Cross-match the KStars named and unnamed star catalogs against HIP/TYC1 using J1991.25 coords
    """
    N_trixel = (4 ** indexer.level) * 8

    logger.info(f'Cross-matching Hipparcos/Tycho-1 with KStars binary files')

    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{TABLE_NAMES.KS_HIP}` ("
        "    ks_id INTEGER PRIMARY KEY,"
        "    xm_id INTEGER NOT NULL,"
        "    dist REAL NOT NULL)"
    )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAMES.KS_HIP}__xm ON {TABLE_NAMES.KS_HIP}(xm_id)")
    cursor.execute(f"DELETE FROM {TABLE_NAMES.KS_HIP}")

    cursor.execute(
        f"CREATE TABLE IF NOT EXISTS `{TABLE_NAMES.KS_TYC1}` ("
        "    ks_id INTEGER PRIMARY_KEY,"
        "    xm_id TEXT NOT NULL,"
        "    dist REAL NOT NULL)"
    )
    cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{TABLE_NAMES.KS_TYC1}__xm ON {TABLE_NAMES.KS_TYC1}(xm_id)")
    cursor.execute(f"DELETE FROM {TABLE_NAMES.KS_TYC1}")

    for trixel in tqdm.tqdm(range(N_trixel)):
        ksbin = cursor.execute(
            f"SELECT `id`, `ra`, `dec`, `V` FROM {TABLE_NAMES.KSBIN_NODUPS} "
            f"WHERE {TABLE_NAMES.KSBIN_NODUPS}.tgt_trixel = {trixel} "
        ).fetchall()
        query_trixels = set()
        for _, ra, dec, _ in ksbin:
            for t in indexer.get_trixels(ra, dec, SEARCH_RADIUS/3600.0):
                query_trixels.add(t)
            # if epra is not None:
            #     assert epdec is not None
            #     for t in indexer.get_trixels(epra, epdec, SEARCH_RADIUS/3600.0): # Just to be safe
            #         query_trixels.add(t)

        hip = cursor.execute(
            f"SELECT `HIP`, `epra`, `epdec`, `V` "
            f"FROM {TABLE_NAMES.HIPPARCOS} WHERE `tgt_trixel` IN (" + ", ".join(map(str, query_trixels)) + ")"
        ).fetchall()
        tyc1 = cursor.execute(
            f"SELECT `TYC`, `epra`, `epdec`, COALESCE(`V`, `VT`) "
            f"FROM {TABLE_NAMES.TYCHO1} WHERE `tgt_trixel` IN (" + ", ".join(map(str, query_trixels)) + ")"
        ).fetchall()
        # Note that tgt_trixel is computed on jra / jdec

        # Calculate pair-wise angular distance using J1991.25 ICRS coordinates
        try:
            ksbin_data = np.asarray([[ra, dec] for _, ra, dec, _ in ksbin])
            hip_data = np.asarray([[epra, epdec] for _, epra, epdec, _ in hip])
            tyc1_data = np.asarray([[epra, epdec] for _, epra, epdec, _ in tyc1])

            if len(hip_data) > 0:
                distances = distance_grid(ksbin_data, hip_data)
                nn_inds = distances.argmin(axis=1)

                cursor.executemany(
                    f"INSERT INTO {TABLE_NAMES.KS_HIP} (ks_id, xm_id, dist) VALUES (?, ?, ?)",
                    [(ksbin[i][0], hip[j][0], distances[i, j]) for i, j in enumerate(nn_inds)])

            if len(tyc1_data) > 0:
                distances = distance_grid(ksbin_data, tyc1_data)
                nn_inds = distances.argmin(axis=1)

                cursor.executemany(
                    f"INSERT INTO {TABLE_NAMES.KS_TYC1} (ks_id, xm_id, dist) VALUES (?, ?, ?)",
                    [(ksbin[i][0], tyc1[j][0], distances[i, j]) for i, j in enumerate(nn_inds)])
        except Exception as e:
            embed(header=f'{e}')

    return TABLE_NAMES.KS_ATHYG

    


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
        required=False,
        default=None,
        help='Path to namedstars.dat file',
    )
    parser.add_argument(
        '--unnamedstars', '-u',
        type=str,
        required=False,
        default=None,
        help='Path to unnamedstars.dat file',
    )
    parser.add_argument(
        '--deepstars', '-D',
        type=str,
        required=False,
        default=None,
        help='Path to deepstars.dat file',
    )
    parser.add_argument(
        '--starnames', '-N',
        type=str,
        required=False,
        default=None,
        help='Path to starnames.dat file',
    )
    parser.add_argument(
        '--force-hd-tyc2', '-fhd',
        action='store_true',
        help='Force ingestion of HD-Tycho2 even if it seems like it has been performed.'
    )

    parser.add_argument(
        '--force-hip', '-fh',
        action='store_true',
        help='Force ingestion of Hipparcos even if it seems like it has been performed.'
    )

    parser.add_argument(
        '--force-tyc1', '-f1',
        action='store_true',
        help='Force ingestion of Tycho1 even if it seems like it has been performed.'
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
        '--force-athyg', '-fa',
        action='store_true',
        help='Force ingestion of AT-HYG catalog'
    )

    parser.add_argument(
        '--force-pm-duplicates', '-fp',
        action='store_true',
        help='Force recomputation of proper motion duplicates'
    )

    parser.add_argument(
        '--force-xm-tycho2', '-fxt',
        action='store_true',
        help='Force recomputation of cross-match between KStars binary files and Tycho2 even if it seems like it has been performed.'
    )

    parser.add_argument(
        '--force-xm-athyg', '-fxa',
        action='store_true',
        help='Force recomputation of cross-match between KStars binary files and AT-HYG even if it seems like it has been performed.'
    )

    parser.add_argument(
        '--force-xm-hip', '-fxh',
        action='store_true',
        help='Force recomputation of cross-match between KStars binary files and Hipparcos/Tycho1 even if it seems like it has been performed.'
    )

    parser.add_argument(
        '--force-xm', '-fx',
        action='store_true',
        help='Force recomputation of all cross-matches',
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

    if args.force_hip or estimated_row_count(TABLE_NAMES.HIPPARCOS, cursor) < 1:
        logger.info('Ingested Hipparcos into ' + ingest_hipparcos(args.cache_dir, indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of Hipparcos because it seems to be done already.')

    if args.force_tyc1 or estimated_row_count(TABLE_NAMES.TYCHO1, cursor) < 1:
        logger.info('Ingested Tycho-1 into ' + ingest_tycho1(args.cache_dir, indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of Tycho-1 because it seems to be done already.')

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
        logger.info('Re-indexed and ingested KStars binary files into ' + ingest_kstars_binary_files(indexer, meta, args.starnames, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of KStars binary files because it seems to be done already.')

    if args.force_pm_duplicates or estimated_row_count(TABLE_NAMES.PM_DUPLICATES, cursor) < 1:
        logger.info('Found proper-motion duplications and ingested into ' + find_pm_duplicates(indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping finding proper motion duplicates because it seems to be done already.')

    if args.force_athyg or estimated_row_count(TABLE_NAMES.ATHYG, cursor) < 1:
        logger.info('Ingested and indexed AT-HYG into ' + ingest_athyg(indexer, args.cache_dir, cursor))
        connection.commit()
    else:
        logger.warning('Skipping ingestion of AT-HYG because it seems to be done already.')

    if args.force_xm_tycho2 or args.force_xm or estimated_row_count(TABLE_NAMES.KS_TYC2, cursor) < 1:
        logger.info('Generated KStars-Tycho2 cross-match into ' + match_kstars_tycho2(indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping generation of KStars-Tycho2 cross-match because it seems to be done already.')

    if args.force_xm_athyg or args.force_xm or estimated_row_count(TABLE_NAMES.KS_ATHYG, cursor) < 1:
        logger.info('Generated KStars-ATHYG cross-match into ' + match_kstars_athyg(indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping generation of KStars--AT-HYG cross-match because it seems to be done already.')

    if args.force_xm_hip or args.force_xm or estimated_row_count(TABLE_NAMES.KS_HIP, cursor) < 1:
        logger.info('Generated KStars-Hipparcos cross-match into ' + match_kstars_hip_tyc1(indexer, cursor))
        connection.commit()
    else:
        logger.warning('Skipping generation of KStars-Hipparcos cross-match because it seems to be done already.')



    connection.commit()
    connection.close()

    logger.info("Done")


if __name__ == "__main__":
    main(sys.argv)
