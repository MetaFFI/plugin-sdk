package guest

type Color int

const (
	ColorRed Color = iota
	ColorGreen
	ColorBlue
)

func GetColor(idx int) Color {
	switch idx {
	case 0:
		return ColorRed
	case 1:
		return ColorGreen
	default:
		return ColorBlue
	}
}

func ColorName(color Color) string {
	switch color {
	case ColorRed:
		return "RED"
	case ColorGreen:
		return "GREEN"
	default:
		return "BLUE"
	}
}
