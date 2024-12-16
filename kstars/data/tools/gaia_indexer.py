#!/usr/bin/env python3

import sys
import numpy as np
import os
import tqdm
import pickle
import glob
from pykstars import Indexer, CoordinateConversion as CC
import logging
import stardataio
from stardataio import FieldDescriptor, KSDataType
import sqlite3
import time
import argparse
from typing import Optional
from IPython import embed
logging.basicConfig(level=logging.INFO)

logger = logging.getLogger("Gaia-Indexer")

TARGET_HTM_LEVEL = 6
GAIA_EPOCH = 2016.0
KSBIN_TABLE = 'ksbin'
GAIA_TRIXEL_DATASTRUCT = [
    FieldDescriptor(name='source_id', bytes=8, type=KSDataType.DT_INT64, scale=0),
    FieldDescriptor(name='RA', bytes=8, type=KSDataType.DT_FLOAT64, scale=0),
    FieldDescriptor(name='Dec', bytes=8, type=KSDataType.DT_FLOAT64, scale=0),
    FieldDescriptor(name='parallax', bytes=8, type=KSDataType.DT_FLOAT64, scale=0),
    FieldDescriptor(name='dRA', bytes=8, type=KSDataType.DT_FLOAT64, scale=0),
    FieldDescriptor(name='dDec', bytes=8, type=KSDataType.DT_FLOAT64, scale=0),
    FieldDescriptor(name='G', bytes=4, type=KSDataType.DT_FLOAT32, scale=0),
    FieldDescriptor(name='GBP', bytes=4, type=KSDataType.DT_FLOAT32, scale=0),
    FieldDescriptor(name='GRP', bytes=4, type=KSDataType.DT_FLOAT32, scale=0),
    # FieldDescriptor(name='HD', bytes=4, type=KSDataType.DT_INT32, scale=1),
    # FieldDescriptor(name='mag', bytes=2, type=KSDataType.DT_INT16, scale=100),
    # FieldDescriptor(name='bv_index', bytes=2, type=KSDataType.DT_INT16, scale=100),
    # FieldDescriptor(name='spec_type', bytes=2, type=KSDataType.DT_CHARV, scale=0),
    # FieldDescriptor(name='flags', bytes=1, type=KSDataType.DT_CHAR, scale=0),
    # FieldDescriptor(name='unused', bytes=1, type=KSDataType.DT_CHAR, scale=100),
]

def main():
    parser = argparse.ArgumentParser(description='Process downloaded Gaia DR3 data for KStars to create HTM indices and binary files')
    parser.add_argument(
        '--gaia-dir', '-d',
        type=str,
        required=True,
        help='Specify the directory where the Gaia tables have been downloaded and pickled.'
    )
    parser.add_argument(
        '--output-dir', '-t',
        type=str,
        required=False,
        default=None,
        help='Specify the directory where the output trixel files will be written. Must have about 20GB free space. Default: gaia_dir/trixels'
    )

    args = parser.parse_args(sys.argv[1:])

    if not os.path.isdir(args.gaia_dir):
        raise RuntimeError(f'Invalid Gaia directory path {args.gaia_dir}: does not exist')
    if args.output_dir is None:
        args.output_dir = os.path.join(args.gaia_dir, 'trixels')
    if not os.path.isdir(args.output_dir):
        os.makedirs(args.output_dir)

    def nullable_float(val, masked=None):
        if np.ma.is_masked(val):
            return masked
        else:
            return float(val)

    existing_trixel_files = glob.glob(os.path.join(args.gaia_dir, f'{stardataio.TRIXEL_PREFIX}*.dat'))
    if len(existing_trixel_files) > 0:
        logger.warning(f'Removing {len(existing_trixel_files)} trixel files in five seconds, example: {existing_trixel_files[:5]}')
        time.sleep(5)
        for f in tqdm.tqdm(existing_trixel_files, desc='Removing files: '):
            os.unlink(f)

    indexer = Indexer(TARGET_HTM_LEVEL)
    writer = stardataio.KSBufferedStarCatalogWriter(
        output=None, # Only writing trixel dir
        trixel_dir=args.gaia_dir,
        htm_level=TARGET_HTM_LEVEL,
        datastruct=GAIA_TRIXEL_DATASTRUCT,
        append=False,
        proper_motion_duplicates=0,
        buffer_limit=32000000,
    )

    # connection = sqlite3.connect(os.path.join(args.gaia_dir, 'gaia_aux.db')) # Metadata
    # cursor = connection.cursor()
    # index_tbl = "gaia_index"
    # cursor.execute(f"CREATE TABLE IF NOT EXISTS {index_tbl} ("
    #                "    source_id INTEGER PRIMARY KEY,"
    #                "    trixel    INTEGER NOT NULL,"
    #                "    tr_index  INTEGER NOT NULL,"
    #                "    copies    INTEGER NOT NULL)")
    # cursor.execute(f"DELETE FROM {index_tbl}")
    # cursor.execute(f"CREATE INDEX IF NOT EXISTS idx__{index_tbl}__trixel ON {index_tbl}(trixel)")

    logger.info(f'Writing pickle data into Trixel binary files at {args.gaia_dir}')
    # TOTAL_DUPS = 0
    for pkl_idx, pkl in enumerate(tqdm.tqdm(sorted(glob.glob(os.path.join(args.gaia_dir, '*.pkl'))), position=0, desc='Pickles: ')):
        with open(pkl, 'rb') as p:
            data = pickle.load(p)

        # Note: Documentation for ESDC_EPOCH_PROP is here:
        # https://www.cosmos.esa.int/web/gaia-users/archive/writing-queries/#epoch_prop
        # The units are (deg, deg, mas, mas/yr, mas/yr, mas/yr)
        # Importantly, although input radial velocity is in km/s, output is in mas/yr
        # Unavailable data has 0.0 which works nicely
        # In case of proper motion params being unavailable, the 2016.0 (ra, dec) is returned unaltered
        pv = data['EPOCH_PROP'] # (ra, dec, parallax, pmra, pmdec, radial_velocity) at J2000.0 epoch, ICRS
        ra, dec, parallax, pmra, pmdec, radial_velocity = (pv[:, i] for i in range(6))

        G, GBP, GRP  = map(lambda x: data[f'phot_{x}_mean_mag'].data.astype(np.float64), ('g', 'bp', 'rp'))
        source_id = data['SOURCE_ID'].data
        trixel = indexer.get_trixel(ra, dec, False)

        N = len(data)
        # metadata = []
        for i in tqdm.tqdm(range(N), position=1, desc=f'Stars@pkl{pkl_idx:03}: '):
            tr_index = writer.add_star_to_trixel(
                int(trixel[i]),
                source_id = int(source_id[i]),
                RA=ra[i] / 15.,   # Dreaded HOURS, for compatibility elsewhere
                Dec=dec[i],
                dRA=pmra[i],
                dDec=pmdec[i],
                parallax=parallax[i],
                G = G[i],
                GBP = GBP[i],
                GRP = GRP[i],
            )
            # TOTAL_DUPS += dups
            # metadata.append((int(ids[i]), int(trixel), int(index), int(dups)))
        # cursor.executemany(
        #     f"INSERT INTO {index_tbl} (source_id, trixel, tr_index, copies) VALUES (?, ?, ?, ?)",
        #     metadata
        # )
        # connection.commit()

    writer.flush()
    # connection.commit()

if __name__ == "__main__":
    main()
