"""
Compiler context for dependency injection.
Allows the compiler to work in different deployment scenarios.
"""

from typing import Any


class CompilerContext:
    """
    Context object that holds dependencies needed by the compiler.
    Allows flexible deployment without hardcoded paths.
    
    Fail-fast: All required modules must be provided at construction time.
    """
    
    def __init__(
        self,
        idl_entities_module: Any,
        metaffi_types_module: Any
    ):
        """
        Initialize compiler context.
        
        Args:
            idl_entities_module: The idl_entities.model module (required)
            metaffi_types_module: The metaffi.metaffi_types module (required)
            
        Raises:
            ValueError: If any required module is None
        """
        if idl_entities_module is None:
            raise ValueError("idl_entities_module is required and cannot be None")
        if metaffi_types_module is None:
            raise ValueError("metaffi_types_module is required and cannot be None")
        
        self._idl_entities_module = idl_entities_module
        self._metaffi_types_module = metaffi_types_module
    
    @property
    def idl_entities(self):
        """Get idl_entities.model module."""
        return self._idl_entities_module
    
    @property
    def metaffi_types(self):
        """Get metaffi.metaffi_types module."""
        return self._metaffi_types_module
