from collections import namedtuple, OrderedDict
from enum import Enum
from typing import Union, List, Callable, IO, Optional, Any, Iterable
from io import BytesIO
import os
import logging
import functools
import math
import shutil
import fcntl
import threading
import glob
import pykstars
logging.basicConfig(level=logging.INFO)
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

    # New types added in 2024, currently supported only by this script
    DT_INT64   =  9 # 64-bit Integer
    DT_UINT64  = 10 # 64-bit Unsigned Integer
    DT_FLOAT32 = 11 # 32-bit Floating Point
    DT_FLOAT64 = 12 # 64-bit Floating Point

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

TRIXEL_PREFIX = 'trixel' # Prefix for trixel files

class TrixelIO:
    @staticmethod
    def get_converter_with_endian(endian: str, dtype: KSDataType):
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
            case KSDataType.DT_INT64:
                return lambda x : int.from_bytes(x, endian, signed=True)
            case KSDataType.DT_UINT64:
                return lambda x : int.from_bytes(x, endian, signed=False)
            case KSDataType.DT_FLOAT32:
                return lambda x : struct.unpack(('<' if endian == 'little' else '>') + 'f', x)
            case KSDataType.DT_FLOAT64:
                return lambda x : struct.unpack(('<' if endian == 'little' else '>') + 'd', x)
            case _:
                raise TypeError(f'Unhandled data type {dtype}')

    def __init__(self, fd: IO, field_descriptors: List[FieldDescriptor] = [], owns_fd = False):
        self.fd = fd
        self.field_descriptors = field_descriptors
        self.owns_fd = owns_fd

    @property
    def field_descriptors(self):
        return list(self._field_descriptors)

    @field_descriptors.setter
    def field_descriptors(self, field_descriptors):
        self._field_descriptors = list(field_descriptors)
        self._record_size = sum(field.bytes for field in self._field_descriptors)

    @property
    def record_size(self):
        return self._record_size

    def get_converter(self, dtype: KSDataType):
        try:
            endian = self.endian
        except AttributeError:
            logger.warning(f'Conversion method does not know endianness of file, will assume `little`')
            self.endian = 'little'
            endian = self.endian
        return TrixelIO.get_converter_with_endian(endian, dtype)

    def __del__(self):
        if self.owns_fd:
            self.fd.close()

class Record:
    def __init__(self, io: TrixelIO, offset, blob: bytes):
        self.io = io
        self.offset = offset
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
            return float(raw)/(desc.scale if desc.scale != 0 else 1.)
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
        return f'Record(offset={self.offset}' + ', {' + ', '.join([f'{key}={self[key]}' for key in self._data]) + '})'

    def _asdict(self):
        return {key: self[key] for key in self._data}


class Trixel:
    def __init__(self, io: TrixelIO, descriptor: TrixelDescriptor):
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

        offset = self.descriptor.offset + i * self.io.record_size
        self.io.fd.seek(offset)
        blob = self.io.fd.read(self.io.record_size)
        return Record(self.io, offset, blob)

    def __iter__(self):
        for i in range(self.descriptor.count):
            offset = self.descriptor.offset + i * self.io.record_size
            self.io.fd.seek(offset)
            blob = self.io.fd.read(self.io.record_size)
            if len(blob) != self.io.record_size:
                raise RuntimeError(f'Incomplete / corrupt file: could not read {self.io.record_size} bytes')
            yield Record(self.io, offset, blob)

    def __repr__(self):
        return f'Trixel(id={self.descriptor.id}, count={self.descriptor.count}, offset={self.descriptor.offset})'

class KSBinFileReader(TrixelIO):
    def __init__(self, path: str):
        self.path = path
        super().__init__(open(path, 'rb'))
        self._read_preamble(self.fd)
        self.read_expansion_fields(self.fd)

    @staticmethod
    def cstr(b: bytes):
        """ Interpret bytes as null-terminated C string """
        return b.split(b'\x00')[0].decode('ascii')

    def read_expansion_fields(self, fd):
        """ To be implemented by subclasses for specific files """
        pass

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
        field_descriptors = []
        for i in range(self.fields_per_entry):
            field_descriptors.append(FieldDescriptor(
                name=self.cstr(fd.read(10)),
                bytes=int.from_bytes(fd.read(1)),
                type=KSDataType(int.from_bytes(fd.read(1))),
                scale=int.from_bytes(fd.read(4), self.endian),
            ))
        self.field_descriptors = field_descriptors

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

