package guest

type Greeter interface {
	Greet(name string) string
}

type SimpleGreeter struct{}

func (g SimpleGreeter) Greet(name string) string {
	return "Hello " + name
}

func CallGreeter(g Greeter, name string) string {
	return g.Greet(name)
}
