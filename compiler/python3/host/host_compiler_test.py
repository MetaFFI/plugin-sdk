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


if __name__ == "__main__":
    unittest.main()
