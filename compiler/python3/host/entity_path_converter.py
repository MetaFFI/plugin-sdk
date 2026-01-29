"""
Helper functions for converting entity_path dictionaries to strings.
This module provides convenience wrappers around the entity_path_as_string() methods
from IDL entities.
"""

from typing import Dict, Optional, Any


def function_entity_path_to_string(func: Any, idl_definition: Any) -> str:
    """
    Convert FunctionDefinition entity_path to string.
    
    Args:
        func: FunctionDefinition instance
        idl_definition: IDLDefinition containing metaffi_guest_lib
        
    Returns:
        Entity path string in format "key1=value1,key2=value2"
    """
    return func.entity_path_as_string(idl_definition)


def method_entity_path_to_string(
    method: Any,
    idl_definition: Any,
    parent_entity_path: Optional[Dict[str, str]] = None
) -> str:
    """
    Convert MethodDefinition entity_path to string.
    
    Args:
        method: MethodDefinition instance
        idl_definition: IDLDefinition containing metaffi_guest_lib
        parent_entity_path: Optional parent class entity_path to merge
        
    Returns:
        Entity path string in format "key1=value1,key2=value2"
    """
    return method.entity_path_as_string(idl_definition, parent_entity_path)


def constructor_entity_path_to_string(ctor: Any, idl_definition: Any) -> str:
    """
    Convert ConstructorDefinition entity_path to string.
    
    Args:
        ctor: ConstructorDefinition instance
        idl_definition: IDLDefinition containing metaffi_guest_lib
        
    Returns:
        Entity path string in format "key1=value1,key2=value2"
    """
    return ctor.entity_path_as_string(idl_definition)


def global_entity_path_to_string(
    global_def: Any,
    idl_definition: Any,
    use_getter: bool = True
) -> str:
    """
    Convert GlobalDefinition entity_path to string (via getter or setter).
    
    Args:
        global_def: GlobalDefinition instance
        idl_definition: IDLDefinition containing metaffi_guest_lib
        use_getter: If True, use getter's entity_path; if False, use setter's
        
    Returns:
        Entity path string in format "key1=value1,key2=value2"
    """
    return global_def.entity_path_as_string(idl_definition, use_getter)


def field_entity_path_to_string(
    field: Any,
    idl_definition: Any,
    use_getter: bool = True
) -> str:
    """
    Convert FieldDefinition entity_path to string (via getter or setter).
    
    Args:
        field: FieldDefinition instance
        idl_definition: IDLDefinition containing metaffi_guest_lib
        use_getter: If True, use getter's entity_path; if False, use setter's
        
    Returns:
        Entity path string in format "key1=value1,key2=value2"
    """
    return field.entity_path_as_string(idl_definition, use_getter)
