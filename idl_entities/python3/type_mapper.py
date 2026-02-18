"""
MetaFFI Type Mapper for Python3 (Host Compiler)

Maps MetaFFI types → Python3 types for host code generation.
Used by the Python3 host compiler to generate type annotations and
metaffi_type_info structures for entity loading.
"""

from typing import Any, Optional, List, Tuple


def metaffi_type_to_python_type_annotation(metaffi_type: str, dimensions: int = 0, type_alias: Optional[str] = None) -> str:
    """
    Convert MetaFFI type string to Python type annotation.

    Args:
        metaffi_type: MetaFFI type string (e.g., "int32", "string8", "handle")
        dimensions: Number of array dimensions (0 = not an array)
        type_alias: Optional type alias for handle types

    Returns:
        Python type annotation string (e.g., "int", "str", "List[int]", "Any")
    """
    # Handle arrays
    if dimensions > 0:
        base_type = metaffi_type_to_python_type_annotation(metaffi_type, 0, type_alias)
        # For multi-dimensional arrays, nest List types
        result = base_type
        for _ in range(dimensions):
            result = f"List[{result}]"
        return result

    # Map base types
    type_mapping = {
        # Integers
        "int8": "int",
        "int16": "int",
        "int32": "int",
        "int64": "int",
        "uint8": "int",
        "uint16": "int",
        "uint32": "int",
        "uint64": "int",
        # Floats
        "float32": "float",
        "float64": "float",
        # Strings
        "string8": "str",
        "string16": "str",
        "string32": "str",
        # Characters
        "char8": "str",
        "char16": "str",
        "char32": "str",
        # Boolean
        "bool": "bool",
        # Handle
        "handle": type_alias if type_alias else "Any",
        # Callable
        "callable": "Any",  # Python callable type
        # Other
        "any": "Any",
        "null": "None",
        "size": "int",
    }

    # Handle array types (e.g., "int32_array")
    if metaffi_type.endswith("_array"):
        base_type = metaffi_type[:-6]  # Remove "_array" suffix
        base_annotation = type_mapping.get(base_type, "Any")
        return f"List[{base_annotation}]"

    return type_mapping.get(metaffi_type, "Any")


def arg_to_metaffi_type_info(arg: "ArgDefinition", metaffi_types_module: Any) -> Any:
    """
    Convert ArgDefinition to metaffi_type_info object.

    Args:
        arg: ArgDefinition from IDL entities
        metaffi_types_module: metaffi_types module (required)

    Returns:
        metaffi_type_info object for use with load_entity()
    """
    if metaffi_types_module is None:
        raise ValueError("metaffi_types_module is required and cannot be None")

    metaffi_type_enum = _metaffi_type_string_to_enum(arg.type, arg.dimensions, metaffi_types_module)
    alias = arg.type_alias if arg.type_alias else None

    return metaffi_types_module.metaffi_type_info(
        metaffi_type=metaffi_type_enum,
        alias=alias,
        dims=arg.dimensions
    )


def _metaffi_type_string_to_enum(metaffi_type: str, dimensions: int, metaffi_types_module: Any):
    """
    Convert MetaFFI type string to MetaFFITypes enum.

    For 1D arrays of packable primitive types, automatically upgrades to packed
    array types for performance optimization.

    Args:
        metaffi_type: MetaFFI type string
        dimensions: Number of array dimensions
        metaffi_types_module: metaffi_types module (required)

    Returns:
        MetaFFITypes enum value
    """
    if metaffi_types_module is None:
        raise ValueError("metaffi_types_module is required and cannot be None")

    # Handle array types
    if dimensions > 0 or metaffi_type.endswith("_array"):
        # Strip array suffixes to get base type name
        # Must check "_packed_array" before "_array" to avoid leaving "int64_packed"
        is_packed_in_idl = "_packed_array" in metaffi_type
        if is_packed_in_idl:
            base_type = metaffi_type.replace("_packed_array", "")
        else:
            base_type = metaffi_type.replace("_array", "")

        base_enum = _base_type_to_enum(base_type, metaffi_types_module)

        # Determine effective dimensions (if type ends with _array but dims=0, treat as 1D)
        effective_dims = dimensions if dimensions > 0 else 1

        # Use packed array type if IDL explicitly says packed, or if 1D packable primitive
        if is_packed_in_idl or (effective_dims == 1 and _is_packable_primitive(base_type)):
            return base_enum | metaffi_types_module.MetaFFITypes.metaffi_array_type | metaffi_types_module.MetaFFITypes.metaffi_packed_type

        # Otherwise use regular array type
        return base_enum | metaffi_types_module.MetaFFITypes.metaffi_array_type

    return _base_type_to_enum(metaffi_type, metaffi_types_module)


