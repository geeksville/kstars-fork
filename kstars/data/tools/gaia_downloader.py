#!/usr/bin/env python3

import sys
from astroquery.gaia import Gaia
import numpy as np
import argparse
import os
import json
import tqdm
import pickle
from IPython import embed
Gaia.MAIN_GAIA_TABLE = "gaiadr3.gaia_source"

def query(query: str):
    job = Gaia.launch_job_async(query)
    status = job.wait_for_job_end()[0]
    if status != 200:
        return None

    return job.get_results()

def magspace():
    """Return appropriately spaced magnitude bins between 10th mag
    and 17th mag so we can expect under a million results in each
    bin"""

    # Note: Do not change the chunking without purging the downloaded cache, or
    # we will assume the same chunking holds

    return np.log10(np.linspace(10**(0.5*10.0), 10**(0.5*17.0), 300)**2)

def main(argv):
    parser = argparse.ArgumentParser(description='Download Gaia DR3 data for KStars')
    parser.add_argument(
        '--cache-dir', '-d',
        type=str,
        required=True,
        help='Specify a directory where the Gaia tables will be downloaded and pickled. It should have maybe 20GB of free space.'
    )
    args = parser.parse_args(sys.argv[1:])

    if not os.path.isdir(args.cache_dir):
        os.makedirs(args.cache_dir)

    # info = {}
    # info_file = os.path.join(args.cache_dir, 'info.json')
    # if os.path.isfile(info_file):
    #     with open(info_file, 'r') as f:
    #         info = json.load(f)

    mags = magspace()
    maglow = None
    for i, maglim in tqdm.tqdm(enumerate(mags)):
        pkl_file = os.path.join(args.cache_dir, f'data_{i:04}.pkl')
        if os.path.isfile(pkl_file):
            maglow = maglim
            continue # Already downloaded and pickled

        query_i = (
            f"SELECT ra, dec, parallax, pmra, pmdec, phot_g_mean_mag, phot_bp_mean_mag, phot_rp_mean_mag " +
            f"FROM {Gaia.MAIN_GAIA_TABLE} WHERE "
            ) + (
                f"phot_g_mean_mag > {maglow} AND " if maglow is not None else ""
                ) + f"phot_g_mean_mag <= {maglim}"

        print(f'Executing: {query_i}')
        result = query(query_i)
        if result is None:
            maglow = maglim
            print(f'Failed to execute query {query_i} successfully!', file=sys.stderr)
            continue # We'll pick it up after restarting

        try:
            with open(pkl_file, 'wb') as pkl:
                pickle.dump(result, pkl)
        except Exception as e:
            os.rename(pkl_file, f'{pkl_file}.bak')
            print(f'Error {e} while writing pickle {pkl_file}, renamed it to {pkl_file}.bak', file=sys.stderr)

        print(f'{len(result)} stars with G in ({maglow}, {maglim}] pickled into {pkl_file}')
        maglow = maglim

    print(f'Downloading complete!')

if __name__ == "__main__":
    main(sys.argv)
