.SUFFIXES:
.PHONY: clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors -I.
hmonsrcs:= HoareMonitor.hpp HoareMonitor.cpp 

all: prod-consLifo prod-consFifo

prod-consLifo: prod_consSULifoVarios.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp

prod-consFifo: prod_consSUFifoVarios.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp

clean:
	rm -f prod-consLifo prod-consFifo
