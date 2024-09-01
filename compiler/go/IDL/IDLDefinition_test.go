package IDL

import (
	"testing"
)

// Creates IDL definition for sanity test (specifically, with Go's function names)

// --------------------------------------------------------------------
func TestIDL(t *testing.T) {

	idl := NewIDLDefinition("test.json", "testlang")
	idl.AddModule(createModule())

	idl.FinalizeConstruction()

	_, err := idl.ToJSON()
	if err != nil {
		t.Fatal(err)
	}

}

// --------------------------------------------------------------------
func createModule() *ModuleDefinition {

	module := NewModuleDefinition("Sanity")

	module.AddFunction(createHelloWorld())
	module.AddFunction(createReturnsAnError())
	module.AddFunction(createDivIntegers())
	module.AddFunction(createJoinStrings())

	module.AddFunction(createWaitABit())
	module.AddExternalResource("time")

	module.AddClass(createTestMap())

	module.AddGlobal(create5Seconds())

	module.SetEntityPath("package", "TestFuncs")
	module.SetEntityPath("module", "$PWD")

	return module
}

// --------------------------------------------------------------------
func createHelloWorld() *FunctionDefinition {
	funcDef := NewFunctionDefinition("HelloWorld")
	funcDef.AppendComment("prints hello_world")
	return funcDef
}

// --------------------------------------------------------------------
func createReturnsAnError() *FunctionDefinition {
	funcDef := NewFunctionDefinition("ReturnsAnError")
	funcDef.AppendComment("always returns an error")
	return funcDef
}

// --------------------------------------------------------------------
func createDivIntegers() *FunctionDefinition {
	funcDef := NewFunctionDefinition("DivIntegers")
	funcDef.AppendComment("divides two integers")

	funcDef.AddParameter(NewArgDefinitionWithAlias("x", INT64, "int"))
	funcDef.AddParameter(NewArgDefinitionWithAlias("y", INT64, "int"))
	funcDef.AddReturnValues(NewArgDefinition("z", FLOAT32))

	return funcDef
}

// --------------------------------------------------------------------
func createJoinStrings() *FunctionDefinition {
	funcDef := NewFunctionDefinition("JoinStrings")
	funcDef.AppendComment("joins the strings using a comma separator")

	funcDef.AddParameter(NewArgArrayDefinition("strings", STRING8, 1))
	funcDef.AddReturnValues(NewArgDefinition("result", STRING8))

	return funcDef
}

// --------------------------------------------------------------------
func createWaitABit() *FunctionDefinition {
	funcDef := NewFunctionDefinition("WaitABit")
	funcDef.AppendComment("Wait a bit")

	funcDef.AddParameter(NewArgDefinitionWithAlias("d", INT64, "time.Duration"))
	funcDef.AddReturnValues(NewArgDefinition("err", HANDLE))

	return funcDef
}

// --------------------------------------------------------------------
func createTestMap() *ClassDefinition {
	cls := NewClassDefinition("TestMap")
	cls.AppendComment("Go map wrapper")

	cls.AddConstructor(NewConstructorDefinition("NewTestMap"))

	f := NewFieldDefinition(cls, "Name", STRING8, "GetName", "SetName", true)
	f.SetTag("receiver_pointer", "true")
	cls.AddField(f)

	setKey := NewMethodDefinition(cls, "Set", true)
	setKey.AddParameter(NewArgDefinition("key", STRING8))
	setKey.AddParameter(NewArgDefinition("value", ANY))
	setKey.SetTag("receiver_pointer", "true")
	cls.AddMethod(setKey)

	getKey := NewMethodDefinition(cls, "Get", true)
	getKey.AddParameter(NewArgDefinition("key", STRING8))
	getKey.AddReturnValues(NewArgDefinition("value", ANY))
	getKey.SetTag("receiver_pointer", "true")
	cls.AddMethod(getKey)

	containsKey := NewMethodDefinition(cls, "Contains", true)
	containsKey.AddParameter(NewArgDefinition("key", STRING8))
	containsKey.AddReturnValues(NewArgDefinition("exists", BOOL))
	containsKey.SetTag("receiver_pointer", "true")
	cls.AddMethod(containsKey)

	return cls
}

// --------------------------------------------------------------------
func create5Seconds() *GlobalDefinition {
	return NewGlobalDefinitionWithAlias("FiveSeconds", INT64, "time.Duration", "GetFiveSeconds", "")
}

//--------------------------------------------------------------------
