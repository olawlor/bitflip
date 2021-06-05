all: bitflip

bitflip: bitflip.cpp
	$(CXX) $< -o $@ -O3

clean:
	- rm bitflip