class TrixelChunk:
    def __init__(self):
        self.descriptor = None
        self.path = None
        self.auto_delete = False


class KSBinFileWriter:

    def __init__(self, output: str, tmp_dir: str, num_trixels: int, sort_trixels=True, auto_delete_chunks=True):
        """
        Creates a binary file writer

        output: Destination file path
        tmp_dir: A temporary directory with enough free disk space to write trixel files (/tmp memory may not be enough)
        field_descriptors: A list of field descriptors (see `FieldDescriptor`) describing the fields written
        num_trixels: The number of trixels in the HTMesh
        sort_trixels: Sort the trixels by ID in the final result
        auto_delete_chunks: Automatically delete temporary trixel chunks created in `tmp_dir` upon destruction

        Note: Auto-deletion does not occur if `register_trixel` is called manually with a `path` argument
        """

        if os.path.isfile(output):
            logger.warning(f'Output file {output} exists, will be overwritten!')
        self.output = output
        if not os.path.isdir(tmp_dir):
            os.makedirs(tmp_dir)
        self.field_descriptors = OrderedDict()
        self.num_trixels = num_trixels
        self.tmp_dir = tmp_dir
        self._trixel_chunks = OrderedDict() # Offsets are invalid and not yet set
        self.endian = 'little'
        self._writer_created = False
        self.sort_trixels = sort_trixels
        self.description = 'KStars binary data'
        self.lock = threading.Lock()
        self.record_size = 0
        self.auto_delete_chunks = auto_delete_chunks

    def get_converter(self, dtype: KSDataType, length: Optional[int] = None, scale: Optional[float] = None) -> Callable[Any, bytes]:
        """ Generates a converter function to take raw data and convert it into bytes """
        try:
            endian = self.endian
        except AttributeError:
            logger.warning(f'Conversion method does not know endianness of file, will assume `little`')
            endian = 'little'

        def convert_num(number: Union[int, float], num_bytes: int, signed: bool) -> bytes:
            """ Throws on overflow """
            if scale is not None:
                number = int(number * scale)
            try:
                result = number.to_bytes(length=num_bytes, byteorder=self.endian, signed=signed)
            except Exception as e:
                logger.error(f'Overflow Error while converting {number} to {num_bytes}-byte signed={signed} integer')
                raise
            return result

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
            case KSDataType.DT_INT64:
                return lambda x : convert_num(x, 8, True)
            case KSDataType.DT_UINT64:
                return lambda x : convert_num(x, 8, False)
            case KSDataType.DT_FLOAT32:
                return lambda x : struct.pack('<f', x)
            case KSDataType.DT_FLOAT64:
                return lambda x : struct.pack('<d', x)
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
                try:
                    b.write(converter(params[name]))
                except Exception as e:
                    logger.error(f'Exception while trying to write value {params[name]} for field {name}')
                    raise

            remainder = set(params) - set(self.field_descriptors)
            if len(remainder) > 0:
                logger.error(f'Ignored parameters: {", ".join(remainder)}')
            return fd.write(b.getbuffer())

        return write

    def get_trixel_writer(self, id: int):
        with self.lock:
            chunk = self._trixel_chunks.setdefault(id, TrixelChunk())
            if chunk.path is None:
                chunk.auto_delete = True
            chunk.path = os.path.join(self.tmp_dir, f'{TRIXEL_PREFIX}{id:012}.dat')
        return TrixelWriter(self, id, chunk.path)

    def get_trixel_size(self, id: int):
        with self.lock:
            descriptor = self._trixel_chunks.get(id, TrixelChunk()).descriptor
            if descriptor is not None:
                return descriptor.count
            else:
                return 0

    def __enter__(self):
        return self

    def register_trixel(self, descriptor: TrixelDescriptor, path=None):
        """ If using TrixelWriter, let TrixelWriter call this. Otherwise you can register chunks manually using this method """
        existing_path = self._trixel_chunks.get(descriptor.id, TrixelChunk()).path
        if path is None and existing_path is None:
            raise ValueError(f'Trying to register trixel #{descriptor.id} without a pre-determined path!')
        if path is not None and existing_path is not None:
            logger.warning(f'Overwriting existing path for trixel #{descriptor.id} with {path}')
        with self.lock:
            self._trixel_chunks.setdefault(descriptor.id, TrixelChunk()).descriptor = descriptor
            if path is not None:
                self._trixel_chunks[descriptor.id].path = path
                self._trixel_chunks[descriptor.id].auto_delete = False

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
        self.record_size = sum(field.bytes for field in self.field_descriptors.values())

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
            logger.error(f'Number of trixels written {len(trixel_ids)} does not match the declared number of trixels {self.num_trixels}')
            for i in range(self.num_trixels):
                if i not in self._trixel_chunks:
                    trixel_ids.append(i)
        if self.sort_trixels:
            trixel_ids = sorted(trixel_ids)
        missing_count = 0
        for trixel_id in trixel_ids:
            self.fd.write(uint32(trixel_id))
            self.fd.write(uint32(0)) # Phony offset
            if self._trixel_chunks.get(trixel_id, TrixelChunk()).descriptor is None:
                missing_count += 1
                self.fd.write(uint32(0))
            else:
                self.fd.write(uint32(self._trixel_chunks[trixel_id].descriptor.count))

        logger.warning(f'Trixel descriptors for {missing_count} trixels were not registered! Assumed empty.')

        # Write any expansion fields
        self.write_expansion_fields(self.fd)
        data_offset = self.fd.tell()

        # Write the trixel data while fixing offsets in the table
        missing_count = 0
        for i, trixel_id in enumerate(trixel_ids):
            # Fix the offset in the trixel table
            offset = self.fd.tell()
            self.fd.seek(trixel_table_offset + 12 * i + 4)
            self.fd.write(uint32(offset)) # True offset

            # Come back
            self.fd.seek(offset)

            if trixel_id not in self._trixel_chunks:
                missing_count += 1
                continue

            trixel_path = self._trixel_chunks[trixel_id].path

            # Verify that the number of bytes written matches
            if self.record_size != 0:
                trixel_file_size = os.path.getsize(trixel_path)
                if trixel_file_size  % self.record_size != 0:
                    raise RuntimeError(f'Record size {self.record_size} does not divide the trixel-file size {trixel_file_size} for trixel {trixel_id} at path {trixel_path}')
                expected_count = trixel_file_size // self.record_size
                if self._trixel_chunks[trixel_id].descriptor.count != expected_count:
                    raise RuntimeError(f'Number of entries {expected_count} calculated from file size {trixel_file_size} for trixel {trixel_id} (path: {trixel_path}) does not match the declared count in the descriptor {self._trixel_chunks[trixel_id]}')

            # Copy the trixel's data over
            with open(trixel_path, 'rb') as trixel_source:
                shutil.copyfileobj(trixel_source, self.fd)

        logger.warning(f'Trixel content for {missing_count} trixels were not registered! Assumed empty.')
        # Close the file descriptor
        self.fd.close()

        # Delete trixel files in the temporary directory
        if self.auto_delete_chunks:
            logger.info(f'Removing temporary trixel files')
            for _, trixel_chunk in self._trixel_chunks.items():
                if trixel_chunk.auto_delete:
                    try:
                        os.remove(trixel_chunk.path)
                    except Exception as e:
                        logger.error(f'Exception while trying to remove temporary trixel file {trixel_chunk.path}: {e}')

    def write_expansion_fields(self, fd):
        """ To be implemented by subclasses """
        pass

