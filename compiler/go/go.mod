module github.com/MetaFFI/sdk/compiler/go

go 1.21

require (
	github.com/Masterminds/sprig/v3 v3.2.3
	github.com/MetaFFI/sdk/idl_entities/go v0.0.0
	golang.org/x/text v0.27.0
)

replace github.com/MetaFFI/sdk/idl_entities/go => ../../idl_entities/go
