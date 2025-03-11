import json
from base64 import b64decode, b64encode
from string import ascii_letters

def solve(ciphertexts: list[bytes]):
    n = len(ciphertexts)
    m = len(ciphertexts[0])
    key = bytearray(m)
    for i in range(m):
        count = [0] * n
        for j, x in enumerate(ciphertexts):
            for k, y in enumerate(ciphertexts[:j]):
                if chr(x[i] ^ y[i]) in ascii_letters:
                    count[j] += 1
                    count[k] += 1
        key[i] = 32 ^ ciphertexts[count.index(max(count))][i]
    plaintexts = []
    for ciphertext in ciphertexts:
        plaintext = bytes(c ^ k for c, k in zip(ciphertext, key))
        plaintexts.append(b64encode(plaintext).decode())
    print(json.dumps(plaintexts))

with open('src/messages.txt') as f:
    stream = b64decode(f.read())

n = len(stream)
high_bits = bytes([b >> 7 for b in stream])

i = 0
while i < n:
    if high_bits.count(high_bits[i:i+50], i) < 5:
        i += 1
        continue
    l = i + 10
    r = i + 40
    k = high_bits.count(high_bits[l:r], i)
    while high_bits.count(high_bits[l-1:r], i) == k:
        l -= 1
    while high_bits.count(high_bits[l:r+1], i) == k:
        r += 1
    pattern = high_bits[l:r]
    ciphertexts = []
    while (p := high_bits.find(pattern, i)) != -1:
        ciphertexts.append(stream[p:p+len(pattern)])
        i = p + len(pattern)
    solve(ciphertexts)
