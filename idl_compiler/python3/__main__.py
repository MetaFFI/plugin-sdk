"""
MetaFFI Python3 IDL Compiler - Command Line Interface

IMPORTANT: This CLI tool is for DEVELOPMENT AND TESTING ONLY.

The primary use case for this package is as a library imported by the C++ IDL
plugin bridge in lang-plugin-python311/idl/python_idl_plugin.cpp.

For production use, the C++ bridge imports this package directly and calls
PythonExtractor and IDLGenerator classes programmatically, not via this CLI.

Usage (development/testing):
    python -m sdk.idl_compiler.python3 <source_path> [--output OUTPUT_FILE]

Arguments:
    source_path    Path to Python file, module name, or package directory

Options:
    --output, -o   Output file path (default: stdout)
    --validate     Validate generated JSON against schema (requires jsonschema)
    --help, -h     Show this help message
"""

import argparse
import sys
from pathlib import Path

from .extractor import PythonExtractor
from .idl_generator import IDLGenerator


def main():
    """Main entry point for CLI"""
    parser = argparse.ArgumentParser(
        description="MetaFFI Python3 IDL Compiler - Extract interface definitions from Python modules",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument(
        "source_path",
        help="Path to Python file, module name, or package directory"
    )

    parser.add_argument(
        "-o", "--output",
        help="Output file path (default: stdout)",
        default=None
    )

    parser.add_argument(
        "--validate",
        action="store_true",
        help="Validate generated JSON against schema (requires jsonschema)"
    )

    args = parser.parse_args()

    try:
        # Extract module information
        print(f"Extracting from: {args.source_path}", file=sys.stderr)
        extractor = PythonExtractor(args.source_path)
        module_info = extractor.extract()

        print(f"Found:", file=sys.stderr)
        print(f"  - {len(module_info.functions)} functions", file=sys.stderr)
        print(f"  - {len(module_info.classes)} classes", file=sys.stderr)
        print(f"  - {len(module_info.globals)} global variables", file=sys.stderr)

        # Generate IDL JSON
        print("Generating IDL JSON...", file=sys.stderr)
        generator = IDLGenerator(args.source_path, module_info)
        idl_json = generator.generate_json()

        # Validate if requested
        if args.validate:
            print("Validating JSON...", file=sys.stderr)
            try:
                import jsonschema
                import json

                # Load schema
                schema_path = Path(__file__).parent.parent.parent / "compiler" / "go" / "IDL" / "schema.json"
                with open(schema_path, 'r') as f:
                    schema = json.load(f)

                # Validate
                idl_dict = json.loads(idl_json)
                jsonschema.validate(idl_dict, schema)
                print("✓ Validation passed", file=sys.stderr)
            except ImportError:
                print("Warning: jsonschema not installed, skipping validation", file=sys.stderr)
            except Exception as e:
                print(f"✗ Validation failed: {e}", file=sys.stderr)
                sys.exit(1)

        # Output JSON
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(idl_json)
            print(f"✓ IDL written to: {args.output}", file=sys.stderr)
        else:
            print(idl_json)

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc(file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
