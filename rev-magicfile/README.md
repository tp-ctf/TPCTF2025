# magicfile (16 solves)

This program is based on libmagic, which is used by the Unix file command to determine the type of a file. It loads the magic rule and checks the MIME type of the input.

To make it more challenging, I link the libmagic statically and remove the symbols. But you can still identify the library by the strings.

After extracting the magic bytes from the binary, you need to analyze the magic rule to find the flag.

According to the [definition](https://github.com/file/file/blob/0fa0ffd15ff17d798e2f985447af47ab8c3440fb/src/file.h#L223) of the magic rule, you can find the rule in this challenge is very simple: just check the every byte of the input by order.

Find the only one that returns "Congratulations! You got the flag.".

~~This challenge is designed just one day before the game, so some places may not be well done.~~
