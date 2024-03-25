from collections import namedtuple, OrderedDict
from enum import Enum
from typing import Union, List, Callable, IO
from io import BytesIO
import os
import logging
import functools
import math
import shutil
logger = logging.getLogger("KSBinFileIO")

class KSDataType(Enum):
    DT_CHAR =   0 # Character
    DT_INT8 =   1 # 8-bit Integer
    DT_UINT8 =  2 # 8-bit Unsigned Integer
    DT_INT16 =  3 # 16-bit Integer
    DT_UINT16 = 4 # 16-bit Unsigned Integer
    DT_INT32 =  5 # 32-bit Integer
    DT_UINT32 = 6 # 32-bit Unsigned Integer
    DT_CHARV =  7 # Fixed-length array of characters
    DT_STR =    8 # Variable length array of characters, either terminated by nullptr or by the limit on field size
    DT_SPCL = 128 # Flag indicating that the field requires special treatment (eg: Different bits may mean different things)

FieldDescriptor = namedtuple(
    "FieldDescriptor",
    ["name", "bytes", "type", "scale"]
)

TrixelDescriptor = namedtuple(
    "TrixelDescriptor",
    ["id", "offset", "count"]
)
TrixelDescriptor.__doc__ = """
Describes a trixel by storing its id, data offset in the binary file, and number of entries
"""

_Field = namedtuple('_Field', ['index', 'descriptor', 'data'])

class Record:
    def __init__(self, io, blob: bytes):
        self.io = io
        self.blob = blob
        self._data = None
        self._interpret()
        assert self._data is not None

    def _interpret(self):
        f = BytesIO(self.blob)
        self._data = {
            desc.name: _Field(index=i, descriptor=desc, data=f.read(desc.bytes))
            for i, desc in enumerate(self.io.field_descriptors)
        }

    def fields(self) -> List[str]:
        return list(self._data.keys())

    def __getitem__(self, key: str):
        if key not in self._data:
            raise KeyError(key)

        field = self._data[key]
        desc = field.descriptor
        raw = self.io.get_converter(desc.type)(field.data)
        if isinstance(raw, int):
            return float(raw)/desc.scale
        return raw

    def get_blob(self, key:str):
        if key not in self._data:
            raise KeyError(key)

        return self._data[key].data

    def get_unscaled(self, key:str):
        if key not in self._data:
            raise KeyError(key)

        field = self._data[key]
        desc = field.descriptor
        return self.io.get_converter(desc.type)(field.data)

    def __contains__(self, key: str):
        return key in self._data

    def __repr__(self):
        return 'Record(' + ', '.join([f'{key}={self[key]}' for key in self._data]) + ')'


class Trixel:
    def __init__(self, io, descriptor: TrixelDescriptor):
        self.descriptor = descriptor
        self.io = io

    def id(self):
        return self.descriptor.id

    def __len__(self):
        return self.descriptor.count

    def __getitem__(self, i: int):
        if i < 0:
            i += self.descriptor.count
        if i >= self.descriptor.count or i < 0:
            raise ValueError(i)

        self.io.fd.seek(self.descriptor.offset + i * self.io.record_size)
        blob = self.io.fd.read(self.io.record_size)
        return Record(self.io, blob)

    def __iter__(self):
        for i in range(self.descriptor.count):
            self.io.fd.seek(self.descriptor.offset + i * self.io.record_size)
            blob = self.io.fd.read(self.io.record_size)
            if len(blob) != self.io.record_size:
                raise RuntimeError(f'Incomplete / corrupt file: could not read {self.io.record_size} bytes')
            yield Record(self.io, blob)

    def __repr__(self):
        return f'Trixel(id={self.descriptor.id}, count={self.descriptor.count}, offset={self.descriptor.offset})'

