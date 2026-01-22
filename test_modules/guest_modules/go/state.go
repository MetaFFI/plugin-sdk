package guest

const ConstFiveSeconds int64 = 5

var FiveSeconds int64 = 5
var mutableCounter int64 = 0

func GetCounter() int64 {
	return mutableCounter
}

func SetCounter(value int64) {
	mutableCounter = value
}

func IncCounter(delta int64) int64 {
	mutableCounter += delta
	return mutableCounter
}
