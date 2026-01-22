package guest

import "strings"

func Sum(values ...int) int {
	total := 0
	for _, val := range values {
		total += val
	}
	return total
}

func Join(prefix string, values ...string) string {
	builder := strings.Builder{}
	builder.WriteString(prefix)
	for _, v := range values {
		builder.WriteString(":")
		builder.WriteString(v)
	}
	return builder.String()
}