class KSBinFileReader:
    def __init__(self, path: str):
        self.path = path
        self.fd = open(path, 'rb')
        self._read_preamble(self.fd)
        self.read_expansion_fields(self.fd)

    @staticmethod
    def cstr(b: bytes):
        """ Interpret bytes as null-terminated C string """
        return b.split(b'\x00')[0].decode('ascii')

    def read_expansion_fields(self, fd):
        """ To be implemented by subclasses for specific files """
        pass

    def get_converter(self, dtype: KSDataType):
        try:
            endian = self.endian
        except AttributeError:
            logger.warning(f'Conversion method does not know endianness of file, will assume `little`')
            endian = 'little'

        match dtype:
            case KSDataType.DT_CHAR:
                return lambda x : x.decode('ascii')
            case KSDataType.DT_INT8:
                return lambda x : int.from_bytes(x, signed=True)
            case KSDataType.DT_UINT8:
                return lambda x : int.from_bytes(x, signed=False)
            case KSDataType.DT_INT16:
                return lambda x : int.from_bytes(x, endian, signed=True)
            case KSDataType.DT_UINT16:
                return lambda x : int.from_bytes(x, endian, signed=False)
            case KSDataType.DT_INT32:
                return lambda x : int.from_bytes(x, endian, signed=True)
            case KSDataType.DT_UINT32:
                return lambda x : int.from_bytes(x, endian, signed=False)
            case KSDataType.DT_CHARV:
                return lambda x : x.decode('ascii')
            case KSDataType.DT_STR:
                return lambda x : self.cstr(x)
            case KSDataType.DT_SPCL:
                return lambda x: x
            case _:
                raise TypeError(f'Unhandled data type {dtype}')

    def _read_preamble(self, fd):
        fd.seek(0)
        self.description = self.cstr(fd.read(124))
        match fd.read(2).decode('ascii'):
            case "SK":
                self.endian = 'little'
            case "KS":
                self.endian = 'big'
            case _:
                raise ValueError(f'Expected endianness indicator, got {_} instead!')
        self.format_version = int.from_bytes(fd.read(1))

        logger.info(f'Opened file. Description: {self.description}')
        if self.format_version != 1:
            raise NotImplementedError(f'Unhandled format version: {self.format_version}')

        self.fields_per_entry = int.from_bytes(fd.read(2), self.endian)

        # Read field descriptions
        self.field_descriptors = []
        for i in range(self.fields_per_entry):
            self.field_descriptors.append(FieldDescriptor(
                name=self.cstr(fd.read(10)),
                bytes=int.from_bytes(fd.read(1)),
                type=KSDataType(int.from_bytes(fd.read(1))),
                scale=int.from_bytes(fd.read(4), self.endian),
            ))

        self.record_size = sum(field.bytes for field in self.field_descriptors)

        self.num_trixels = int.from_bytes(fd.read(4), self.endian)
        logger.info(f'Number of trixels: {self.num_trixels}')

        self.trixel_descriptors = []
        for i in range(self.num_trixels):
            self.trixel_descriptors.append(TrixelDescriptor(
                id=int.from_bytes(fd.read(4), self.endian),
                offset=int.from_bytes(fd.read(4), self.endian),
                count=int.from_bytes(fd.read(4), self.endian),
            ))

        assert len(self.trixel_descriptors) == self.num_trixels

        self.data_offset = fd.tell()

    def __iter__(self):
        """ Iterate over trixels """
        trixel = 0
        while trixel < self.num_trixels:
            yield Trixel(self, self.trixel_descriptors[trixel])
            trixel += 1

    def __getitem__(self, i: int):
        """ Get a specific trixel (by index, not necessarily same as ID) """
        return Trixel(self, self.trixel_descriptors[i])

    def __len__(self):
        """ Number of trixels """
        return self.num_trixels

    def __del__(self):
        self.fd.close()

    def __repr__(self):
        return 'KSBinFileReader(' + ', '.join(f'{key}={value}' for key, value in [
            ("path", self.path),
            ("description", self.description),
            ("endian", self.endian),
            ("record_size", self.record_size),
            ("num_fields", len(self.field_descriptors)),
            ("num_trixels", self.num_trixels),
            ("data_offset", self.data_offset),
        ]) + ')'

class KSStarDataReader(KSBinFileReader):

    def read_expansion_fields(self, fd):
        fd.seek(self.data_offset)
        self.maglim = int.from_bytes(fd.read(2), self.endian)
        self.htm_level = int.from_bytes(fd.read(1))
        self.max_stars_per_trixel = int.from_bytes(fd.read(2), self.endian)

    def __repr__(self):
        return super().__repr__()[:-2] + f', maglim={self.maglim}, htm_level={self.htm_level}, max_stars_per_trixel={self.max_stars_per_trixel})'

