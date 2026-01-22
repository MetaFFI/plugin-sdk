"""
MetaFFI Python3 Extractor

Extracts interface definitions from Python modules using runtime introspection.
"""

import builtins
import importlib
import importlib.util
import inspect
import os
import pkgutil
import re
import sys
import types
from dataclasses import dataclass, field
from enum import Enum
from inspect import Parameter
from typing import List, Optional, Any


# Ignored built-in constants
IGNORED_BUILTINS = {'False', 'Ellipsis', 'None', 'True', 'NotImplemented', 'super'}


class SourceType(Enum):
    """Type of source to load"""
    FILE = "file"           # Path to .py or .pyc file
    MODULE = "module"       # Importable module name
    PACKAGE = "package"     # Installed package (recursive from root)
    AUTO = "auto"           # Auto-detect (try file first, then module)


@dataclass
class ParameterInfo:
    """Information about a function/method parameter"""
    name: str
    type_str: str
    is_optional: bool = False
    has_default: bool = False
    kind: str = ""  # POSITIONAL_ONLY, VAR_POSITIONAL, etc.


@dataclass
class FunctionInfo:
    """Information about a function or method"""
    name: str
    parameters: List[ParameterInfo] = field(default_factory=list)
    return_types: List[str] = field(default_factory=list)
    comment: Optional[str] = None
    has_varargs: bool = False
    has_named_args: bool = False


@dataclass
class FieldInfo:
    """Information about a class field/property"""
    name: str
    type_str: str
    has_getter: bool = True
    has_setter: bool = True


@dataclass
class ClassInfo:
    """Information about a class"""
    name: str
    constructors: List[FunctionInfo] = field(default_factory=list)
    methods: List[FunctionInfo] = field(default_factory=list)
    fields: List[FieldInfo] = field(default_factory=list)
    has_destructor: bool = False
    comment: Optional[str] = None


@dataclass
class ModuleInfo:
    """Information about a Python module"""
    module_name: str
    functions: List[FunctionInfo] = field(default_factory=list)
    classes: List[ClassInfo] = field(default_factory=list)
    globals: List[FieldInfo] = field(default_factory=list)