class TrixelWriter:
    def __init__(self, writer: KSBinFileWriter, id: int, path: str, append: bool = True):
        """ To only be created through KSBinFileWriter.get_trixel_writer() """
        if id < 0:
            raise ValueError(f'Invalid trixel id {id} < 0')
        if id >= writer.num_trixels:
            raise ValueError(f'Invalid trixel id {id} exceeds number of trixels {writer.num_trixels}')
        self.writer = writer
        self.id = id
        self.output = path
        self.append = append

    def __enter__(self):
        if os.path.isfile(self.output) and (not self.append or self.writer.get_trixel_size(self.id) == 0):
            raise RuntimeError(f'Trixel file {self.output} already exists. Clear temporary directory or check for race condition!')
        # Locking mechanism from https://stackoverflow.com/questions/489861/locking-a-file-in-python
        self.fd = open(self.output, 'ab' if self.append else 'wb')
        fcntl.lockf(self.fd, fcntl.LOCK_EX)
        # Read count after locking the file in case a concurrent writer is running on the same trixel
        self.count = self.writer.get_trixel_size(self.id) # Get the current trixel size
        assert self.count >= 0, self.count
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
        # Locking mechanism from https://stackoverflow.com/questions/489861/locking-a-file-in-python
        self.fd.flush()
        os.fsync(self.fd.fileno())
        if exc_type is None:
            # Register trixel before unlocking file so a concurrent writer to the same trixel gets the current count
            self.writer.register_trixel(TrixelDescriptor(id=self.id, count=self.count, offset=0))

        fcntl.lockf(self.fd, fcntl.LOCK_UN)
        self.fd.close()
        if exc_type:
            # Might still result in a weird race condition but only if there is an exception
            logger.error(f'TrixelWriter.__exit__ encountered exception {exc_value} of type {exc_type} and will remove the file {self.output}')
            os.remove(self.output)
            return False
        else:
            return True

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