class KSBinFileWriter:

    def __init__(self, output: str, tmp_dir: str, num_trixels: int, sort_trixels=True):
        """
        Creates a binary file writer

        output: Destination file path
        tmp_dir: A temporary directory with enough free disk space to write trixel files (/tmp memory may not be enough)
        field_descriptors: A list of field descriptors (see `FieldDescriptor`) describing the fields written
        num_trixels: The number of trixels in the HTMesh
        sort_trixels: Sort the trixels by ID in the final result
        """

        if os.path.isfile(output):
            logger.warning(f'Output file {output} exists, will be overwritten!')
        self.output = output
        if not os.path.isdir(tmp_dir):
            os.makedirs(tmp_dir)
        self.field_descriptors = OrderedDict()
        self.num_trixels = num_trixels
        self.tmp_dir = tmp_dir
        self._current_trixel_index = 0
        self._trixel_chunks = OrderedDict() # Offsets are invalid and not yet set
        self.endian = 'little'
        self._writer_created = False
        self.sort_trixels = sort_trixels
        self.description = 'KStars binary data'

    def get_converter(self, dtype: KSDataType, length=None, scale=None):
        try:
            endian = self.endian
        except AttributeError:
            logger.warning(f'Conversion method does not know endianness of file, will assume `little`')
            endian = 'little'

        def convert_num(number: Union[int, float], num_bytes: int, signed: bool) -> bytes:
            """ Throws on overflow """
            if scale is not None:
                number = int(number * scale)
            return number.to_bytes(length=num_bytes, byteorder=self.endian, signed=signed)

        def convert_str(text: str, length: int, null_terminate: bool) -> bytes:
            """ Throws on overflow """
            if len(text) > length:
                raise OverflowError(f'String {text} longer than allowed length of {length}')
            result = bytes(text, encoding='ascii')
            if null_terminate:
                while len(result) < length:
                    result += b'\x00'
            if len(result) != length:
                raise ValueError(f'String {text} is shorter than required length of {length}')
            return result

        match dtype:
            case KSDataType.DT_CHAR:
                return lambda x : convert_str(x, 1, False)
            case KSDataType.DT_INT8:
                return lambda x : convert_num(x, 1, True)
            case KSDataType.DT_UINT8:
                return lambda x : convert_num(x, 1, False)
            case KSDataType.DT_INT16:
                return lambda x : convert_num(x, 2, True)
            case KSDataType.DT_UINT16:
                return lambda x : convert_num(x, 2, False)
            case KSDataType.DT_INT32:
                return lambda x : convert_num(x, 4, True)
            case KSDataType.DT_UINT32:
                return lambda x : convert_num(x, 4, False)
            case KSDataType.DT_CHARV:
                if length is None:
                    raise ValueError(f'Need to supply the length argument to serialize DT_CHARV')
                return lambda x : convert_str(x, length, False)
            case KSDataType.DT_STR:
                if length is None:
                    raise ValueError(f'Need to supply the max length argument to serialize DT_STR')
                return lambda x : convert_str(x, length, True)
            case KSDataType.DT_SPCL:
                return lambda x: bytes(x)
            case _:
                raise TypeError(f'Unhandled data type {dtype}')

    def _get_record_writer(self) -> Callable[[IO, ...], int]:
        self._writer_created = True
        """ Constructs a function that can write a record given as keyword args """
        converters = OrderedDict([
            (name, self.get_converter(desc.type, length=desc.bytes, scale=desc.scale))
            for name, desc in self.field_descriptors.items()
        ])

        def write(fd, **params) -> int:
            if len(params) != len(self.field_descriptors):
                raise ValueError(f'Number of parameters {len(params)} does not match number of fields {len(self.field_descriptors)}: {", ".join(self.field_descriptors.keys())}')
            b = BytesIO()
            for name, converter in converters.items():
                if name not in params:
                    raise KeyError(f'Missing expected field {name}')
                b.write(converter(params[name]))

            remainder = set(params) - set(self.field_descriptors)
            if len(remainder) > 0:
                logger.error(f'Ignored parameters: {", ".join(remainder)}')
            return fd.write(b.getbuffer())

        return write

    def get_trixel_writer(self, id: int):
        return TrixelWriter(self, id)

    def __enter__(self):
        return self

    def _register_trixel(self, descriptor, path):
        self._trixel_chunks[descriptor.id] = {
            'descriptor': descriptor,
            'path': path
        }

    def add_description(self, description):
        self.description = description
        if len(description) > 124:
            logger.warning('File description is longer than 124 characters, will be truncated!')

    def add_field_descriptor(self, descriptor: FieldDescriptor, no_check=False):
        if self._writer_created and not no_check:
            raise RuntimeError(f'Refusing to add field after writer has been assembled! Supply no_check=True to force.')
        if len(descriptor.name) > 10:
            raise ValueError(f'Field name {name} is too long (10-character limit)!')
        self.field_descriptors[descriptor.name] = descriptor # N.B. this is an OrderedDict

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type:
            logger.error(f'KSBinFileWriter.__exit__ encountered exception {exc_value} of type {exc_type}. Trixel files will not be removed!!! No output is written.')
            return False

        # Assemble the preamble
        self.fd = open(self.output, 'w+b')
        description = self.get_converter(KSDataType.DT_STR, length=124)(self.description[:124])
        self.fd.write(description)
        assert self.endian == 'little'
        self.fd.write(b"SK") # Little endian
        self.fd.write(b"\x01") # Format version 1

        uint8 = self.get_converter(KSDataType.DT_UINT8)
        uint16 = self.get_converter(KSDataType.DT_UINT16)
        uint32 = self.get_converter(KSDataType.DT_UINT32)

        self.fd.write(uint16(len(self.field_descriptors))) # Number of fields
        for name, descriptor in self.field_descriptors.items():
            self.fd.write(self.get_converter(KSDataType.DT_STR, length=10)(descriptor.name))         # Name
            self.fd.write(uint8(descriptor.bytes))      # Bytes
            self.fd.write(uint8(descriptor.type.value)) # Type
            self.fd.write(uint32(descriptor.scale))     # Scale

        self.fd.write(uint32(self.num_trixels))

        # Write trixel descriptors with zero offsets
        trixel_table_offset = self.fd.tell()
        trixel_ids = list(self._trixel_chunks.keys())
        if len(trixel_ids) != self.num_trixels:
            raise RuntimeError(f'Number of trixels written {len(trixel_ids)} does not match the declared number of trixels {self.num_trixels}')
        if self.sort_trixels:
            trixel_ids = sorted(self._trixel_chunks)
        for trixel_id in trixel_ids:
            self.fd.write(uint32(trixel_id))
            self.fd.write(uint32(0)) # Phony offset
            self.fd.write(uint32(self._trixel_chunks[trixel_id]['descriptor'].count))

        # Write any expansion fields
        self.write_expansion_fields(self.fd)
        data_offset = self.fd.tell()

        # Write the trixel data while fixing offsets in the table
        for i, trixel_id in enumerate(trixel_ids):
            # Fix the offset in the trixel table
            offset = self.fd.tell()
            self.fd.seek(trixel_table_offset + 12 * i + 4)
            self.fd.write(uint32(offset)) # True offset

            # Come back
            self.fd.seek(offset)

            # Copy the trixel's data over
            with open(self._trixel_chunks[trixel_id]['path'], 'rb') as trixel_source:
                shutil.copyfileobj(trixel_source, self.fd)

        # Close the file descriptor
        self.fd.close()

    def __del__(self):
        """ Delete the trixel files in the temporary directory """
        logger.info(f'Removing temporary trixel files')
        for _, trixel_info in self._trixel_chunks.items():
            os.remove(trixel_info['path'])

    def write_expansion_fields(self, fd):
        """ To be implemented by subclasses """
        pass

