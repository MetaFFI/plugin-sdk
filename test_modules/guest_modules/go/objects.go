package guest

type SomeClass struct {
	Name string
}

func (s *SomeClass) Print() string {
	return "Hello from SomeClass " + s.Name
}

type TestMap struct {
	data map[string]any
	Name string
}

func NewTestMap() *TestMap {
	return &TestMap{data: map[string]any{}, Name: "name1"}
}

func (t *TestMap) Set(key string, value any) {
	t.data[key] = value
}

func (t *TestMap) Get(key string) any {
	return t.data[key]
}

func (t *TestMap) Contains(key string) bool {
	_, ok := t.data[key]
	return ok
}

type Base struct {
	BaseValue int
}

func (b Base) BaseMethod() int {
	return b.BaseValue
}

type Derived struct {
	Base
	Extra string
}

func (d Derived) DerivedMethod() string {
	return d.Extra
}

type Outer struct {
	Inner *Inner
}

type Inner struct {
	Value int
}

func NewOuter(value int) *Outer {
	return &Outer{Inner: &Inner{Value: value}}
}