STARDATAV2_FIELDS = [
    FieldDescriptor(name='RA', bytes=4, type=KSDataType.DT_INT32, scale=1e8),
    FieldDescriptor(name='Dec', bytes=4, type=KSDataType.DT_INT32, scale=1e7),
    FieldDescriptor(name='dRA', bytes=4, type=KSDataType.DT_INT32, scale=100),
    FieldDescriptor(name='dDec', bytes=4, type=KSDataType.DT_INT32, scale=100),
    FieldDescriptor(name='parallax', bytes=4, type=KSDataType.DT_UINT32, scale=1e5),
    FieldDescriptor(name='HD', bytes=4, type=KSDataType.DT_INT32, scale=1),
    FieldDescriptor(name='mag', bytes=2, type=KSDataType.DT_INT16, scale=1000),
    FieldDescriptor(name='bv_index', bytes=2, type=KSDataType.DT_INT16, scale=1000),
    FieldDescriptor(name='spec_type', bytes=2, type=KSDataType.DT_CHARV, scale=0),
    FieldDescriptor(name='flags', bytes=1, type=KSDataType.DT_CHAR, scale=0),
    FieldDescriptor(name='unused', bytes=1, type=KSDataType.DT_CHAR, scale=100),
]

DEEPSTARDATAV2_FIELDS = [
    FieldDescriptor(name='RA', bytes=4, type=KSDataType.DT_UINT32, scale=1e8),
    FieldDescriptor(name='Dec', bytes=4, type=KSDataType.DT_INT32, scale=1e7),
    FieldDescriptor(name='dRA', bytes=2, type=KSDataType.DT_INT16, scale=100), # [-327.68, 327.67]
    FieldDescriptor(name='dDec', bytes=2, type=KSDataType.DT_INT16, scale=100),
    FieldDescriptor(name='B', bytes=2, type=KSDataType.DT_INT16, scale=1000), # [-32.768, 32.767]
    FieldDescriptor(name='V', bytes=2, type=KSDataType.DT_INT16, scale=1000),
]


class KSStarDataStruct(Enum):
    STARDATA = 0
    DEEPSTARDATA = 1

class KSStarDataWriter(KSBinFileWriter):

    """
    Example usage:

    ```
    HTM_LEVEL = 0
    N_TRIXELS = (4 ** HTM_LEVEL) * 8
    with stardataio.KSStarDataWriter('test.dat', '/tmp/trixels', N_TRIXELS, stardataio.KSStarDataStruct.DEEPSTARDATA) as writer:
        for trixel_id in range(N_TRIXELS):
            with writer.get_trixel_writer(trixel_id) as trixel:
                if trixel_id == 3:
                    trixel.add_entry(RA=30.0, Dec=-13.2, dRA=1.5, dDec=2.3, B=12.5, V=13.5)
    ```
    """

    def __init__(self, output:str, tmp_dir: str, num_trixels: int, datastruct: Union[KSStarDataStruct, List[FieldDescriptor]], sort_trixels = True, auto_delete_chunks = True):
        super().__init__(output, tmp_dir, num_trixels, sort_trixels=sort_trixels, auto_delete_chunks=auto_delete_chunks)
        self.datastruct = datastruct
        if isinstance(datastruct, list):
            # We got a list of fields
            for desc in datastruct:
                assert isinstance(desc, FieldDescriptor)
                self.add_field_descriptor(desc)
        else:
            match datastruct:
                case KSStarDataStruct.STARDATA:
                    for desc in STARDATA_FIELDS:
                        self.add_field_descriptor(desc)
                case KSStarDataStruct.DEEPSTARDATA:
                    for desc in DEEPSTARDATA_FIELDS:
                        self.add_field_descriptor(desc)
                case _:
                    raise ValueError(f'Unhandled star data structure {datastruct}')
        self.maglim = 65.5

    def set_maglim(maglim: float):
        self.maglim = maglim

    def write_expansion_fields(self, fd):
        uint16 = self.get_converter(KSDataType.DT_UINT16)
        maglim_scale = 1000 if isinstance(self.datastruct, list) or self.datastruct != KSStarDataStruct.STARDATA else 100
        fd.write(uint16(int(self.maglim * maglim_scale)))
        htm_level = round(math.log2(self.num_trixels/8)/2)
        fd.write(self.get_converter(KSDataType.DT_UINT8)(htm_level))
        max_stars_per_trixel = 0
        if len(self._trixel_chunks) > 0:
            max_stars_per_trixel = max(trixel.descriptor.count for trixel in self._trixel_chunks.values())
        else:
            logger.warning('No trixels were committed!')
        if max_stars_per_trixel >= (1 << 16):
            logger.error(f'Max Stars per Trixel = {max_stars_per_trixel} overflows uint16, will wrap')
        fd.write(uint16(max_stars_per_trixel % (1<< 16))) # Maximum stars per trixel

