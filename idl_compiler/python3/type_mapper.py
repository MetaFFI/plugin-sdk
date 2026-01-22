"""
MetaFFI Type Mapper for Python3

Maps Python type annotations to MetaFFI type system (lowercase).
"""

from typing import Any, Tuple


class TypeMapper:
    """Maps Python types to MetaFFI types"""

    # Core type mapping (MUST be lowercase per schema.json)
    BASIC_TYPE_MAP = {
        'int': 'int32',
        'float': 'float64',
        'str': 'string8',
        'bool': 'bool',
        'bytes': 'uint8_array',
        'None': 'null',
        'NoneType': 'null',
        # Special types
        'Any': 'any',
        'object': 'handle',
        # Collections (generic)
        'list': 'handle_array',
        'dict': 'handle',
        'tuple': 'handle',
        'set': 'handle',
        'frozenset': 'handle',
        # Other built-ins
        'complex': 'handle',
        'bytearray': 'uint8_array',
        'memoryview': 'handle',
        'range': 'handle',
        # Type system
        'type': 'handle',
        'typing.Any': 'any',
    }

    @staticmethod
    def map_type(python_type: str) -> Tuple[str, int]:
        """
        Map Python type to MetaFFI type and dimensions.

        Args:
            python_type: Python type string (e.g., 'int', 'List[str]', 'any')

        Returns:
            Tuple of (metaffi_type, dimensions)
            - metaffi_type: lowercase MetaFFI type enum value
            - dimensions: 0 for scalar, 1+ for arrays

        Examples:
            'int' -> ('int32', 0)
            'list' -> ('handle_array', 0)
            'List[int]' -> ('handle_array', 0)
            'any' -> ('any', 0)
        """
        if not python_type:
            return ('any', 0)

        # Clean up the type string
        python_type = python_type.strip()

        # Handle empty or _empty
        if python_type == '' or python_type == '_empty':
            return ('any', 0)

        # Direct mapping
        if python_type in TypeMapper.BASIC_TYPE_MAP:
            return (TypeMapper.BASIC_TYPE_MAP[python_type], 0)

        # Handle Union types (X | Y or Union[X, Y])
        if '|' in python_type or python_type.startswith('Union['):
            return ('any', 0)

        # Handle Optional[X] -> treat as 'any'
        if python_type.startswith('Optional['):
            return ('any', 0)

        # Handle List[X], Dict[X,Y], etc.
        if python_type.startswith('List[') or python_type.startswith('list['):
            return ('handle_array', 0)

        if python_type.startswith('Dict[') or python_type.startswith('dict['):
            return ('handle', 0)

        if python_type.startswith('Tuple[') or python_type.startswith('tuple['):
            return ('handle', 0)

        if python_type.startswith('Set[') or python_type.startswith('set['):
            return ('handle', 0)

        # Handle Callable
        if python_type.startswith('Callable'):
            return ('handle', 0)

        # Unknown types -> handle
        return ('handle', 0)

    @staticmethod
    def get_type_alias(python_type: str, metaffi_type: str) -> str:
        """
        Get type alias for handle types.

        Args:
            python_type: Original Python type string
            metaffi_type: Mapped MetaFFI type

        Returns:
            Type alias string (original type if handle/handle_array, empty otherwise)
        """
        if metaffi_type in ['handle', 'handle_array']:
            return python_type
        return ""
