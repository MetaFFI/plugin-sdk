package guest

import "sync"

func MakeBufferedChannel(size int) chan int {
	return make(chan int, size)
}

func SendAndClose(ch chan int, values []int) {
	for _, val := range values {
		ch <- val
	}
	close(ch)
}

func ReceiveAll(ch chan int) []int {
	var res []int
	for val := range ch {
		res = append(res, val)
	}
	return res
}

func AddAsync(a int, b int) int {
	var wg sync.WaitGroup
	ch := make(chan int, 1)
	wg.Add(1)
	go func() {
		defer wg.Done()
		ch <- a + b
	}()
	wg.Wait()
	close(ch)
	return <-ch
}
