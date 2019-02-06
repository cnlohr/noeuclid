all: ../sfml-system-2.dll ../sfml-graphics-2.dll ../sfml-window-2.dll

SFML-2.2:
	wget http://www.sfml-dev.org/files/SFML-2.2-windows-gcc-4.9.2-mingw-32-bit.zip
	unzip SFML-2.2-windows-gcc-4.9.2-mingw-32-bit.zip

../sfml-%-2.dll: SFML-2.2
	cp SFML-2.2/bin/sfml-$*-2.dll ../

.PHONY: all