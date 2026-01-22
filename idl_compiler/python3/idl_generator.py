"""
MetaFFI IDL Generator for Python3

Converts extracted Python interface definitions to MetaFFI IDL JSON format.
"""

import inspect
import json
import os
from typing import Dict, List, Any, Optional

from .extractor import ModuleInfo, FunctionInfo, ClassInfo, FieldInfo, ParameterInfo
from .type_mapper import TypeMapper
from .entity_path import EntityPathGenerator


class IDLGenerator:
    """Generates MetaFFI IDL JSON from extracted Python module information"""

    def __init__(self, source_path: str, module_info: ModuleInfo):
        """
        Initialize IDL generator.

        Args:
            source_path: Path to the source Python file/module
            module_info: Extracted module information
        """
        self.source_path = os.path.abspath(source_path)
        self.module_info = module_info
        self.type_mapper = TypeMapper()
        self.entity_path_gen = EntityPathGenerator()

        # Extract file info
        self.idl_filename_with_extension = os.path.basename(self.source_path)
        self.idl_source = os.path.splitext(self.idl_filename_with_extension)[0]
        self.idl_extension = os.path.splitext(self.idl_filename_with_extension)[1]

    def generate(self) -> Dict[str, Any]:
        """Generate complete MetaFFI IDL JSON"""

        # Generate module
        module_def = self._generate_module()

        # Generate top-level IDL
        idl_def = {
            "idl_source": self.idl_source,
            "idl_extension": self.idl_extension,
            "idl_filename_with_extension": self.idl_filename_with_extension,
            "idl_full_path": self.source_path,
            "metaffi_guest_lib": self.idl_source,  # Auto-generated from idl_source
            "target_language": "python3",
            "modules": [module_def]
        }

        return idl_def

    def generate_json(self) -> str:
        """Generate MetaFFI IDL JSON string"""
        idl_def = self.generate()
        return json.dumps(idl_def, indent=2)

    def _generate_module(self) -> Dict[str, Any]:
        """Generate module definition"""

        module_def = {
            "name": self.module_info.module_name,
            "comment": f"Generated from {self.idl_filename_with_extension}",
            "tags": {},
            "functions": [],
            "classes": [],
            "globals": [],
            "external_resources": [self.module_info.module_name]
        }

        # Generate functions
        for func in self.module_info.functions:
            module_def["functions"].append(self._generate_function(func))

        # Generate globals (as getter/setter function pairs)
        for global_var in self.module_info.globals:
            getter, setter = self._generate_global(global_var)
            module_def["functions"].append(getter)
            module_def["functions"].append(setter)

        # Generate classes
        for cls in self.module_info.classes:
            module_def["classes"].append(self._generate_class(cls))

        return module_def

    def _generate_function(self, func: FunctionInfo) -> Dict[str, Any]:
        """Generate function definition"""

        function_def = {
            "name": func.name,
            "comment": func.comment or "",
            "tags": {},
            "entity_path": self.entity_path_gen.create_function_entity_path(
                func.name,
                has_varargs=func.has_varargs,
                has_named_args=func.has_named_args
            ),
            "parameters": [],
            "return_values": [],
            "overload_index": 0
        }

        # Generate parameters
        for param in func.parameters:
            function_def["parameters"].append(self._generate_parameter(param))

        # Generate return values
        for i, return_type in enumerate(func.return_types):
            function_def["return_values"].append(self._generate_return_value(return_type, i))

        return function_def

    def _generate_method(self, method: FunctionInfo, class_name: str, instance_required: bool) -> Dict[str, Any]:
        """Generate method definition"""

        method_def = {
            "name": method.name,
            "comment": method.comment or "",
            "tags": {},
            "entity_path": self.entity_path_gen.create_method_entity_path(
                class_name,
                method.name,
                instance_required=instance_required,
                has_varargs=method.has_varargs,
                has_named_args=method.has_named_args
            ),
            "parameters": [],
            "return_values": [],
            "overload_index": 0,
            "instance_required": instance_required
        }

        # Generate parameters (skip 'self' for instance methods - already handled by extractor)
        for param in method.parameters:
            method_def["parameters"].append(self._generate_parameter(param))

        # Generate return values
        for i, return_type in enumerate(method.return_types):
            method_def["return_values"].append(self._generate_return_value(return_type, i))

        return method_def

    def _generate_constructor(self, constructor: FunctionInfo, class_name: str) -> Dict[str, Any]:
        """Generate constructor definition"""

        constructor_def = {
            "name": "__init__",
            "comment": constructor.comment or f"Constructor for {class_name}",
            "tags": {},
            "entity_path": self.entity_path_gen.create_constructor_entity_path(
                class_name,
                has_varargs=constructor.has_varargs,
                has_named_args=constructor.has_named_args
            ),
            "parameters": [],
            "return_values": [self._generate_return_value(class_name, 0)],
            "overload_index": 0
        }

        # Generate parameters
        for param in constructor.parameters:
            constructor_def["parameters"].append(self._generate_parameter(param))

        return constructor_def

    def _generate_release(self, class_name: str) -> Dict[str, Any]:
        """Generate release (destructor) method definition"""

        release_def = {
            "name": "release",
            "comment": f"Release {class_name} instance",
            "tags": {},
            "entity_path": self.entity_path_gen.create_method_entity_path(
                class_name,
                "release",
                instance_required=True
            ),
            "parameters": [],
            "return_values": [],
            "overload_index": 0,
            "instance_required": True
        }

        return release_def

    def _generate_field(self, field: FieldInfo, class_name: str, instance_required: bool) -> Dict[str, Any]:
        """Generate field definition with optional getter/setter"""

        metaffi_type, dimensions = self.type_mapper.map_type(field.type_str)
        type_alias = self.type_mapper.get_type_alias(field.type_str, metaffi_type)

        field_def = {
            "name": field.name,
            "type": metaffi_type,
            "type_alias": type_alias,
            "comment": "",
            "tags": {},
            "dimensions": dimensions
        }

        # Generate getter if present
        if field.has_getter:
            getter_func = FunctionInfo(
                name=f"get_{field.name}",
                return_types=[field.type_str]
            )
            field_def["getter"] = {
                "name": f"get_{field.name}",
                "comment": f"Get {field.name}",
                "tags": {},
                "entity_path": self.entity_path_gen.create_field_getter_entity_path(
                    class_name,
                    field.name,
                    instance_required=instance_required
                ),
                "parameters": [],
                "return_values": [self._generate_return_value(field.type_str, 0)],
                "overload_index": 0,
                "instance_required": instance_required
            }
        else:
            field_def["getter"] = None

        # Generate setter if present
        if field.has_setter:
            field_def["setter"] = {
                "name": f"set_{field.name}",
                "comment": f"Set {field.name}",
                "tags": {},
                "entity_path": self.entity_path_gen.create_field_setter_entity_path(
                    class_name,
                    field.name,
                    instance_required=instance_required
                ),
                "parameters": [self._generate_parameter(ParameterInfo(name="value", type_str=field.type_str))],
                "return_values": [],
                "overload_index": 0,
                "instance_required": instance_required
            }
        else:
            field_def["setter"] = None

        return field_def

    def _generate_global(self, global_var: FieldInfo) -> tuple[Dict[str, Any], Dict[str, Any]]:
        """Generate getter and setter functions for a global variable"""

        # Getter function
        getter = {
            "name": f"Get{global_var.name}",
            "comment": f"Get global variable {global_var.name}",
            "tags": {},
            "entity_path": self.entity_path_gen.create_global_getter_entity_path(global_var.name),
            "parameters": [],
            "return_values": [self._generate_return_value(global_var.type_str, 0)],
            "overload_index": 0
        }

        # Setter function
        setter = {
            "name": f"Set{global_var.name}",
            "comment": f"Set global variable {global_var.name}",
            "tags": {},
            "entity_path": self.entity_path_gen.create_global_setter_entity_path(global_var.name),
            "parameters": [self._generate_parameter(ParameterInfo(name="value", type_str=global_var.type_str))],
            "return_values": [],
            "overload_index": 0
        }

        return getter, setter

    def _generate_class(self, cls: ClassInfo) -> Dict[str, Any]:
        """Generate class definition"""

        class_def = {
            "name": cls.name,
            "comment": cls.comment or "",
            "tags": {},
            "entity_path": {},  # Class-level entity_path (Go pattern)
            "constructors": [],
            "release": None,
            "methods": [],
            "fields": []
        }

        # Generate constructors
        for constructor in cls.constructors:
            class_def["constructors"].append(self._generate_constructor(constructor, cls.name))

        # Generate release method (only if destructor exists)
        if cls.has_destructor:
            class_def["release"] = self._generate_release(cls.name)

        # Generate methods
        for method in cls.methods:
            # Detect if method is static/class method
            instance_required = self._is_instance_method(method.name, cls.name)
            class_def["methods"].append(self._generate_method(method, cls.name, instance_required))

        # Generate fields
        for field in cls.fields:
            # Assume instance fields by default
            instance_required = True
            class_def["fields"].append(self._generate_field(field, cls.name, instance_required))

        return class_def

    def _generate_parameter(self, param: ParameterInfo) -> Dict[str, Any]:
        """Generate parameter definition"""

        metaffi_type, dimensions = self.type_mapper.map_type(param.type_str)
        type_alias = self.type_mapper.get_type_alias(param.type_str, metaffi_type)

        param_def = {
            "name": param.name,
            "type": metaffi_type,
            "type_alias": type_alias,
            "comment": "",
            "tags": {},
            "dimensions": dimensions
        }

        # Add is_optional if parameter has default value
        if param.has_default:
            param_def["is_optional"] = True

        return param_def

    def _generate_return_value(self, return_type: str, index: int) -> Dict[str, Any]:
        """Generate return value definition"""

        metaffi_type, dimensions = self.type_mapper.map_type(return_type)
        type_alias = self.type_mapper.get_type_alias(return_type, metaffi_type)

        return_def = {
            "name": f"ret_{index}",
            "type": metaffi_type,
            "type_alias": type_alias,
            "comment": "",
            "tags": {},
            "dimensions": dimensions
        }

        return return_def

    @staticmethod
    def _is_instance_method(method_name: str, class_name: str) -> bool:
        """
        Determine if method is an instance method (vs static/class method).

        Note: This is a heuristic. True detection would require accessing
        the actual class object and using inspect.getattr_static().
        For now, assume all methods are instance methods.
        """
        # TODO: Enhance detection using inspect.getattr_static() if class object available
        return True  # Default: assume instance method
