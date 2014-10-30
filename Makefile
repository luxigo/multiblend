
all: multiblend

multiblend:
	g++ -msse2 -O3 -ltiff -ltiffxx -ljpeg -lpng multiblend.cpp -o multiblend

install:
	install multiblend /usr/bin/