class KSBufferedStarCatalogWriter(KSStarDataWriter):
    """High-level class to write ICRS/J2000 star catalogs.

    This high-level class handles writing of stars including
    indexing them, making sure to buffer the stars in memory to
    amortize writing latencies, and calculating proper-motion
    duplicates. The catalog coordinates and proper motion are
    assumed to be in the J2000 epoch and ICRS reference
    frame.
    """
    def __init__(self, output:str, trixel_dir: str, htm_level: int, datastruct: KSStarDataStruct, append: bool = False, buffer_limit: int = None, proper_motion_duplicates: int = 10000, proper_motion_threshold: float = 0.1):
        """
        proper_motion_duplicates: Number of years ± from J2000.0 that we should duplicate the star for
        proper_motion_threshold: Threshold in arcseconds that the star should move in above range to do the calculation
        """
        self.htm_level = htm_level
        self.num_trixels = (4 ** htm_level) * 8
        self.indexer = pykstars.Indexer(self.htm_level)
        existing_trixel_files = glob.glob(os.path.join(trixel_dir, f'{TRIXEL_PREFIX}*.dat'))
        if len(existing_trixel_files) > 0 and not append:
            raise RuntimeError(f'Trixel directory {trixel_dir} is not empty while writing in append mode!')
        super().__init__(output, trixel_dir, self.num_trixels, datastruct, sort_trixels=True, auto_delete_chunks=(not append))
        self._trixel_buffers = {}
        self._mem_count = 0
        self._count = 0
        self.buffer_limit = buffer_limit if buffer_limit is not None else (25 * self.num_trixels)
        self.proper_motion_duplicates = proper_motion_duplicates
        self.proper_motion_threshold = proper_motion_threshold

        self.pm_sqr_thresh = (self.proper_motion_threshold/(2 * self.proper_motion_duplicates / 1000)) ** 2

        for existing_trixel in existing_trixel_files:
            self.register_trixel(existing_trixel)

        self.Star = namedtuple("Star", list(self.field_descriptors.keys()))

    def register_trixel(self, *args):
        """
        register_trixel(path: str)   Infers the trixel descriptor from the file
        register_trixel(descriptor: TrixelDescriptor, path: Optional[str])    Forwards to base class
        """
        if len(args) > 1 or isinstance(args[0], TrixelDescriptor):
            return super().register_trixel(*args)

        path, = args

        try:
            trixel_id = int(os.path.splitext(os.path.basename(path))[0][len(TRIXEL_PREFIX):])
        except ValueError:
            raise RuntimeError(f'Trixel file {path} does not match the expected pattern {TRIXEL_PREFIX}####.dat')

        trixel_file_size = os.path.getsize(path)
        if trixel_file_size  % self.record_size != 0:
            raise RuntimeError(f'Record size {self.record_size} does not divide the trixel-file size {trixel_file_size} for trixel {trixel_id} at path {path}')
        count = trixel_file_size // self.record_size
        descriptor = TrixelDescriptor(id=trixel_id, count=count, offset=0)
        super().register_trixel(descriptor, path)
        logger.debug('Registered trixel {descriptor} from {path}')

    def add_star(self, **params) -> int:
        """ Note: RA in hours for compatibility with old catalog convention
        Returns number of times the star was duplicated for proper motion
        """
        star = self.Star(**params) # Checks that parameters match the expectations and raises otherwise
        RA, Dec = star.RA * 15.0, star.Dec
        if self.proper_motion_duplicates is not None and (star.dRA ** 2 + star.dDec ** 2) > self.pm_sqr_thresh:
            future_ra, future_dec = pykstars.CoordinateConversion.proper_motion(
                RA, Dec, star.dRA, star.dDec, 2000.0, 2000.0 + self.proper_motion_duplicates)
            past_ra, past_dec = pykstars.CoordinateConversion.proper_motion(
                RA, Dec, star.dRA, star.dDec, 2000.0, 2000.0 - self.proper_motion_duplicates)
            trixels = self.indexer.get_trixels(past_ra, past_dec, future_ra, future_dec)
        else:
            trixels = [self.indexer.get_trixel(RA, Dec),]
        for trixel in trixels:
            self._trixel_buffers.setdefault(trixel, []).append(star)
        self._count += len(trixels)
        self._mem_count += len(trixels)

        if self._mem_count > self.buffer_limit:
            self.flush(all=False)

        return len(trixels)

    def flush(self, all=True):
        limit = 0 if all else self.buffer_limit // 4
        logger.info(f'Flushing some of the {self._mem_count} stars held in memory up to {limit}')
        trixels = sorted(self._trixel_buffers.keys(), key=lambda k: len(self._trixel_buffers[k])) # Largest trixel buffer last
        while self._mem_count > limit:
            trixel = trixels.pop()
            with self.get_trixel_writer(trixel) as trixel_writer:
                for star in self._trixel_buffers.pop(trixel):
                    trixel_writer.add_entry(**star._asdict())
                    self._mem_count -= 1


    def __enter__(self):
        return super().__enter__()

    def __del__(self):
        self.flush()

    def __exit__(self, *args):
        self.flush()
        return super().__exit__(*args)


