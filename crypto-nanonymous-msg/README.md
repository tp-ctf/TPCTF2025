# nanonymous msg

## Description

󠇖󠆖󠄟󠇖󠅗󠆫󠅯󠆧󠅮󠄮󠇦󠇛󠄣󠆤󠄧󠇛󠄪󠅣󠇬󠆦󠄪󠆧󠅮󠇛󠄧󠆪󠇛󠅯󠇮󠅮󠇬󠇬󠇛󠇦󠄢󠅮󠇛󠅧󠅢󠄮󠅯󠇛󠇯󠆧󠄪󠇦󠅮󠇮󠆦󠇛󠆤󠅧󠇛󠆧󠆤󠇮󠇦󠅮󠆧󠇛󠅣󠄪󠄪󠇛󠅧󠆧󠆤󠅪󠇛󠅦󠅮󠅧󠄯󠆤󠅣󠇛󠄯󠇦󠅧󠇛󠆥󠆤󠆥󠆥󠇛󠆮󠇮󠄮󠅢󠇬󠇪Work in progress...

## Flag

`TPCTF{great_j0b_in5pire_by_gue55_the_flag_writeup_0f_r0uter_nii_fr0m_defc0n_ctf_2022_qual5}`

## Writeup

This challenge was inspired by [Guess The Flag: Writeup of router-nii from DEF CON CTF 2022 Quals](https://ptr-yudai.hatenablog.com/entry/2022/06/02/223338). However, fully replicating the approach would require another challenge to expose the flag format, which is why the additional challenge `nanonymous spam` was created.

The encryption method draws inspiration from [Hide a message in an emoji](https://emoji.paulbutler.org/?mode=encode). In reality, you'll find that this encryption method has little to do with emojis—it's essentially a zero-width character encoding. This led me to a mischievous idea: the challenge description directly states "󠇪Work in progress..." with zero-width characters placed at the beginning of the string, successfully tricking many into thinking it was merely an unfinished description (?

The zero-width Unicode used here is VARIATION SELECTOR, totaling 256 variants. For specifics, please refer to relevant documentation, as this won't be elaborated further.

Two primary solutions emerged during the competition. One mirrored the Guess The Flag approach, leveraging word frequency analysis and [quipquip](https://quipqiup.com/) alongside the flag format from the other challenge to deduce this one. The other involved reverse-engineering the encryption algorithm's bit map, which was feasible because the flag must start with `TPCTF{`. Additionally, the one-to-one correspondence between Unicode and 0–255 (evident from counting Unicode occurrences) allowed for rough reconstruction of the bit map structure. The obfuscation algorithm can be found in the source code.
