"""
MetaFFI Entity Path Generator for Python3

Generates entity_path dictionaries based on the centralized entity_path_specs.json.
"""

import json
import os
from typing import Dict, Any


class EntityPathGenerator:
    """Generates entity_path structures for Python3 entities"""

    def __init__(self):
        """Load the entity_path spec for Python3"""
        self.spec = self._load_spec()

    @staticmethod
    def _load_spec() -> Dict[str, Any]:
        """Load entity_path spec for python3 from sdk/idl_entities/entity_path_specs.json"""
        # Find SDK root (assume current file is at sdk/idl_compiler/python3/)
        current_dir = os.path.dirname(os.path.abspath(__file__))  # sdk/idl_compiler/python3/
        idl_compiler_dir = os.path.dirname(current_dir)  # sdk/idl_compiler/
        sdk_dir = os.path.dirname(idl_compiler_dir)  # sdk/
        spec_path = os.path.join(sdk_dir, 'idl_entities', 'entity_path_specs.json')

        if not os.path.exists(spec_path):
            raise FileNotFoundError(f"entity_path_specs.json not found at: {spec_path}")

        with open(spec_path, 'r', encoding='utf-8') as f:
            all_specs = json.load(f)

        if 'python3' not in all_specs:
            raise ValueError("No entity_path spec for language: python3")

        return all_specs['python3']

    def create_function_entity_path(self, name: str, has_varargs: bool = False, has_named_args: bool = False) -> Dict[str, Any]:
        """
        Create entity_path for a module-level function.

        Args:
            name: Function name (e.g., 'my_function')
            has_varargs: True if function accepts *args
            has_named_args: True if function accepts **kwargs

        Returns:
            entity_path dictionary
        """
        entity_path = {"callable": name}

        if has_varargs:
            entity_path["varargs"] = True

        if has_named_args:
            entity_path["named_args"] = True

        return entity_path

    def create_method_entity_path(self, class_name: str, method_name: str,
                                   instance_required: bool = True,
                                   has_varargs: bool = False,
                                   has_named_args: bool = False) -> Dict[str, Any]:
        """
        Create entity_path for a class method.

        Args:
            class_name: Class name
            method_name: Method name
            instance_required: True for instance methods, False for static/class methods
            has_varargs: True if method accepts *args
            has_named_args: True if method accepts **kwargs

        Returns:
            entity_path dictionary
        """
        # Dotted path: ClassName.method_name
        callable_path = f"{class_name}.{method_name}"
        entity_path = {"callable": callable_path}

        if instance_required:
            entity_path["instance_required"] = True

        if has_varargs:
            entity_path["varargs"] = True

        if has_named_args:
            entity_path["named_args"] = True

        return entity_path

    def create_constructor_entity_path(self, class_name: str,
                                       has_varargs: bool = False,
                                       has_named_args: bool = False) -> Dict[str, Any]:
        """
        Create entity_path for a class constructor.

        Args:
            class_name: Class name
            has_varargs: True if constructor accepts *args
            has_named_args: True if constructor accepts **kwargs

        Returns:
            entity_path dictionary
        """
        # Dotted path: ClassName.__init__
        callable_path = f"{class_name}.__init__"
        entity_path = {"callable": callable_path}

        if has_varargs:
            entity_path["varargs"] = True

        if has_named_args:
            entity_path["named_args"] = True

        return entity_path

    def create_global_getter_entity_path(self, global_name: str) -> Dict[str, Any]:
        """
        Create entity_path for a global variable getter.

        Args:
            global_name: Global variable name

        Returns:
            entity_path dictionary
        """
        return {
            "attribute": global_name,
            "getter": True
        }

    def create_global_setter_entity_path(self, global_name: str) -> Dict[str, Any]:
        """
        Create entity_path for a global variable setter.

        Args:
            global_name: Global variable name

        Returns:
            entity_path dictionary
        """
        return {
            "attribute": global_name,
            "setter": True
        }

    def create_field_getter_entity_path(self, class_name: str, field_name: str,
                                        instance_required: bool = True) -> Dict[str, Any]:
        """
        Create entity_path for a class field getter.

        Args:
            class_name: Class name
            field_name: Field name
            instance_required: True for instance fields, False for class/static fields

        Returns:
            entity_path dictionary
        """
        # Dotted path: ClassName.field_name
        attribute_path = f"{class_name}.{field_name}"
        entity_path = {
            "attribute": attribute_path,
            "getter": True
        }

        if instance_required:
            entity_path["instance_required"] = True

        return entity_path

    def create_field_setter_entity_path(self, class_name: str, field_name: str,
                                        instance_required: bool = True) -> Dict[str, Any]:
        """
        Create entity_path for a class field setter.

        Args:
            class_name: Class name
            field_name: Field name
            instance_required: True for instance fields, False for class/static fields

        Returns:
            entity_path dictionary
        """
        # Dotted path: ClassName.field_name
        attribute_path = f"{class_name}.{field_name}"
        entity_path = {
            "attribute": attribute_path,
            "setter": True
        }

        if instance_required:
            entity_path["instance_required"] = True

        return entity_path
