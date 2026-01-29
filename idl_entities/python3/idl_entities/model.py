from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Any
import json
import os

METAFFI_TYPES = [
    "float64",
    "float32",
    "int8",
    "int16",
    "int32",
    "int64",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
    "bool",
    "char8",
    "char16",
    "char32",
    "string8",
    "string16",
    "string32",
    "handle",
    "array",
    "any",
    "size",
    "null",
    "float64_array",
    "float32_array",
    "int8_array",
    "int16_array",
    "int32_array",
    "int64_array",
    "uint8_array",
    "uint16_array",
    "uint32_array",
    "uint64_array",
    "bool_array",
    "char8_array",
    "char16_array",
    "char32_array",
    "string8_array",
    "string16_array",
    "string32_array",
    "handle_array",
    "any_array",
    "size_array",
]


def _copy_dict(value: Optional[Dict[str, str]]) -> Dict[str, str]:
    return dict(value) if value else {}


def _parse_list(data: Optional[List[Any]], cls):
    if not data:
        return []
    return [cls.from_dict(item) for item in data]


@dataclass
class ArgDefinition:
    name: str
    type: str
    type_alias: str
    comment: str = ""
    tags: Dict[str, str] = field(default_factory=dict)
    dimensions: int = 0
    is_optional: bool = False

    def to_dict(self) -> Dict[str, Any]:
        data = {
            "name": self.name,
            "type": self.type,
            "type_alias": self.type_alias,
            "comment": self.comment,
            "tags": _copy_dict(self.tags),
            "dimensions": self.dimensions,
        }
        if self.is_optional:
            data["is_optional"] = True
        return data

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ArgDefinition":
        return cls(
            name=data.get("name", ""),
            type=data.get("type", ""),
            type_alias=data.get("type_alias", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            dimensions=int(data.get("dimensions", 0)),
            is_optional=bool(data.get("is_optional", False)),
        )


@dataclass
class FunctionDefinition:
    name: str
    comment: str = ""
    tags: Dict[str, str] = field(default_factory=dict)
    entity_path: Dict[str, str] = field(default_factory=dict)
    parameters: List[ArgDefinition] = field(default_factory=list)
    return_values: List[ArgDefinition] = field(default_factory=list)
    overload_index: int = 0

    def entity_path_as_string(self, idl_definition: "IDLDefinition") -> str:
        """
        Convert entity_path dict to comma-separated string format.
        Similar to Go's EntityPathAsString().
        
        Args:
            idl_definition: The IDLDefinition containing metaffi_guest_lib
            
        Returns:
            String in format "key1=value1,key2=value2"
        """
        keys = set(self.entity_path.keys())
        keys.add("metaffi_guest_lib")
        
        sorted_keys = sorted(keys)
        parts = []
        
        for k in sorted_keys:
            v = self.entity_path.get(k)
            if v is None:
                if k == "metaffi_guest_lib":
                    v = idl_definition.metaffi_guest_lib
                else:
                    raise ValueError(f"Unexpected key {k}")
            
            parts.append(f"{k}={v}")
        
        return ",".join(parts)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "comment": self.comment,
            "tags": _copy_dict(self.tags),
            "entity_path": _copy_dict(self.entity_path),
            "parameters": [p.to_dict() for p in self.parameters],
            "return_values": [r.to_dict() for r in self.return_values],
            "overload_index": self.overload_index,
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "FunctionDefinition":
        return cls(
            name=data.get("name", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            entity_path=_copy_dict(data.get("entity_path")),
            parameters=_parse_list(data.get("parameters"), ArgDefinition),
            return_values=_parse_list(data.get("return_values"), ArgDefinition),
            overload_index=int(data.get("overload_index", 0)),
        )


@dataclass
class MethodDefinition(FunctionDefinition):
    instance_required: bool = True

    def entity_path_as_string(self, idl_definition: "IDLDefinition", parent_entity_path: Optional[Dict[str, str]] = None) -> str:
        """
        Convert entity_path dict to comma-separated string format.
        Also checks parent class entity_path if provided.
        
        Args:
            idl_definition: The IDLDefinition containing metaffi_guest_lib
            parent_entity_path: Optional parent class entity_path to merge
        """
        keys = set(self.entity_path.keys())
        if parent_entity_path:
            keys.update(parent_entity_path.keys())
        keys.add("metaffi_guest_lib")
        
        sorted_keys = sorted(keys)
        parts = []
        
        for k in sorted_keys:
            v = self.entity_path.get(k)
            if v is None:
                # Check parent class entity_path
                if parent_entity_path:
                    v = parent_entity_path.get(k)
                if v is None:
                    if k == "metaffi_guest_lib":
                        v = idl_definition.metaffi_guest_lib
                    else:
                        raise ValueError(f"Unexpected key {k}")
            
            parts.append(f"{k}={v}")
        
        return ",".join(parts)

    def to_dict(self) -> Dict[str, Any]:
        data = super().to_dict()
        data["instance_required"] = bool(self.instance_required)
        return data

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "MethodDefinition":
        base = FunctionDefinition.from_dict(data)
        return cls(
            name=base.name,
            comment=base.comment,
            tags=base.tags,
            entity_path=base.entity_path,
            parameters=base.parameters,
            return_values=base.return_values,
            overload_index=base.overload_index,
            instance_required=bool(data.get("instance_required", True)),
        )


@dataclass
class ConstructorDefinition(FunctionDefinition):
    # Inherits entity_path_as_string from FunctionDefinition
    pass


@dataclass
class ReleaseDefinition(MethodDefinition):
    pass


@dataclass
class FieldDefinition(ArgDefinition):
    getter: Optional[MethodDefinition] = None
    setter: Optional[MethodDefinition] = None

    def entity_path_as_string(self, idl_definition: "IDLDefinition", use_getter: bool = True) -> str:
        """
        Convert entity_path dict to comma-separated string format.
        Uses getter's or setter's entity_path_as_string.
        
        Args:
            idl_definition: The IDLDefinition containing metaffi_guest_lib
            use_getter: If True, use getter's entity_path; if False, use setter's
            
        Returns:
            String in format "key1=value1,key2=value2"
        """
        if use_getter and self.getter:
            return self.getter.entity_path_as_string(idl_definition)
        elif not use_getter and self.setter:
            return self.setter.entity_path_as_string(idl_definition)
        else:
            raise ValueError(f"Field {self.name} does not have {'getter' if use_getter else 'setter'}")

    def to_dict(self) -> Dict[str, Any]:
        data = super().to_dict()
        data["getter"] = self.getter.to_dict() if self.getter else None
        data["setter"] = self.setter.to_dict() if self.setter else None
        return data

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "FieldDefinition":
        getter = data.get("getter")
        setter = data.get("setter")
        return cls(
            name=data.get("name", ""),
            type=data.get("type", ""),
            type_alias=data.get("type_alias", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            dimensions=int(data.get("dimensions", 0)),
            is_optional=bool(data.get("is_optional", False)),
            getter=MethodDefinition.from_dict(getter) if getter else None,
            setter=MethodDefinition.from_dict(setter) if setter else None,
        )


@dataclass
class GlobalDefinition(ArgDefinition):
    getter: Optional[FunctionDefinition] = None
    setter: Optional[FunctionDefinition] = None

    def entity_path_as_string(self, idl_definition: "IDLDefinition", use_getter: bool = True) -> str:
        """
        Convert entity_path dict to comma-separated string format.
        Uses getter's or setter's entity_path_as_string.
        
        Args:
            idl_definition: The IDLDefinition containing metaffi_guest_lib
            use_getter: If True, use getter's entity_path; if False, use setter's
            
        Returns:
            String in format "key1=value1,key2=value2"
        """
        if use_getter and self.getter:
            return self.getter.entity_path_as_string(idl_definition)
        elif not use_getter and self.setter:
            return self.setter.entity_path_as_string(idl_definition)
        else:
            raise ValueError(f"Global {self.name} does not have {'getter' if use_getter else 'setter'}")

    def to_dict(self) -> Dict[str, Any]:
        data = super().to_dict()
        data["getter"] = self.getter.to_dict() if self.getter else None
        data["setter"] = self.setter.to_dict() if self.setter else None
        return data

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "GlobalDefinition":
        getter = data.get("getter")
        setter = data.get("setter")
        return cls(
            name=data.get("name", ""),
            type=data.get("type", ""),
            type_alias=data.get("type_alias", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            dimensions=int(data.get("dimensions", 0)),
            is_optional=bool(data.get("is_optional", False)),
            getter=FunctionDefinition.from_dict(getter) if getter else None,
            setter=FunctionDefinition.from_dict(setter) if setter else None,
        )


@dataclass
class ClassDefinition:
    name: str
    comment: str = ""
    tags: Dict[str, str] = field(default_factory=dict)
    entity_path: Dict[str, str] = field(default_factory=dict)
    constructors: List[ConstructorDefinition] = field(default_factory=list)
    release: Optional[ReleaseDefinition] = None
    methods: List[MethodDefinition] = field(default_factory=list)
    fields: List[FieldDefinition] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "comment": self.comment,
            "tags": _copy_dict(self.tags),
            "entity_path": _copy_dict(self.entity_path),
            "constructors": [c.to_dict() for c in self.constructors],
            "release": self.release.to_dict() if self.release else None,
            "methods": [m.to_dict() for m in self.methods],
            "fields": [f.to_dict() for f in self.fields],
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ClassDefinition":
        return cls(
            name=data.get("name", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            entity_path=_copy_dict(data.get("entity_path")),
            constructors=_parse_list(data.get("constructors"), ConstructorDefinition),
            release=ReleaseDefinition.from_dict(data["release"]) if data.get("release") else None,
            methods=_parse_list(data.get("methods"), MethodDefinition),
            fields=_parse_list(data.get("fields"), FieldDefinition),
        )


@dataclass
class ModuleDefinition:
    name: str
    comment: str = ""
    tags: Dict[str, str] = field(default_factory=dict)
    functions: List[FunctionDefinition] = field(default_factory=list)
    classes: List[ClassDefinition] = field(default_factory=list)
    globals: List[GlobalDefinition] = field(default_factory=list)
    external_resources: List[str] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "comment": self.comment,
            "tags": _copy_dict(self.tags),
            "functions": [f.to_dict() for f in self.functions],
            "classes": [c.to_dict() for c in self.classes],
            "globals": [g.to_dict() for g in self.globals],
            "external_resources": list(self.external_resources),
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ModuleDefinition":
        return cls(
            name=data.get("name", ""),
            comment=data.get("comment", ""),
            tags=_copy_dict(data.get("tags")),
            functions=_parse_list(data.get("functions"), FunctionDefinition),
            classes=_parse_list(data.get("classes"), ClassDefinition),
            globals=_parse_list(data.get("globals"), GlobalDefinition),
            external_resources=list(data.get("external_resources", [])),
        )


@dataclass
class IDLDefinition:
    idl_source: str
    idl_extension: str
    idl_filename_with_extension: str
    idl_full_path: str
    metaffi_guest_lib: str
    target_language: str
    modules: List[ModuleDefinition] = field(default_factory=list)

    def finalize_construction(self) -> None:
        for module in self.modules:
            module.external_resources = [os.path.expandvars(p) for p in module.external_resources]

    def to_dict(self) -> Dict[str, Any]:
        return {
            "idl_source": self.idl_source,
            "idl_extension": self.idl_extension,
            "idl_filename_with_extension": self.idl_filename_with_extension,
            "idl_full_path": self.idl_full_path,
            "metaffi_guest_lib": self.metaffi_guest_lib,
            "target_language": self.target_language,
            "modules": [m.to_dict() for m in self.modules],
        }

    def to_json(self, pretty: bool = True) -> str:
        if pretty:
            return json.dumps(self.to_dict(), indent=2)
        return json.dumps(self.to_dict())

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "IDLDefinition":
        instance = cls(
            idl_source=data.get("idl_source", ""),
            idl_extension=data.get("idl_extension", ""),
            idl_filename_with_extension=data.get("idl_filename_with_extension", ""),
            idl_full_path=data.get("idl_full_path", ""),
            metaffi_guest_lib=data.get("metaffi_guest_lib", ""),
            target_language=data.get("target_language", ""),
            modules=_parse_list(data.get("modules"), ModuleDefinition),
        )
        instance.finalize_construction()
        return instance

    @classmethod
    def from_json(cls, data: str) -> "IDLDefinition":
        return cls.from_dict(json.loads(data))
