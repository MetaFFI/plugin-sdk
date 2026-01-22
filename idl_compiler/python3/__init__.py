"""
MetaFFI Python3 IDL Compiler

Extracts interface definitions from Python modules and generates MetaFFI IDL JSON.
"""

__version__ = "0.1.0"
__author__ = "MetaFFI"

from .extractor import PythonExtractor, SourceType
from .idl_generator import IDLGenerator
from .type_mapper import TypeMapper
from .entity_path import EntityPathGenerator

__all__ = [
    "PythonExtractor",
    "SourceType",
    "IDLGenerator",
    "TypeMapper",
    "EntityPathGenerator"
]