class TrixelWriter:

    def __init__(self, writer: KSBinFileWriter, id: int):
        if id < 0:
            raise ValueError(f'Invalid trixel id {id} < 0')
        if id >= writer.num_trixels:
            raise ValueError(f'Invalid trixel id {id} exceeds number of trixels {writer.num_trixels}')
        self.writer = writer
        self.id = id

    def __enter__(self):
        self.output = os.path.join(self.writer.tmp_dir, f'trixel{self.writer._current_trixel_index:08}.dat')
        self.writer._current_trixel_index += 1
        if os.path.isfile(self.output):
            raise RuntimeError(f'Trixel file {self.output} already exists. Clear temporary directory or check for race condition!')
        self.fd = open(self.output, 'wb')
        self.count = 0
        self.record_writer = functools.partial(self.writer._get_record_writer(), self.fd)
        return self

    def __len__(self):
        return self.count

    def add_entry(self, **params) -> int:
        """ Returns the number of bytes written """
        num_bytes_written = self.record_writer(**params)
        self.count += 1
        return num_bytes_written

    def __exit__(self, exc_type, exc_value, traceback):
        self.fd.close()
        if exc_type:
            logger.error(f'TrixelWriter.__exit__ encountered exception {exc_value} of type {exc_type} and will remove the file {self.output}')
            os.remove(self.output)
        else:
            self.writer._register_trixel(TrixelDescriptor(id=self.id, count=self.count, offset=0), path=self.output)