# Set of base types that have packed array variants
_PACKABLE_PRIMITIVES = frozenset({
    "float64", "float32",
    "int8", "int16", "int32", "int64",
    "uint8", "uint16", "uint32", "uint64",
    "bool", "string8", "handle", "callable",
})


def _is_packable_primitive(base_type: str) -> bool:
    """
    Returns True if the given base type has a packed array variant.
    Packed arrays are supported for numeric types, bool, string8, handle, and callable.
    """
    return base_type in _PACKABLE_PRIMITIVES


def _base_type_to_enum(metaffi_type: str, metaffi_types_module: Any):
    """
    Convert base MetaFFI type string to MetaFFITypes enum.

    Args:
        metaffi_type: MetaFFI type string
        metaffi_types_module: metaffi_types module (required)
    """
    if metaffi_types_module is None:
        raise ValueError("metaffi_types_module is required and cannot be None")
    MetaFFITypes = metaffi_types_module.MetaFFITypes
    mapping = {
        "int8": MetaFFITypes.metaffi_int8_type,
        "int16": MetaFFITypes.metaffi_int16_type,
        "int32": MetaFFITypes.metaffi_int32_type,
        "int64": MetaFFITypes.metaffi_int64_type,
        "uint8": MetaFFITypes.metaffi_uint8_type,
        "uint16": MetaFFITypes.metaffi_uint16_type,
        "uint32": MetaFFITypes.metaffi_uint32_type,
        "uint64": MetaFFITypes.metaffi_uint64_type,
        "float32": MetaFFITypes.metaffi_float32_type,
        "float64": MetaFFITypes.metaffi_float64_type,
        "bool": MetaFFITypes.metaffi_bool_type,
        "char8": MetaFFITypes.metaffi_char8_type,
        "char16": MetaFFITypes.metaffi_char16_type,
        "char32": MetaFFITypes.metaffi_char32_type,
        "string8": MetaFFITypes.metaffi_string8_type,
        "string16": MetaFFITypes.metaffi_string16_type,
        "string32": MetaFFITypes.metaffi_string32_type,
        "handle": MetaFFITypes.metaffi_handle_type,
        "callable": MetaFFITypes.metaffi_callable_type,
        "any": MetaFFITypes.metaffi_any_type,
        "null": MetaFFITypes.metaffi_null_type,
        "size": MetaFFITypes.metaffi_size_type,
    }

    return mapping.get(metaffi_type, MetaFFITypes.metaffi_handle_type)


def parameters_to_metaffi_type_infos(parameters: List["ArgDefinition"], metaffi_types_module: Any) -> List:
    """
    Convert list of ArgDefinition parameters to list of metaffi_type_info objects.

    Args:
        parameters: List of ArgDefinition from function/method
        metaffi_types_module: metaffi_types module (required)

    Returns:
        List of metaffi_type_info objects
    """
    return [arg_to_metaffi_type_info(arg, metaffi_types_module) for arg in parameters]


def return_values_to_metaffi_type_infos(return_values: List["ArgDefinition"], metaffi_types_module: Any) -> List:
    """
    Convert list of ArgDefinition return values to list of metaffi_type_info objects.

    Args:
        return_values: List of ArgDefinition from function/method
        metaffi_types_module: metaffi_types module (required)

    Returns:
        List of metaffi_type_info objects
    """
    return [arg_to_metaffi_type_info(arg, metaffi_types_module) for arg in return_values]


# =============================================================================
# TypeMapper class - Maps Python types → MetaFFI types (for IDL generation)
# =============================================================================

