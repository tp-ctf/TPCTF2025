# chase (47 solves)

The players receive a nes rom file. This game is modified from the original game "chase" made by [Shiru](https://shiru.untergrund.net/software.shtml#nes).

You can download the source code and the original version, which is very helpful for solving this challenge.

## Modifications

- Hide the flag pt.1 in the winning screen.
- Extend the game to hidden level 6, which contains the flag pt.2.
- Adding some new tiles in CHR-ROM, which represent the flag pt.3.

Additionally, I add some constraints to make it more challenging.

But in general, this challenge is very easy.

## A Easy Way to Solve

Using the emulator like [FCEUX](https://fceux.com/web/home.html) is very easy to run the game rom.

## Flag pt.1

The game is not difficult, and you can easily play to win. ~~But as a CTFer, you must use another way to solve it.~~

FCEUX provides debugger, memory viewer, cheats and other useful tools. You can just modify the score to easily win the game.

## Flag pt.3

The flag pt.3 is hidden in the CHR-ROM. You can use the PPU viewer in FCEUX to view the CHR-ROM.

## Flag pt.2

### Intended

To get into the hidden level 6, you need to modify the game's code to bypass the check. There are many ways to do this.

1. Use the debugger to step through the program.
2. Use the IDA Pro to disassemble the code. (The IDA Pro can't load the nes rom directly, you need to use the loader from [https://github.com/Jinmo/nesldr-py](https://github.com/Jinmo/nesldr-py).)

Change the jump condition and you can easily enter the hidden level 6.

~~It is my fault not to add any hint for hidden level.~~

### Unintended

Some players use the hex editor to find the corresponding code of flag pt.1 in the winning screen, and find the flag pt.2 in the game rom.

Just replace the flag pt.1 with the pt.2 and win the game again to get the flag pt.2.

This is not the intended way, but it is also a valid way to solve this challenge.

~~I will obfuscate the flag next time.~~
