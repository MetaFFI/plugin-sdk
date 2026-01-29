"""
Type mapping utilities for Python host compiler.
Converts MetaFFI types to Python type annotations and metaffi_type_info objects.
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
        base_type = metaffi_type.replace("_array", "")
        base_enum = _base_type_to_enum(base_type, metaffi_types_module)
        # Combine with array flag
        return base_enum | metaffi_types_module.MetaFFITypes.metaffi_array_type
    
    return _base_type_to_enum(metaffi_type, metaffi_types_module)


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