class TypeMapper:
    """
    Maps Python type annotations to MetaFFI types.
    Used by the IDL generator to convert Python source types to MetaFFI IDL format.
    """

    # Python type → MetaFFI type mapping
    _PYTHON_TO_METAFFI = {
        # Basic types
        "int": "int64",
        "float": "float64",
        "str": "string8",
        "bool": "bool",
        "bytes": "uint8_array",
        "bytearray": "uint8_array",
        # None type
        "None": "null",
        "NoneType": "null",
        # Any type
        "Any": "any",
        "object": "handle",
    }

    def map_type(self, type_str: str) -> Tuple[str, int]:
        """
        Map a Python type string to MetaFFI type and dimensions.

        Args:
            type_str: Python type annotation string (e.g., "int", "str", "List[int]")

        Returns:
            Tuple of (metaffi_type, dimensions)
        """
        if not type_str:
            return ("any", 0)

        # Handle List types (arrays)
        dimensions = 0
        inner_type = type_str

        while self._is_list_type(inner_type):
            dimensions += 1
            inner_type = self._extract_list_inner_type(inner_type)

        # Map the base type
        metaffi_type = self._map_base_type(inner_type)

        return (metaffi_type, dimensions)

    def get_type_alias(self, type_str: str, metaffi_type: str) -> str:
        """
        Get the type alias for a Python type.

        Args:
            type_str: Original Python type string
            metaffi_type: The mapped MetaFFI type

        Returns:
            Type alias string (the original Python type for handles, empty for primitives)
        """
        # For handle types, preserve the original Python type as alias
        if metaffi_type == "handle":
            # Extract base type if it's a List
            inner_type = type_str
            while self._is_list_type(inner_type):
                inner_type = self._extract_list_inner_type(inner_type)
            return inner_type

        # For primitives, return the Python type annotation
        return type_str

    def _is_list_type(self, type_str: str) -> bool:
        """Check if a type string represents a List type."""
        type_str = type_str.strip()
        return (
            type_str.startswith("List[") or
            type_str.startswith("list[") or
            type_str.startswith("typing.List[")
        )

    def _extract_list_inner_type(self, type_str: str) -> str:
        """Extract the inner type from a List[X] type string."""
        type_str = type_str.strip()

        # Find the opening bracket
        bracket_start = type_str.find("[")
        if bracket_start == -1:
            return type_str

        # Find matching closing bracket
        depth = 0
        for i, char in enumerate(type_str[bracket_start:], bracket_start):
            if char == "[":
                depth += 1
            elif char == "]":
                depth -= 1
                if depth == 0:
                    return type_str[bracket_start + 1:i].strip()

        return type_str

    def _map_base_type(self, type_str: str) -> str:
        """Map a base Python type to MetaFFI type."""
        type_str = type_str.strip()

        # Direct mapping
        if type_str in self._PYTHON_TO_METAFFI:
            return self._PYTHON_TO_METAFFI[type_str]

        # Handle Optional[X] -> maps to base type (with is_optional flag elsewhere)
        if type_str.startswith("Optional[") or type_str.startswith("typing.Optional["):
            inner = self._extract_list_inner_type(type_str)
            return self._map_base_type(inner)

        # Handle Union types - take first non-None type
        if type_str.startswith("Union[") or type_str.startswith("typing.Union["):
            inner = self._extract_list_inner_type(type_str)
            parts = self._split_union_types(inner)
            for part in parts:
                if part.strip() not in ("None", "NoneType"):
                    return self._map_base_type(part.strip())
            return "any"

        # Unknown types become handles (custom classes, etc.)
        return "handle"

    def _split_union_types(self, union_inner: str) -> List[str]:
        """Split Union type arguments, respecting nested brackets."""
        parts = []
        current = ""
        depth = 0

        for char in union_inner:
            if char == "[":
                depth += 1
                current += char
            elif char == "]":
                depth -= 1
                current += char
            elif char == "," and depth == 0:
                parts.append(current.strip())
                current = ""
            else:
                current += char

        if current.strip():
            parts.append(current.strip())

        return parts
