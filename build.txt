YMMV, but best results on Linux seem to be had with:

	g++ -msse2 -O3 -ltiff -ltiffxx -ljpeg -lpng multiblend.cpp -o multiblend

FreeBSD may need:

	g++ -msse2 -O3 -D__APPLE__=YES -I/usr/local/include -L/usr/local/lib -ltiff -ltiffxx -ljpeg -lpng multiblend.cpp -o multiblend
