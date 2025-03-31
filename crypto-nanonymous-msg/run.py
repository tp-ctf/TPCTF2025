from random import Random

FLAG = "TPCTF{great_j0b_in5pire_by_gue55_the_flag_writeup_0f_r0uter_nii_fr0m_defc0n_ctf_2022_qual5}"

hidden_chars = list(range(65024, 65024+16))
hidden_chars += list(range(917760, 917760+256-16))
assert len(hidden_chars) == 256
hidden_chars = [chr(c) for c in hidden_chars]


RANDOM_SEED = 'NanoApe'
random_generator = Random(RANDOM_SEED)

shuffle_pattern = list(range(8))
random_generator.shuffle(shuffle_pattern)
mask = random_generator.randint(0, 255)

# print(shuffle_pattern)

c = {}
for x in range(256):
    result = 0
    for new_pos, old_pos in enumerate(shuffle_pattern):
        if x & (1 << old_pos):
            result |= (1 << new_pos)
    result ^= mask
    c[chr(x)] = result


for i in range(len(FLAG)):
    if i < 0x50:
        print(hidden_chars[c[FLAG[i]]], end='')
    else:
        print(hidden_chars[c[FLAG[i]]], end='')

print('Work in progress', end='')
print('...', end='')
