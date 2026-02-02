"""
Test suite for Python host compiler.
"""

import unittest
import json
import os
import tempfile
import shutil
from pathlib import Path
from .compiler_context import CompilerContext
from .host_compiler import HostCompiler, compile_idl_json

# Create test context with explicit imports for testing
# Fail-fast: if imports don't work, the test should fail immediately
import sys
import importlib.util
sdk_root = os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")
if sdk_root not in sys.path:
    sys.path.insert(0, sdk_root)
from sdk.idl_entities.python3.idl_entities import model as idl_entities_model
# Import metaffi_types directly to avoid triggering __init__.py which imports installed metaffi package
metaffi_types_path = os.path.join(sdk_root, "sdk", "api", "python3", "metaffi", "metaffi_types.py")
spec = importlib.util.spec_from_file_location("metaffi_types", metaffi_types_path)
metaffi_types = importlib.util.module_from_spec(spec)
spec.loader.exec_module(metaffi_types)
test_context = CompilerContext(
    idl_entities_module=idl_entities_model,
    metaffi_types_module=metaffi_types
)


class TestHostCompiler(unittest.TestCase):
    """Test cases for Python host compiler."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.compiler = HostCompiler(test_context)
        self.test_output_dir = None
    
    def tearDown(self):
        """Clean up test fixtures."""
        if self.test_output_dir and os.path.exists(self.test_output_dir):
            shutil.rmtree(self.test_output_dir, ignore_errors=True)
    
    def test_simple_function(self):
        """Test compilation of IDL with a simple function."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_MetaFFIGuest",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [
                        {
                            "name": "add",
                            "comment": "Add two numbers",
                            "tags": {},
                            "entity_path": {"callable": "add"},
                            "parameters": [
                                {"name": "a", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False},
                                {"name": "b", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                            ],
                            "return_values": [
                                {"name": "result", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                            ],
                            "overload_index": 0
                        }
                    ],
                    "classes": [],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }
        
        # Get IDLDefinition from context
        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)
        
        with tempfile.TemporaryDirectory() as tmpdir:
            self.compiler.compile(definition, tmpdir, "test_output", {})
            
            # Check that output file was created
            output_file = Path(tmpdir) / "test_module" / "test_output_MetaFFIHost.py"
            self.assertTrue(output_file.exists(), "Output file should be created")
            
            # Check that file contains expected content
            content = output_file.read_text(encoding="utf-8")
            self.assertIn("def add", content, "Should contain function definition")
            self.assertIn("bind_module_to_code", content, "Should contain module initialization")
            self.assertIn("_module.load_entity", content, "Should contain entity loading")
    
    def test_class_with_methods_and_fields(self):
        """Test compilation of IDL with a class containing methods and fields."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_MetaFFIGuest",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [],
                    "classes": [
                        {
                            "name": "MyClass",
                            "comment": "Test class",
                            "tags": {},
                            "entity_path": {"class": "MyClass"},
                            "constructors": [
                                {
                                    "name": "MyClass",
                                    "comment": "",
                                    "tags": {},
                                    "entity_path": {"callable": "MyClass"},
                                    "parameters": [
                                        {"name": "value", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                    ],
                                    "return_values": [
                                        {"name": "instance", "type": "handle", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                    ],
                                    "overload_index": 0
                                }
                            ],
                            "release": {
                                "name": "ReleaseMyClass",
                                "comment": "",
                                "tags": {},
                                "entity_path": {"callable": "ReleaseMyClass"},
                                "parameters": [
                                    {"name": "this_instance", "type": "handle", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                ],
                                "return_values": [],
                                "instance_required": True
                            },
                            "methods": [
                                {
                                    "name": "get_value",
                                    "comment": "Get value",
                                    "tags": {},
                                    "entity_path": {"callable": "get_value"},
                                    "parameters": [
                                        {"name": "this_instance", "type": "handle", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                    ],
                                    "return_values": [
                                        {"name": "value", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                    ],
                                    "instance_required": True
                                }
                            ],
                            "fields": [
                                {
                                    "name": "name",
                                    "type": "string8",
                                    "type_alias": "",
                                    "comment": "",
                                    "tags": {},
                                    "dimensions": 0,
                                    "is_optional": False,
                                    "getter": {
                                        "name": "get_name",
                                        "comment": "",
                                        "tags": {},
                                        "entity_path": {"callable": "get_name"},
                                        "parameters": [
                                            {"name": "this_instance", "type": "handle", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                        ],
                                        "return_values": [
                                            {"name": "name", "type": "string8", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                                        ],
                                        "instance_required": True
                                    },
                                    "setter": None
                                }
                            ]
                        }
                    ],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }
        
        # Get IDLDefinition from context
        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)
        
        with tempfile.TemporaryDirectory() as tmpdir:
            self.compiler.compile(definition, tmpdir, "test_output", {})
            
            output_file = Path(tmpdir) / "test_module" / "test_output_MetaFFIHost.py"
            self.assertTrue(output_file.exists())
            
            content = output_file.read_text(encoding="utf-8")
            self.assertIn("class MyClass", content, "Should contain class definition")
            self.assertIn("@property", content, "Should contain property decorator")
            self.assertIn("def get_value", content, "Should contain method definition")
            self.assertIn("def myclass", content, "Should contain constructor function")
    
    def test_optional_parameters(self):
        """Test compilation with optional parameters."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_MetaFFIGuest",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [
                        {
                            "name": "func_with_optional",
                            "comment": "",
                            "tags": {},
                            "entity_path": {"callable": "func_with_optional"},
                            "parameters": [
                                {"name": "required", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False},
                                {"name": "optional", "type": "int32", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": True}
                            ],
                            "return_values": [],
                            "overload_index": 0
                        }
                    ],
                    "classes": [],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }
        
        # Get IDLDefinition from context
        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)
        
        with tempfile.TemporaryDirectory() as tmpdir:
            self.compiler.compile(definition, tmpdir, "test_output", {})
            
            output_file = Path(tmpdir) / "test_module" / "test_output_MetaFFIHost.py"
            content = output_file.read_text(encoding="utf-8")
            self.assertIn("optional: int = None", content, "Should have optional parameter with default")
    
    def test_validation_errors(self):
        """Test that validation fails fast on invalid IDL."""
        # Missing target_language
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_MetaFFIGuest",
            "target_language": "",
            "modules": []
        }
        
        # Get IDLDefinition from context
        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)
        
        with self.assertRaises(ValueError) as cm:
            self.compiler.compile(definition, "/tmp", "test", {})
        
        self.assertIn("target_language", str(cm.exception))
    
    def test_compile_idl_json_function(self):
        """Test the convenience compile_idl_json function."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_MetaFFIGuest",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [
                        {
                            "name": "test_func",
                            "comment": "",
                            "tags": {},
                            "entity_path": {"callable": "test_func"},
                            "parameters": [],
                            "return_values": [],
                            "overload_index": 0
                        }
                    ],
                    "classes": [],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
            json.dump(idl_data, f)
            idl_path = f.name
        
        try:
            with tempfile.TemporaryDirectory() as tmpdir:
                compile_idl_json(idl_path, tmpdir, "test_output", test_context, {})
                
                output_file = Path(tmpdir) / "test_module" / "test_output_MetaFFIHost.py"
                self.assertTrue(output_file.exists())
        finally:
            os.unlink(idl_path)

    def test_type_mapping_primitives(self):
        """Test MetaFFI → Python primitive type mapping."""
        from sdk.idl_entities.python3 import type_mapper

        # Integer types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int8", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int16", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int32", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int64", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("uint8", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("uint16", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("uint32", 0, ""), "int")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("uint64", 0, ""), "int")

        # Float types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("float32", 0, ""), "float")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("float64", 0, ""), "float")

        # String types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("string8", 0, ""), "str")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("string16", 0, ""), "str")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("string32", 0, ""), "str")

        # Character types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("char8", 0, ""), "str")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("char16", 0, ""), "str")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("char32", 0, ""), "str")

        # Other types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("bool", 0, ""), "bool")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("null", 0, ""), "None")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("size", 0, ""), "int")

    def test_type_mapping_arrays(self):
        """Test MetaFFI → Python array type mapping."""
        from sdk.idl_entities.python3 import type_mapper

        # 1D arrays with dimensions parameter
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int64", 1, ""), "List[int]")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("string8", 1, ""), "List[str]")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("float64", 1, ""), "List[float]")

        # 1D arrays with _array suffix
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int64_array", 0, ""), "List[int]")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("string8_array", 0, ""), "List[str]")

        # 2D arrays
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int64", 2, ""), "List[List[int]]")

        # 3D arrays
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("int64", 3, ""), "List[List[List[int]]]")

        # Special case: uint8_array should still map to List[int], not bytes
        # (bytes is handled at runtime, not in type annotations)
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("uint8_array", 0, ""), "List[int]")

    def test_type_mapping_special_types(self):
        """Test MetaFFI → Python special type mapping."""
        from sdk.idl_entities.python3 import type_mapper

        # Handle types
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("handle", 0, ""), "Any")
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("handle", 0, "MyClass"), "MyClass")

        # Handle arrays
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("handle_array", 0, ""), "List[Any]")

        # Callable
        # Note: callable might not be in the type_mapping dict, so it might return "Any"
        result = type_mapper.metaffi_type_to_python_type_annotation("callable", 0, "")
        self.assertTrue(result in ["Any", "callable"], f"Expected 'Any' or 'callable', got '{result}'")

        # Any type
        self.assertEqual(type_mapper.metaffi_type_to_python_type_annotation("any", 0, ""), "Any")

    def test_arg_to_metaffi_type_info(self):
        """Test ArgDefinition → metaffi_type_info conversion."""
        from sdk.idl_entities.python3 import type_mapper

        # Create test argument
        arg = idl_entities_model.ArgDefinition(
            name="test",
            type="int64",
            type_alias="int64_t",
            comment="",
            tags={},
            dimensions=0,
            is_optional=False
        )

        # Convert to type_info
        type_info = type_mapper.arg_to_metaffi_type_info(arg, metaffi_types)

        # Verify type
        self.assertEqual(type_info.type, metaffi_types.MetaFFITypes.metaffi_int64_type)
        self.assertEqual(type_info.fixed_dimensions, 0)

    def test_metaffi_type_info_array(self):
        """Test metaffi_type_info creation for array types."""
        from sdk.idl_entities.python3 import type_mapper

        # Create array argument
        arg = idl_entities_model.ArgDefinition(
            name="numbers",
            type="int64_array",
            type_alias="",
            comment="",
            tags={},
            dimensions=1,
            is_optional=False
        )

        # Convert to type_info
        type_info = type_mapper.arg_to_metaffi_type_info(arg, metaffi_types)

        # Verify it includes array flag
        expected_type = metaffi_types.MetaFFITypes.metaffi_int64_type | metaffi_types.MetaFFITypes.metaffi_array_type
        self.assertEqual(type_info.type, expected_type)
        self.assertEqual(type_info.fixed_dimensions, 1)

    def test_environment_variable_handling_in_generated_code(self):
        """Test that generated code includes environment variable handling."""
        header = self.compiler._generate_header("test.idl.json")

        # Verify environment variable checks are present
        self.assertIn("METAFFI_SOURCE_ROOT", header)
        self.assertIn("METAFFI_HOME", header)
        self.assertIn("raise RuntimeError", header)
        self.assertIn("sdk_path", header)
        self.assertIn("sys.path.insert", header)

        # Verify it's checking SOURCE_ROOT first, then HOME
        source_root_pos = header.find("METAFFI_SOURCE_ROOT")
        home_pos = header.find("METAFFI_HOME")
        self.assertLess(source_root_pos, home_pos, "Should check METAFFI_SOURCE_ROOT before METAFFI_HOME")

    def test_validation_missing_guest_lib(self):
        """Test fail-fast validation for missing metaffi_guest_lib."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "",  # Empty guest lib
            "target_language": "python3",
            "modules": []
        }

        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)

        with self.assertRaises(ValueError) as cm:
            self.compiler.compile(definition, "/tmp", "test", {})

        self.assertIn("metaffi_guest_lib", str(cm.exception))

    def test_validation_missing_entity_path(self):
        """Test fail-fast validation for missing entity_path in function."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_lib",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [
                        {
                            "name": "bad_func",
                            "comment": "",
                            "tags": {},
                            "entity_path": {},  # Empty entity path
                            "parameters": [],
                            "return_values": [],
                            "overload_index": 0
                        }
                    ],
                    "classes": [],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }

        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)

        with self.assertRaises(ValueError) as cm:
            self.compiler.compile(definition, "/tmp", "test", {})

        self.assertIn("entity_path", str(cm.exception))

    def test_multiple_return_values(self):
        """Test function with multiple return values generates Tuple type annotation."""
        idl_data = {
            "idl_source": "test",
            "idl_extension": ".json",
            "idl_filename_with_extension": "test.json",
            "idl_full_path": "test.json",
            "metaffi_guest_lib": "test_lib",
            "target_language": "python3",
            "modules": [
                {
                    "name": "test_module",
                    "comment": "",
                    "tags": {},
                    "functions": [
                        {
                            "name": "return_two",
                            "comment": "",
                            "tags": {},
                            "entity_path": {"callable": "return_two"},
                            "parameters": [],
                            "return_values": [
                                {"name": "a", "type": "int64", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False},
                                {"name": "b", "type": "string8", "type_alias": "", "comment": "", "tags": {}, "dimensions": 0, "is_optional": False}
                            ],
                            "overload_index": 0
                        }
                    ],
                    "classes": [],
                    "globals": [],
                    "external_resources": []
                }
            ]
        }

        IDLDefinition = test_context.idl_entities.IDLDefinition
        definition = IDLDefinition.from_dict(idl_data)

        with tempfile.TemporaryDirectory() as tmpdir:
            self.compiler.compile(definition, tmpdir, "test_output", {})

            output_file = Path(tmpdir) / "test_module" / "test_output_MetaFFIHost.py"
            content = output_file.read_text(encoding="utf-8")

            # Should have Tuple return type
            self.assertIn("Tuple[int, str]", content)


if __name__ == "__main__":
    unittest.main()
