#!/usr/bin/env python3

import sys
import numpy as np
import os
import tqdm
import pickle
import glob
import pykstars
import logging
logging.basicConfig(level=logging.INFO)

logger = logging.getLogger("Deduplicator")

def read_star_data(fd):
    def to_str(b: bytes):
        """ Interpret bytes as null-terminated C string """
        return b.split(b'\x00')[0].decode('ascii')

    fd.seek(0)
    metadata = {
        'description': to_str(fd.read(124)),
        'endianness': 'little' if fd.read(2).decode('ascii') == 'SK' else 'big',
        'version': int.from_bytes(fd.read(1)),
    }

    logger.info(f'Human readable description of file: {metadata["description"]}')
    if metadata["endianness"] != "little" or metadata["version"] != 1:
        raise NotImplementedError(f'Unhandled endianness ({metadata["endianness"]}) or version ({metadata["version"]})')

    fields_per_entry = int.from_bytes(fd.read(2), 'little')

    # Read field descriptions
    field_descriptors = []
    for i in range(fields_per_entry):
        field_descriptor = {
            'name': to_str(fd.read(10)),
            'bytes': int.from_bytes(fd.read(1)),
            'type': int.from_bytes(fd.read(1)),
            'scale': int.from_bytes(fd.read(4), 'little'),
        }

        # from binfile.h, enum dataType
        typeint = field_descriptor['type']
        field_descriptor['type_int'] = typeint
        if typeint == 0:
            field_descriptor['type'] = 'char'
        elif typeint == 1:
            field_descriptor['type'] = 'int8'
        elif typeint == 2:
            field_descriptor['type'] = 'uint8'
        elif typeint == 3:
            field_descriptor['type'] = 'int16'
        elif typeint == 4:
            field_descriptor['type'] = 'uint16'
        elif typeint == 5:
            field_descriptor['type'] = 'int32'
        elif typeint == 6:
            field_descriptor['type'] = 'uint32'
        elif typeint == 7:
            field_descriptor['type'] = 'chararray'
        elif typeint == 8:
            field_descriptor['type'] = 'str'
        elif typeint == 128:
            field_descriptor['type'] = 'special'
        else:
            field_descriptor['type'] = 'unknown'

        field_descriptors.append(field_descriptor)

    num_trixels = int.from_bytes(fd.read(4), 'little')
    logger.info(f'Number of trixels: {num_trixels}')

    trixels = []
    for i in range(num_trixels):
        trixels.append({
            'id': int.from_bytes(fd.read(4), 'little'),
            'offset': int.from_bytes(fd.read(4), 'little'),
            'count': int.from_bytes(fd.read(4), 'little')
        })

    maglim = int.from_bytes(fd.read(2), 'little')/100.0
    htm_level = int.from_bytes(fd.read(1))
    max_stars_in_a_trixel = int.from_bytes(fd.read(2), 'little')
    data_offset = fd.tell()

    return {
        'metadata': metadata,
        'fields': field_descriptors,
        'trixels': trixels,
        'maglim': maglim,
        'htm_level': htm_level,
        'max_stars_in_any_trixel': max_stars_in_a_trixel,
        'data_offset': data_offset,
    }


def main():
    parser = argparse.ArgumentParser(description='Download Gaia DR3 data for KStars')
    parser.add_argument(
        '--cache-dir', '-d',
        type=str,
        required=True,
        help='Specify the directory where the Gaia tables have been downloaded and pickled.'
    )
    parser.add_argument(
        '--starlists', '-f',
        nargs=3,
        required=True,
        type=str,
        help='Specify three paths to the three files: namedstars.dat, unnamedstars.dat and deepstars.dat for deduplication'
    )

    args = parser.parse_args(sys.argv[1:])

    if not os.path.isdir(args.cache_dir):
        raise RuntimeError(f'Invalid cache directory path {args.cache_dir}: does not exist')

    raise NotImplementedError

if __name__ == "__main__":
    main()
