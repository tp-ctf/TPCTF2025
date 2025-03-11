https://ouuan.moe/post/2025/03/tpctf-2025

## encrypted chat (15 solves)

All senders share the same key stream, so race condition will happen if two participants send messages at the same time, before receiving the message sent from the other side, and then the key stream will be reused.

So we need to locate the parts where the key is reused and then decrypt them. Since the messages are in ASCII, we can use the highest bit in each byte to find reused key. The remaining of this challenge is [this assignment in CS255](https://crypto.stanford.edu/~dabo/cs255/hw_and_proj/hw1.html).

A simple approach is to XOR the messages and notice that XORing a letter with a space is to toggle its casing. So a character is likely to be a space if its XORs with others are letters. Then manually fix the broken words.

Another approach is to use some language model (e.g. GPT, a small one is enough) to calculate the probability of the next character (use the internal results, not to ask an AI assistant) and apply the Viterbi algorithm. Reference: [A Natural Language Approach to Automated Cryptanalysis of Two-time Pads](https://dl.acm.org/doi/abs/10.1145/1180405.1180435). This approach is more accurate but too expensive to implement during a CTF.[^gpt-viterbi]

[^gpt-viterbi]: I have actually implemented something similar before. It was able to decrypt MTP for various file types besides natural language. Specifically, it was used to decrypt the Conti ransomware.

Or you can use the known plaintext `TPCTF{` as a starting point.
