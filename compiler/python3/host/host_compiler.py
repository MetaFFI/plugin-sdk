"""
Python host compiler main class.
Generates Python code from MetaFFI IDL JSON.
"""

import json
from typing import Dict, Optional, List, Any
from pathlib import Path
from .compiler_context import CompilerContext
from sdk.idl_entities.python3 import type_mapper
from . import entity_path_converter


class HostCompiler:
    """
    Python host compiler that generates Python stubs from MetaFFI IDL JSON.
    """
    
    def __init__(self, context: CompilerContext):
        """
        Initialize the compiler.
        
        Args:
            context: CompilerContext for dependency injection (required).
        """
        if context is None:
            raise ValueError("CompilerContext is required and cannot be None")
        self.context = context
        self.metaffi_types_module = self.context.metaffi_types
    
    def compile(
        self,
        definition: "IDLDefinition",
        output_dir: str,
        output_filename: str,
        host_options: Optional[Dict[str, str]] = None
    ) -> None:
        """
        Compile IDL definition to Python host code.
        
        Args:
            definition: IDLDefinition parsed from JSON
            output_dir: Directory to write output files
            output_filename: Base filename for output (without extension)
            host_options: Optional compiler options (currently unused)
            
        Raises:
            ValueError: If IDL definition is invalid
            IOError: If output directory cannot be created or written to
        """
        if host_options is None:
            host_options = {}
        
        # Validate IDL definition
        self._validate_definition(definition)
        
        # Create output directory if it doesn't exist
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Generate code for each module
        for module in definition.modules:
            self._generate_module_code(module, definition, output_path, output_filename)
    
    def _validate_definition(self, definition: "IDLDefinition") -> None:
        """
        Validate IDL definition (fail-fast policy).
        
        Args:
            definition: IDLDefinition to validate
            
        Raises:
            ValueError: If definition is invalid
        """
        if not definition.target_language:
            raise ValueError("IDL definition missing target_language")
        
        if not definition.metaffi_guest_lib:
            raise ValueError("IDL definition missing metaffi_guest_lib")
        
        if not definition.modules:
            raise ValueError("IDL definition has no modules")
        
        # Validate each module
        for module in definition.modules:
            if not module.name:
                raise ValueError(f"Module missing name")
            
            # Validate functions
            for func in module.functions:
                if not func.name:
                    raise ValueError(f"Function in module {module.name} missing name")
                if not func.entity_path:
                    raise ValueError(f"Function {func.name} in module {module.name} missing entity_path")
            
            # Validate classes
            for cls in module.classes:
                if not cls.name:
                    raise ValueError(f"Class in module {module.name} missing name")
                if not cls.entity_path:
                    raise ValueError(f"Class {cls.name} in module {module.name} missing entity_path")
                
                # Validate methods
                for method in cls.methods:
                    if not method.name:
                        raise ValueError(f"Method in class {cls.name} missing name")
                    if not method.entity_path:
                        raise ValueError(f"Method {method.name} in class {cls.name} missing entity_path")
                
                # Validate constructors
                for ctor in cls.constructors:
                    if not ctor.name:
                        raise ValueError(f"Constructor in class {cls.name} missing name")
                    if not ctor.entity_path:
                        raise ValueError(f"Constructor {ctor.name} in class {cls.name} missing entity_path")
            
            # Validate globals
            for global_def in module.globals:
                if not global_def.name:
                    raise ValueError(f"Global in module {module.name} missing name")
                if not global_def.getter and not global_def.setter:
                    raise ValueError(f"Global {global_def.name} in module {module.name} has neither getter nor setter")
    
    def _generate_module_code(
        self,
        module: "ModuleDefinition",
        definition: "IDLDefinition",
        output_path: Path,
        output_filename: str
    ) -> None:
        """
        Generate Python code for a single module.
        
        Args:
            module: ModuleDefinition to generate code for
            definition: IDLDefinition containing metadata
            output_path: Path to output directory
            output_filename: Base filename for output
        """
        # Create module subdirectory
        module_dir = output_path / module.name
        module_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate output filename
        output_file = module_dir / f"{output_filename}_MetaFFIHost.py"
        
        # Collect all code sections
        code_sections = []
        
        # Header
        code_sections.append(self._generate_header(definition.idl_filename_with_extension))
        code_sections.append("")
        
        # Module initialization
        code_sections.append(self._generate_module_initialization(module, definition))
        code_sections.append("")
        
        # Functions
        for func in module.functions:
            code_sections.append(self._generate_function_stub(func, definition))
            code_sections.append("")
        
        # Globals
        for global_def in module.globals:
            code_sections.append(self._generate_global_accessors(global_def, definition))
            code_sections.append("")
        
        # Classes
        for cls in module.classes:
            # Constructor functions (module-level)
            for ctor in cls.constructors:
                code_sections.append(self._generate_constructor_function(cls, ctor, definition))
                code_sections.append("")
            
            # Class definition
            code_sections.append(self._generate_class_definition(cls, definition))
            code_sections.append("")
        
        # Write to file
        full_code = "\n".join(code_sections)
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(full_code)
    
    def _generate_header(self, idl_filename_with_extension: str) -> str:
        """
        Generate header with imports and environment variable handling.

        Args:
            idl_filename_with_extension: IDL filename for comment

        Returns:
            Python code string with imports
        """
        return f'''# Code generated by MetaFFI. DO NOT EDIT.
# Host code for {idl_filename_with_extension}

import os
import sys

# Add MetaFFI SDK to path based on environment
if 'METAFFI_SOURCE_ROOT' in os.environ:
    # Development environment
    sdk_path = os.path.join(os.environ['METAFFI_SOURCE_ROOT'], 'sdk', 'api', 'python3')
    if sdk_path not in sys.path:
        sys.path.insert(0, sdk_path)
elif 'METAFFI_HOME' in os.environ:
    # Production environment
    sdk_path = os.path.join(os.environ['METAFFI_HOME'], 'sdk', 'api', 'python3')
    if sdk_path not in sys.path:
        sys.path.insert(0, sdk_path)
else:
    raise RuntimeError("Neither METAFFI_SOURCE_ROOT nor METAFFI_HOME environment variables are set. Please set one to point to your MetaFFI installation.")

import metaffi
from metaffi import metaffi_types
from typing import Any, Optional, List, Tuple
'''
    
    def _generate_module_initialization(
        self,
        module: Any,
        idl_definition: Any
    ) -> str:
        """
        Generate module initialization code with entity loading.
        
        Args:
            module: ModuleDefinition to generate code for
            idl_definition: IDLDefinition containing target_language and metaffi_guest_lib
            
        Returns:
            Python code string for module initialization
        """
        lines = [
            "_runtime: Optional[metaffi.MetaFFIRuntime] = None",
            "_module: Optional[metaffi.MetaFFIModule] = None",
            "",
            "# Cached entity callers (MetaFFIEntity instances loaded via _module.load_entity())"
        ]
        
        # Collect all entity variable declarations
        entity_vars = []
        
        # Functions
        for func in module.functions:
            var_name = f"{func.name}_caller"
            entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
        
        # Globals
        for global_def in module.globals:
            if global_def.getter:
                var_name = f"{global_def.name}_getter_caller"
                entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
            if global_def.setter:
                var_name = f"{global_def.name}_setter_caller"
                entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
        
        # Classes
        for cls in module.classes:
            # Constructors
            for ctor in cls.constructors:
                var_name = f"{cls.name}_{ctor.name}_caller"
                entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
            
            # Release
            if cls.release:
                var_name = f"{cls.name}_release_caller"
                entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
            
            # Methods
            for method in cls.methods:
                var_name = f"{cls.name}_{method.name}_caller"
                entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
            
            # Fields
            for field in cls.fields:
                if field.getter:
                    var_name = f"{cls.name}_{field.name}_getter_caller"
                    entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
                if field.setter:
                    var_name = f"{cls.name}_{field.name}_setter_caller"
                    entity_vars.append(f"{var_name}: Optional[metaffi.MetaFFIEntity] = None")
        
        lines.extend(entity_vars)
        lines.append("")
        lines.append("def bind_module_to_code(module_path: str) -> None:")
        lines.append("    global _runtime, _module")
        lines.append(f"    _runtime = metaffi.MetaFFIRuntime('{idl_definition.target_language}')")
        lines.append("    _runtime.load_runtime_plugin()")
        lines.append("    _module = _runtime.load_module(module_path)")
        lines.append("    ")
        lines.append("    # Load and cache all entities using _module.load_entity()")
        
        # Generate load_entity calls
        load_calls = []
        
        # Functions
        for func in module.functions:
            entity_path = entity_path_converter.function_entity_path_to_string(func, idl_definition)
            params_types = type_mapper.parameters_to_metaffi_type_infos(func.parameters, self.metaffi_types_module)
            retval_types = type_mapper.return_values_to_metaffi_type_infos(func.return_values, self.metaffi_types_module)
            params_str = self._format_metaffi_type_infos(params_types)
            retvals_str = self._format_metaffi_type_infos(retval_types)
            var_name = f"{func.name}_caller"
            load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
        
        # Globals
        for global_def in module.globals:
            if global_def.getter:
                entity_path = entity_path_converter.global_entity_path_to_string(global_def, idl_definition, use_getter=True)
                params_types = type_mapper.parameters_to_metaffi_type_infos(global_def.getter.parameters, self.metaffi_types_module)
                retval_types = type_mapper.return_values_to_metaffi_type_infos(global_def.getter.return_values, self.metaffi_types_module)
                params_str = self._format_metaffi_type_infos(params_types)
                retvals_str = self._format_metaffi_type_infos(retval_types)
                var_name = f"{global_def.name}_getter_caller"
                load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
            
            if global_def.setter:
                entity_path = entity_path_converter.global_entity_path_to_string(global_def, idl_definition, use_getter=False)
                params_types = type_mapper.parameters_to_metaffi_type_infos(global_def.setter.parameters, self.metaffi_types_module)
                retval_types = type_mapper.return_values_to_metaffi_type_infos(global_def.setter.return_values, self.metaffi_types_module)
                params_str = self._format_metaffi_type_infos(params_types)
                retvals_str = self._format_metaffi_type_infos(retval_types)
                var_name = f"{global_def.name}_setter_caller"
                load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
        
        # Classes
        for cls in module.classes:
            # Constructors
            for ctor in cls.constructors:
                entity_path = entity_path_converter.constructor_entity_path_to_string(ctor, idl_definition)
                params_types = type_mapper.parameters_to_metaffi_type_infos(ctor.parameters, self.metaffi_types_module)
                retval_types = type_mapper.return_values_to_metaffi_type_infos(ctor.return_values, self.metaffi_types_module)
                params_str = self._format_metaffi_type_infos(params_types)
                retvals_str = self._format_metaffi_type_infos(retval_types)
                var_name = f"{cls.name}_{ctor.name}_caller"
                load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
            
            # Release
            if cls.release:
                entity_path = entity_path_converter.method_entity_path_to_string(cls.release, idl_definition, cls.entity_path)
                params_types = type_mapper.parameters_to_metaffi_type_infos(cls.release.parameters, self.metaffi_types_module)
                retval_types = type_mapper.return_values_to_metaffi_type_infos(cls.release.return_values, self.metaffi_types_module)
                params_str = self._format_metaffi_type_infos(params_types)
                retvals_str = self._format_metaffi_type_infos(retval_types)
                var_name = f"{cls.name}_release_caller"
                load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
            
            # Methods
            for method in cls.methods:
                entity_path = entity_path_converter.method_entity_path_to_string(method, idl_definition, cls.entity_path)
                params_types = type_mapper.parameters_to_metaffi_type_infos(method.parameters, self.metaffi_types_module)
                retval_types = type_mapper.return_values_to_metaffi_type_infos(method.return_values, self.metaffi_types_module)
                params_str = self._format_metaffi_type_infos(params_types)
                retvals_str = self._format_metaffi_type_infos(retval_types)
                var_name = f"{cls.name}_{method.name}_caller"
                load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
            
            # Fields
            for field in cls.fields:
                if field.getter:
                    entity_path = entity_path_converter.field_entity_path_to_string(field, idl_definition, use_getter=True)
                    params_types = type_mapper.parameters_to_metaffi_type_infos(field.getter.parameters, self.metaffi_types_module)
                    retval_types = type_mapper.return_values_to_metaffi_type_infos(field.getter.return_values, self.metaffi_types_module)
                    params_str = self._format_metaffi_type_infos(params_types)
                    retvals_str = self._format_metaffi_type_infos(retval_types)
                    var_name = f"{cls.name}_{field.name}_getter_caller"
                    load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
                
                if field.setter:
                    entity_path = entity_path_converter.field_entity_path_to_string(field, idl_definition, use_getter=False)
                    params_types = type_mapper.parameters_to_metaffi_type_infos(field.setter.parameters, self.metaffi_types_module)
                    retval_types = type_mapper.return_values_to_metaffi_type_infos(field.setter.return_values, self.metaffi_types_module)
                    params_str = self._format_metaffi_type_infos(params_types)
                    retvals_str = self._format_metaffi_type_infos(retval_types)
                    var_name = f"{cls.name}_{field.name}_setter_caller"
                    load_calls.append(f'    {var_name} = _module.load_entity("{entity_path}", {params_str}, {retvals_str})')
        
        lines.extend(load_calls)
        
        return "\n".join(lines)
    
    def _format_metaffi_type_infos(self, type_infos: List) -> str:
        """
        Format list of metaffi_type_info objects as Python tuple literal.
        
        Args:
            type_infos: List of metaffi_type_info objects
            
        Returns:
            String representation like "(metaffi_types.metaffi_type_info(...), ...)"
        """
        if not type_infos:
            return "()"
        
        parts = []
        for ti in type_infos:
            # metaffi_type_info has: type (ctypes.c_uint64), alias (ctypes.c_char_p), is_free_alias (ctypes.c_bool), fixed_dimensions (ctypes.c_int64)
            # Extract values and format as constructor call
            type_value = int(ti.type)
            alias_str = f'"{ti.alias.decode("utf-8")}"' if ti.alias and ti.alias else "None"
            dims = int(ti.fixed_dimensions)
            parts.append(f"metaffi_types.metaffi_type_info(metaffi_types.MetaFFITypes({type_value}), {alias_str}, {dims})")
        
        return f"({', '.join(parts)})"
    
    def _generate_function_stub(self, func: Any, idl_definition: Any) -> str:
        """
        Generate function stub code.
        
        Args:
            func: FunctionDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python function code string
        """
        # Build parameter list with types and defaults
        params = []
        for param in func.parameters:
            type_ann = type_mapper.metaffi_type_to_python_type_annotation(
                param.type, param.dimensions, param.type_alias
            )
            param_str = f"{param.name}: {type_ann}"
            if param.is_optional:
                param_str += " = None"
            params.append(param_str)
        
        params_str = ", ".join(params) if params else ""
        
        # Build return type
        if func.return_values:
            if len(func.return_values) == 1:
                ret_type = type_mapper.metaffi_type_to_python_type_annotation(
                    func.return_values[0].type,
                    func.return_values[0].dimensions,
                    func.return_values[0].type_alias
                )
            else:
                ret_types = [
                    type_mapper.metaffi_type_to_python_type_annotation(
                        rv.type, rv.dimensions, rv.type_alias
                    )
                    for rv in func.return_values
                ]
                ret_type = f"Tuple[{', '.join(ret_types)}]"
        else:
            ret_type = "None"
        
        # Build function body
        param_names = [p.name for p in func.parameters]
        param_names_str = ", ".join(param_names) if param_names else ""
        
        caller_name = f"{func.name}_caller"
        
        lines = [
            f"def {func.name}({params_str}) -> {ret_type}:",
            f'    """',
        ]
        
        if func.comment:
            # Escape triple quotes in comment
            comment = func.comment.replace('"""', '\\"\\"\\"')
            lines.append(f"    {comment}")
        else:
            lines.append(f"    Generated stub for {func.name}")
        
        lines.append(f'    """')
        lines.append(f"    # Use cached MetaFFIEntity to make the call")
        lines.append(f"    result = {caller_name}({param_names_str})")
        
        # Unpack return values
        if func.return_values:
            if len(func.return_values) == 1:
                lines.append("    return result")
            else:
                # Multiple return values - result is already a tuple
                lines.append("    return result")
        else:
            lines.append("    return None")
        
        return "\n".join(lines)
    
    def _generate_class_definition(self, cls: Any, idl_definition: Any) -> str:
        """
        Generate class definition code.
        
        Args:
            cls: ClassDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python class code string
        """
        lines = [
            f"class {cls.name}:",
            f'    """',
        ]
        
        if cls.comment:
            comment = cls.comment.replace('"""', '\\"\\"\\"')
            lines.append(f"    {comment}")
        else:
            lines.append(f"    Generated stub class for {cls.name}")
        
        lines.append(f'    """')
        lines.append("    def __init__(self, handle: Any):")
        lines.append("        self._handle = handle")
        lines.append("")
        
        # Methods
        for method in cls.methods:
            method_code = self._generate_method_stub(cls.name, method, idl_definition)
            lines.append(method_code)
            lines.append("")
        
        # Fields with @property
        for field in cls.fields:
            field_code = self._generate_field_property(cls.name, field, idl_definition)
            lines.append(field_code)
            lines.append("")
        
        # __del__ method
        release_caller = f"{cls.name}_release_caller"
        lines.append("    def __del__(self):")
        lines.append(f"        if self._handle and {release_caller}:")
        lines.append(f"            {release_caller}(self._handle)")
        
        return "\n".join(lines)
    
    def _generate_method_stub(self, class_name: str, method: Any, idl_definition: Any) -> str:
        """
        Generate method stub code for a class method.
        
        Args:
            class_name: Name of the class
            method: MethodDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python method code string
        """
        # Build parameter list (skip first if instance_required)
        params = []
        start_idx = 1 if method.instance_required else 0
        
        for i, param in enumerate(method.parameters[start_idx:], start=start_idx):
            type_ann = type_mapper.metaffi_type_to_python_type_annotation(
                param.type, param.dimensions, param.type_alias
            )
            param_str = f"{param.name}: {type_ann}"
            if param.is_optional:
                param_str += " = None"
            params.append(param_str)
        
        params_str = ", ".join(params) if params else ""
        
        # Build return type
        if method.return_values:
            if len(method.return_values) == 1:
                ret_type = type_mapper.metaffi_type_to_python_type_annotation(
                    method.return_values[0].type,
                    method.return_values[0].dimensions,
                    method.return_values[0].type_alias
                )
            else:
                ret_types = [
                    type_mapper.metaffi_type_to_python_type_annotation(
                        rv.type, rv.dimensions, rv.type_alias
                    )
                    for rv in method.return_values
                ]
                ret_type = f"Tuple[{', '.join(ret_types)}]"
        else:
            ret_type = "None"
        
        # Build method body
        param_names = ["self._handle"] if method.instance_required else []
        param_names.extend([p.name for p in method.parameters[1:] if method.instance_required] or [p.name for p in method.parameters])
        param_names_str = ", ".join(param_names) if param_names else ""
        
        caller_name = f"{class_name}_{method.name}_caller"
        
        lines = [
            f"    def {method.name}(self, {params_str}) -> {ret_type}:",
            f'        """',
        ]
        
        if method.comment:
            comment = method.comment.replace('"""', '\\"\\"\\"')
            lines.append(f"        {comment}")
        else:
            lines.append(f"        Generated stub method for {method.name}")
        
        lines.append(f'        """')
        lines.append(f"        # Use cached MetaFFIEntity to make the call")
        lines.append(f"        result = {caller_name}({param_names_str})")
        
        # Unpack return values
        if method.return_values:
            if len(method.return_values) == 1:
                lines.append("        return result")
            else:
                lines.append("        return result")
        else:
            lines.append("        return None")
        
        return "\n".join(lines)
    
    def _generate_field_property(self, class_name: str, field: Any, idl_definition: Any) -> str:
        """
        Generate @property getter and setter for a field.
        
        Args:
            class_name: Name of the class
            field: FieldDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python property code string
        """
        type_ann = type_mapper.metaffi_type_to_python_type_annotation(
            field.type, field.dimensions, field.type_alias
        )
        
        lines = []
        
        if field.getter:
            getter_caller = f"{class_name}_{field.name}_getter_caller"
            lines.append(f"    @property")
            lines.append(f"    def {field.name}(self) -> {type_ann}:")
            lines.append(f"        result = {getter_caller}(self._handle)")
            lines.append("        return result[0] if isinstance(result, tuple) and len(result) == 1 else result")
            lines.append("")
        
        if field.setter:
            setter_caller = f"{class_name}_{field.name}_setter_caller"
            lines.append(f"    @{field.name}.setter")
            lines.append(f"    def {field.name}(self, value: {type_ann}) -> None:")
            lines.append(f"        {setter_caller}(self._handle, value)")
        
        return "\n".join(lines)
    
    def _generate_constructor_function(self, cls: Any, ctor: Any, idl_definition: Any) -> str:
        """
        Generate module-level constructor function.
        
        Args:
            cls: ClassDefinition containing the constructor
            ctor: ConstructorDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python constructor function code string
        """
        # Build parameter list
        params = []
        for param in ctor.parameters:
            type_ann = type_mapper.metaffi_type_to_python_type_annotation(
                param.type, param.dimensions, param.type_alias
            )
            param_str = f"{param.name}: {type_ann}"
            if param.is_optional:
                param_str += " = None"
            params.append(param_str)
        
        params_str = ", ".join(params) if params else ""
        
        # Build function body
        param_names = [p.name for p in ctor.parameters]
        param_names_str = ", ".join(param_names) if param_names else ""
        
        caller_name = f"{cls.name}_{ctor.name}_caller"
        
        lines = [
            f"def {cls.name.lower()}({params_str}) -> {cls.name}:",
            f'    """',
        ]
        
        if ctor.comment:
            comment = ctor.comment.replace('"""', '\\"\\"\\"')
            lines.append(f"    {comment}")
        else:
            lines.append(f"    Generated constructor for {cls.name}")
        
        lines.append(f'    """')
        lines.append(f"    result = {caller_name}({param_names_str})")
        lines.append(f"    return {cls.name}(result[0] if isinstance(result, tuple) and len(result) == 1 else result)")
        
        return "\n".join(lines)
    
    def _generate_global_accessors(self, global_def: Any, idl_definition: Any) -> str:
        """
        Generate getter and setter functions for a global variable.
        
        Args:
            global_def: GlobalDefinition to generate code for
            idl_definition: IDLDefinition (for type mapping)
            
        Returns:
            Python getter/setter function code string
        """
        type_ann = type_mapper.metaffi_type_to_python_type_annotation(
            global_def.type, global_def.dimensions, global_def.type_alias
        )
        
        lines = []
        
        if global_def.getter:
            getter_caller = f"{global_def.name}_getter_caller"
            lines.append(f"def get_{global_def.name}() -> {type_ann}:")
            lines.append(f"    result = {getter_caller}()")
            lines.append("    return result[0] if isinstance(result, tuple) and len(result) == 1 else result")
            lines.append("")
        
        if global_def.setter:
            setter_caller = f"{global_def.name}_setter_caller"
            lines.append(f"def set_{global_def.name}(value: {type_ann}) -> None:")
            lines.append(f"    {setter_caller}(value)")
        
        return "\n".join(lines)


def compile_idl_json(
    idl_json_path: str,
    output_dir: str,
    output_filename: str,
    context: CompilerContext,
    host_options: Optional[Dict[str, str]] = None
) -> None:
    """
    Convenience function to compile IDL JSON file to Python host code.
    
    Args:
        idl_json_path: Path to IDL JSON file
        output_dir: Directory to write output files
        output_filename: Base filename for output (without extension)
        context: CompilerContext for dependency injection (required)
        host_options: Optional compiler options
    """
    if context is None:
        raise ValueError("CompilerContext is required and cannot be None")
    
    # Load IDL JSON
    with open(idl_json_path, "r", encoding="utf-8") as f:
        idl_data = json.load(f)
    
    # Parse IDL definition
    IDLDefinition = context.idl_entities.IDLDefinition
    definition = IDLDefinition.from_dict(idl_data)
    
    # Compile
    compiler = HostCompiler(context)
    compiler.compile(definition, output_dir, output_filename, host_options)
