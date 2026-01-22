package guest

import "errors"

func GetThreeBuffers() [][]byte {
	return [][]byte{
		{1, 2, 3, 4},
		{5, 6, 7},
		{8, 9},
	}
}

func ExpectThreeBuffers(buffers [][]byte) error {
	if len(buffers) != 3 {
		return errors.New("buffers length is not 3")
	}
	if len(buffers[0]) != 4 || len(buffers[1]) != 3 || len(buffers[2]) != 2 {
		return errors.New("buffer sizes mismatch")
	}
	return nil
}

func GetSomeClasses() []*SomeClass {
	return []*SomeClass{{Name: "a"}, {Name: "b"}, {Name: "c"}}
}

func ExpectThreeSomeClasses(arr []*SomeClass) error {
	if len(arr) != 3 {
		return errors.New("array length is not 3")
	}
	for _, val := range arr {
		if val == nil {
			return errors.New("SomeClass element is nil")
		}
	}
	return nil
}

func Make2DArray() [][]int {
	return [][]int{{1, 2}, {3, 4}}
}

func Make3DArray() [][][]int {
	return [][][]int{{{1}, {2}}, {{3}, {4}}}
}

func MakeRaggedArray() [][]int {
	return [][]int{{1, 2, 3}, {4}, {5, 6}}
}

func Sum3DArray(arr [][][]int) int {
	sum := 0
	for _, plane := range arr {
		for _, row := range plane {
			for _, val := range row {
				sum += val
			}
		}
	}
	return sum
}

func SumRaggedArray(arr [][]int) int {
	sum := 0
	for _, row := range arr {
		for _, val := range row {
			sum += val
		}
	}
	return sum
}