class PythonExtractor:
    """Extracts interface definitions from Python modules"""

    def __init__(self, source: str, source_type: SourceType = SourceType.AUTO):
        """
        Initialize extractor with source path.

        Args:
            source: Path to Python file, module name, or package name
            source_type: Type of source (FILE, MODULE, PACKAGE, or AUTO)
        """
        self.source = source.replace('\\', '/') if isinstance(source, str) else source
        self.source_type = source_type
        self.module = self._load_source(source, source_type)

    def _load_source(self, source: str, source_type: SourceType) -> types.ModuleType:
        """Load source based on type"""
        if source_type == SourceType.FILE:
            return self._load_file(source)
        elif source_type == SourceType.MODULE:
            return self._load_module(source)
        elif source_type == SourceType.PACKAGE:
            return self._load_package(source)
        elif source_type == SourceType.AUTO:
            # Auto-detect: try file first, then module
            if os.path.exists(source):
                return self._load_file(source)
            else:
                try:
                    return self._load_module(source)
                except (ImportError, ModuleNotFoundError):
                    raise ValueError(f"Cannot load {source} as file or module")
        else:
            raise ValueError(f"Unknown source type: {source_type}")

    def _load_file(self, file_path: str) -> types.ModuleType:
        """Load Python module from .py or .pyc file"""
        # Check if it's a directory (package)
        if os.path.isdir(file_path):
            init_file = os.path.join(file_path, "__init__.py")
            if os.path.exists(init_file):
                file_path = init_file
            else:
                raise ValueError(f"Directory {file_path} does not contain __init__.py")

        # Load from file
        if not self._is_in_site_packages(file_path):
            # Use importlib.util to load module from file
            module_name = os.path.splitext(os.path.basename(file_path))[0]
            spec = importlib.util.spec_from_file_location(module_name, file_path)
            if spec is None:
                raise ImportError(f"Cannot create spec for {file_path}")

            mod = importlib.util.module_from_spec(spec)
            sys.modules[module_name] = mod

            try:
                spec.loader.exec_module(mod)
            except Exception as e:
                raise ImportError(f"Failed to load module {module_name} from {file_path}: {e}")

            return mod
        else:
            # If installed in site-packages
            module_name = re.sub('.*/site-packages/', '', file_path)
            module_name = module_name.replace('.py', '').replace('.pyc', '')
            module_name = module_name.replace('/', '.')

            try:
                return importlib.import_module(module_name)
            except ImportError as e:
                raise ValueError(f"Failed to import module {module_name}: {e}")

    def _load_module(self, module_name: str) -> types.ModuleType:
        """Load importable module by name"""
        try:
            return importlib.import_module(module_name)
        except ImportError as e:
            raise ValueError(f"Cannot import {module_name} as a module: {e}")

    def _load_package(self, package_name: str) -> types.ModuleType:
        """
        Load package and recursively discover all submodules.

        This is used for extracting IDL from installed pip packages.
        All submodules under the specified root will be imported.
        """
        # Import root package
        try:
            package = importlib.import_module(package_name)
        except ImportError as e:
            raise ValueError(f"Cannot import package {package_name}: {e}")

        # Recursively discover and import submodules
        if hasattr(package, '__path__'):
            for importer, modname, ispkg in pkgutil.walk_packages(
                path=package.__path__,
                prefix=package.__name__ + '.',
                onerror=lambda x: None
            ):
                # Skip private modules
                if modname.split('.')[-1].startswith('_'):
                    continue

                try:
                    importlib.import_module(modname)
                except (ImportError, Exception):
                    # Silently skip modules that fail to import
                    continue

        return package

    @staticmethod
    def _is_in_site_packages(path: str) -> bool:
        """Check if path is in site-packages"""
        return '/site-packages' in path or '\\site-packages' in path

    def extract(self) -> ModuleInfo:
        """Extract all interface definitions from the module"""
        module_info = ModuleInfo(
            module_name=self.module.__name__,
            functions=self._extract_functions(),
            classes=self._extract_classes(),
            globals=self._extract_globals()
        )
        return module_info

    def _extract_globals(self) -> List[FieldInfo]:
        """Extract module-level global variables"""
        global_vars = []

        for name, obj in inspect.getmembers(self.module):
            # Skip builtins
            if self.module.__name__ == 'builtins' and name in IGNORED_BUILTINS:
                continue

            # Skip functions, classes, modules
            if inspect.isfunction(obj) or inspect.isclass(obj) or inspect.ismodule(obj):
                continue

            # Skip private members
            if name.startswith('_'):
                continue

            # This is a global variable
            type_str = self._get_type_string(obj)
            global_vars.append(FieldInfo(
                name=name,
                type_str=type_str,
                has_getter=True,
                has_setter=True
            ))

        return global_vars

    def _extract_functions(self) -> List[FunctionInfo]:
        """Extract module-level functions"""
        functions = []

        for name, obj in inspect.getmembers(self.module):
            if not (inspect.isfunction(obj) or inspect.ismethod(obj) or callable(obj)):
                continue

            # Skip private functions
            if name.startswith('_'):
                continue

            # Skip classes
            if inspect.isclass(obj):
                continue

            func_info = self._extract_function(name, obj, class_name=None)
            if func_info:
                functions.append(func_info)

        return functions

    def _extract_classes(self) -> List[ClassInfo]:
        """Extract classes from the module"""
        classes = []

        for name, cls_obj in inspect.getmembers(self.module, inspect.isclass):
            # Skip private classes
            if name.startswith('_'):
                continue

            class_info = ClassInfo(
                name=name,
                comment=inspect.getdoc(cls_obj) or ""
            )

            # Extract constructors, methods, fields
            constructor_found = False
            found_in_annotations = set()

            # Extract class variables (static/class-level fields)
            for attr_name in dir(cls_obj):
                if attr_name.startswith('_') and attr_name not in ['__init__', '__del__']:
                    continue

                try:
                    attr_value = getattr(cls_obj, attr_name)
                except AttributeError:
                    continue

                # Skip methods and functions
                if inspect.ismethod(attr_value) or inspect.isfunction(attr_value):
                    continue

                # Skip properties (handled separately)
                if isinstance(attr_value, property):
                    continue

                # This is a class variable
                if not inspect.isbuiltin(attr_value) and not inspect.ismodule(attr_value):
                    type_str = self._get_type_string(attr_value)
                    class_info.fields.append(FieldInfo(
                        name=attr_name,
                        type_str=type_str,
                        has_getter=True,
                        has_setter=True
                    ))

            # Extract __annotations__ (type-hinted fields)
            if hasattr(cls_obj, '__annotations__'):
                for field_name, field_type in cls_obj.__annotations__.items():
                    if field_name.startswith('_'):
                        continue

                    type_str = self._annotation_to_string(field_type)
                    class_info.fields.append(FieldInfo(
                        name=field_name,
                        type_str=type_str,
                        has_getter=True,
                        has_setter=True
                    ))
                    found_in_annotations.add(field_name)

            # Extract methods, properties, constructors
            for member_name, member_obj in inspect.getmembers(cls_obj):
                if member_name in found_in_annotations:
                    continue

                # Constructor
                if member_name == '__init__':
                    constructor_found = True
                    func_info = self._extract_function('__init__', member_obj, class_name=name)
                    if func_info:
                        class_info.constructors.append(func_info)

                # Destructor
                elif member_name == '__del__':
                    class_info.has_destructor = True

                # Properties
                elif isinstance(member_obj, property):
                    has_getter = member_obj.fget is not None
                    has_setter = member_obj.fset is not None

                    if has_getter or has_setter:
                        # Extract type from getter or setter
                        type_str = 'any'
                        if has_getter and member_obj.fget:
                            sig = inspect.signature(member_obj.fget)
                            if sig.return_annotation != inspect.Signature.empty:
                                type_str = self._annotation_to_string(sig.return_annotation)

                        class_info.fields.append(FieldInfo(
                            name=member_name,
                            type_str=type_str,
                            has_getter=has_getter,
                            has_setter=has_setter
                        ))

                # Methods
                elif inspect.ismethod(member_obj) or inspect.isfunction(member_obj):
                    if member_name.startswith('_') and member_name not in ['__init__', '__del__']:
                        continue

                    if member_name != '__init__':  # Already handled
                        func_info = self._extract_function(member_name, member_obj, class_name=name)
                        if func_info:
                            class_info.methods.append(func_info)

            # Add default constructor if not found
            if not constructor_found:
                class_info.constructors.append(FunctionInfo(
                    name='__init__',
                    comment='Default constructor',
                    return_types=[name]
                ))

            classes.append(class_info)

        return classes

    def _extract_function(self, name: str, func_obj: Any, class_name: Optional[str]) -> Optional[FunctionInfo]:
        """Extract function/method information"""
        func_info = FunctionInfo(name=name)

        # Extract docstring
        func_info.comment = inspect.getdoc(func_obj) or ""

        # Get signature
        try:
            sig = inspect.signature(func_obj)
        except ValueError:
            # Try without following wrappers
            try:
                sig = inspect.signature(func_obj, follow_wrapped=False)
            except ValueError:
                # Can't get signature - return generic
                return func_info

        # Extract parameters
        for param_name, param in sig.parameters.items():
            # Skip 'self' and 'cls' for methods
            if class_name and param_name in ['self', 'cls']:
                continue

            param_info = ParameterInfo(
                name=param_name,
                type_str='any',
                has_default=(param.default != Parameter.empty),
                is_optional=False,
                kind=param.kind.name
            )

            # Check for varargs and named_args
            if param.kind == Parameter.VAR_POSITIONAL:
                func_info.has_varargs = True
                param_info.name = param_name  # Remove * prefix if present
            elif param.kind == Parameter.VAR_KEYWORD:
                func_info.has_named_args = True
                param_info.name = param_name  # Remove ** prefix if present

            # Extract type annotation
            if param.annotation != Parameter.empty:
                param_info.type_str = self._annotation_to_string(param.annotation)

            func_info.parameters.append(param_info)

        # Extract return type
        if name == '__init__' and class_name:
            # Constructor returns class instance
            func_info.return_types = [class_name]
        elif sig.return_annotation != inspect.Signature.empty:
            return_type_str = self._annotation_to_string(sig.return_annotation)
            if return_type_str and return_type_str != 'None':
                func_info.return_types = [return_type_str]

        return func_info

    @staticmethod
    def _annotation_to_string(annotation: Any) -> str:
        """Convert type annotation to string"""
        if annotation is None or annotation == inspect.Signature.empty:
            return 'any'

        if isinstance(annotation, str):
            # Handle Union types
            if '|' in annotation:
                return 'any'
            return annotation

        if isinstance(annotation, types.UnionType):
            return 'any'

        if hasattr(annotation, '__name__'):
            type_name = annotation.__name__
            if type_name == '_empty':
                return 'any'
            return type_name

        if hasattr(annotation, '__class__'):
            type_name = annotation.__class__.__name__
            if type_name in ['_GenericAlias', 'GenericAlias']:
                # Handle typing.List[int], etc.
                return str(annotation)
            return type_name

        # Fallback
        return str(annotation) if annotation else 'any'

    @staticmethod
    def _get_type_string(obj: Any) -> str:
        """Get type string from object"""
        if hasattr(obj, '__name__'):
            return obj.__name__
        elif hasattr(obj, '__class__'):
            return obj.__class__.__name__
        else:
            return 'any'
