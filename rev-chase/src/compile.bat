@echo off

set name="chase.nes"

set path=%path%;D:\CTF\Tools\famicom\cc65-snapshot-win64\bin

cc65 -Oi game.c --add-source
ca65 crt0.s
ca65 game.s

ld65 -C nrom_128_horz.cfg -o %name% crt0.o game.o nes.lib

pause

del *.o
@REM del game.s

