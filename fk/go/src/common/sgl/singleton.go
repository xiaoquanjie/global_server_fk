package sgl

type Singleton struct {
}

func (s *Singleton) Lock() bool {
	ok := lockSgl()
	return ok
}

func (s *Singleton) Unlock() {
	unlockSgl()
}