package guest

func MakeStringList() []string {
	return []string{"a", "b", "c"}
}

func MakeStringIntMap() map[string]int {
	return map[string]int{"a": 1, "b": 2}
}

func MakeIntSet() map[int]struct{} {
	return map[int]struct{}{1: {}, 2: {}, 3: {}}
}

func MakeNestedMap() map[string][]int {
	return map[string][]int{"nums": {1, 2, 3}}
}

func MakeSomeClassList() []*SomeClass {
	return []*SomeClass{{Name: "a"}, {Name: "b"}, {Name: "c"}}
}

func MakeMapAny() map[string]any {
	return map[string]any{"a": 1, "b": "two", "c": []int{3, 4}}
}