class KSTrixelDirReader():
    """ Reads a directory of trixel chunk files without a preamble header. """
    def __init__(self, directory: str, datastruct: KSStarDataStruct):
        self.directory = directory
        self.io = TrixelIO(None) # fd not bound
        match datastruct:
            case KSStarDataStruct.STARDATA:
                self.io.field_descriptors = STARDATA_FIELDS
            case KSStarDataStruct.DEEPSTARDATA:
                self.io.field_descriptors = DEEPSTARDATA_FIELDS
            case _:
                raise ValueError(f'Unhandled star data structure {datastruct}')
        self.trixel_files = glob.glob(os.path.join(directory, f'{TRIXEL_PREFIX}*.dat'))
        self._trixels = {}
        for trixel_file in self.trixel_files:
            self._register_trixel(trixel_file)

    def _register_trixel(self, path:str):
        try:
            trixel_id = int(os.path.splitext(os.path.basename(path))[0][len(TRIXEL_PREFIX):])
        except ValueError:
            raise RuntimeError(f'Trixel file {path} does not match the expected pattern {TRIXEL_PREFIX}####.dat')

        trixel_file_size = os.path.getsize(path)
        if trixel_file_size  % self.io.record_size != 0:
            raise RuntimeError(f'Record size {self.io.record_size} does not divide the trixel-file size {trixel_file_size} for trixel {trixel_id} at path {path}')
        count = trixel_file_size // self.io.record_size
        descriptor = TrixelDescriptor(id=trixel_id, count=count, offset=0)
        chunk = TrixelChunk()
        chunk.descriptor = descriptor
        chunk.path = path
        self._trixels[trixel_id] = chunk
        logger.debug('Registered trixel {descriptor} from {path}')


    def __iter__(self):
        """ Iterate over trixels """
        for trixel_id, chunk in self._trixels.items():
            io = TrixelIO(open(chunk.path, 'rb'), self.io.field_descriptors, owns_fd=True)
            yield Trixel(io, chunk.descriptor)

    def __contains__(self, i: int):
        return i in self._trixels

    def __getitem__(self, i: int):
        """ Get a specific trixel (by index, not necessarily same as ID) """
        if i not in self:
            raise KeyError(f'Trixel {i} not part of {self.__repr__()}')
        chunk = self._trixels[i]
        io = TrixelIO(open(chunk.path, 'rb'), self.io.field_descriptors, owns_fd=True)
        return Trixel(io, chunk.descriptor)

    def __len__(self):
        """ Number of trixels """
        return len(self._trixels)

    def trixel_ids(self) -> List[int]:
        return list(self._trixels.keys())

    def __repr__(self):
        return 'KSTrixelDirReader(' + ', '.join(f'{key}={value}' for key, value in [
            ("directory", self.directory),
            ("record_size", self.io.record_size),
            ("num_fields", len(self.io.field_descriptors)),
            ("num_trixels", len(self._trixels)),
        ]) + ')'