STARDATA_FIELDS = [
    FieldDescriptor(name='RA', bytes=4, type=KSDataType.DT_INT32, scale=1000000),
    FieldDescriptor(name='Dec', bytes=4, type=KSDataType.DT_INT32, scale=100000),
    FieldDescriptor(name='dRA', bytes=4, type=KSDataType.DT_INT32, scale=10),
    FieldDescriptor(name='dDec', bytes=4, type=KSDataType.DT_INT32, scale=10),
    FieldDescriptor(name='parallax', bytes=4, type=KSDataType.DT_INT32, scale=10),
    FieldDescriptor(name='HD', bytes=4, type=KSDataType.DT_INT32, scale=1),
    FieldDescriptor(name='mag', bytes=2, type=KSDataType.DT_INT16, scale=100),
    FieldDescriptor(name='bv_index', bytes=2, type=KSDataType.DT_INT16, scale=100),
    FieldDescriptor(name='spec_type', bytes=2, type=KSDataType.DT_CHARV, scale=0),
    FieldDescriptor(name='flags', bytes=1, type=KSDataType.DT_CHAR, scale=0),
    FieldDescriptor(name='unused', bytes=1, type=KSDataType.DT_CHAR, scale=100),
]

DEEPSTARDATA_FIELDS = [
    FieldDescriptor(name='RA', bytes=4, type=KSDataType.DT_INT32, scale=1000000),
    FieldDescriptor(name='Dec', bytes=4, type=KSDataType.DT_INT32, scale=100000),
    FieldDescriptor(name='dRA', bytes=2, type=KSDataType.DT_INT16, scale=100),
    FieldDescriptor(name='dDec', bytes=2, type=KSDataType.DT_INT16, scale=100),
    FieldDescriptor(name='B', bytes=2, type=KSDataType.DT_INT16, scale=1000),
    FieldDescriptor(name='V', bytes=2, type=KSDataType.DT_INT16, scale=1000),
]


class KSStarDataWriter(KSBinFileWriter):

    class DataStruct(Enum):
        STARDATA = 0
        DEEPSTARDATA = 1

    def __init__(self, output:str, tmp_dir: str, num_trixels: int, datastruct: DataStruct, sort_trixels = True):
        super().__init__(output, tmp_dir, num_trixels, sort_trixels=sort_trixels)
        self.datastruct = datastruct
        match datastruct:
            case self.DataStruct.STARDATA:
                self.field_descriptors = OrderedDict([
                    (desc.name, desc) for desc in STARDATA_FIELDS
                ])
            case self.DataStruct.DEEPSTARDATA:
                self.field_descriptors = OrderedDict([
                    (desc.name, desc) for desc in DEEPSTARDATA_FIELDS
                ])
            case _:
                raise ValueError(f'Unhandled star data structure {datastruct}')
        self.maglim = 65.5

    def set_maglim(maglim: float):
        self.maglim = maglim

    def write_expansion_fields(self, fd):
        uint16 = self.get_converter(KSDataType.DT_UINT16)
        maglim_scale = 100 if self.datastruct == self.DataStruct.STARDATA else 1000
        fd.write(uint16(int(self.maglim * maglim_scale)))
        htm_level = round(math.log2(self.num_trixels/8)/2)
        fd.write(self.get_converter(KSDataType.DT_UINT8)(htm_level))
        max_stars_per_trixel = 0
        if len(self._trixel_chunks) > 0:
            max_stars_per_trixel = max(trixel['descriptor'].count for trixel in self._trixel_chunks.values())
        else:
            logger.warning('No trixels were committed!')
        if max_stars_per_trixel >= (1 << 16):
            logger.error(f'Max Stars per Trixel = {max_stars_per_trixel} overflows uint16, will wrap')
        fd.write(uint16(max_stars_per_trixel % (1<< 16))) # Maximum stars per trixel
