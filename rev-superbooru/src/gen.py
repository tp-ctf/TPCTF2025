import random


class Expr:
    def __and__(self, other):
        if isinstance(other, Const):
            return self if other.value else Const(False)
        if isinstance(self, Const):
            return other & self

        return And(self, other)

    def __or__(self, other):
        if isinstance(other, Const):
            return Const(True) if other.value else self
        if isinstance(self, Const):
            return other | self

        return Or(self, other)

    def __xor__(self, other):
        return (self | other) & ~(self & other)

    def __eq__(self, other):
        return (self & other) | ~(self | other)

    def __invert__(self):
        if isinstance(self, Const):
            return Const(not self.value)
        if isinstance(self, Neg):
            return self.expr
        return Neg(self)

    def select(self, a, b):
        return (self & a) | (~self & b)

    def unwrap(self, op):
        return [self]

    def serialize(self, cx="any"):
        pass

    def save(self, name=None):
        if isinstance(self, Tag):
            return self
        return Tag(name, self)


tag_count = 0
tag_count2 = 0


class Const(Expr):
    def __init__(self, value):
        self.value = bool(value)

    def serialize(self, cx="any"):
        raise NotImplementedError

    def tags(self):
        return []

    def __str__(self):
        return str(self.value)


pcs = {}
current_version = {}
splitted = {}
sources = {}
file = None


class Tag(Expr):
    def __init__(self, name=None, expr=None, slot=False):
        global tag_count, tag_count2
        self.name = name
        self.expr = expr
        self.id = tag_count
        self.uid = tag_count2
        self.slot = slot
        tag_count += 1
        tag_count2 += 1

    def serialize(self, cx="any"):
        return str(self)

    def tags(self):
        yield self

    def get_name(self):
        return f"t{self.id}" if self.name is None else self.name

    def __str__(self):
        name = self.get_name()
        if name in current_version and current_version[name] > self.uid:
            if self.uid not in splitted:
                splitted[self.uid] = f"w{len(splitted)}"
            return splitted[self.uid]
        return name

    def store(self, expr):
        assert self.slot
        if isinstance(expr, Tag) and self.uid == expr.uid:
            return self

        result = Tag(self.name, expr, self.slot)
        result.id = self.id
        return result

    def __hash__(self):
        return self.uid


class Neg(Expr):
    def __init__(self, expr):
        self.expr = expr

    def serialize(self, cx="any"):
        return f"-{self.expr.serialize('neg')}"

    def tags(self):
        yield from self.expr.tags()

    def __str__(self):
        return f"-{self.expr}"


class And(Expr):
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def unwrap(self, op):
        if op == "and":
            return self.left.unwrap(op) + self.right.unwrap(op)
        return [self]

    def serialize(self, cx="any"):
        result = f"{self.left.serialize('and')}, {self.right.serialize('and')}"
        if cx != "any" and cx != "and":
            result = f"({result})"
        return result

    def tags(self):
        yield from self.left.tags()
        yield from self.right.tags()

    def __str__(self):
        return f"({self.left}, {self.right})"


class Or(Expr):
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def unwrap(self, op):
        if op == "or":
            return self.left.unwrap(op) + self.right.unwrap(op)
        return [self]

    def serialize(self, cx="any"):
        result = f"{self.left.serialize('or')} / {self.right.serialize('or')}"
        if cx != "any" and cx != "or":
            result = f"({result})"
        return result

    def tags(self):
        yield from self.left.tags()
        yield from self.right.tags()

    def __str__(self):
        return f"({self.left} / {self.right})"

class Int:
    def __init__(self, bits):
        self.bits = bits

    def const(value, bits):
        assert 0 <= value < 1 << bits
        return Int([Const(value & (1 << i)) for i in range(bits)])

    def same_kind(self, other):
        return isinstance(other, Int) and len(self.bits) == len(other.bits)

    def as_const(self):
        result = 0
        for i, bit in enumerate(self.bits):
            assert isinstance(bit, Const)
            result |= int(bit.value) << i
        return result

    def save(self):
        return Int([bit.save() for bit in self.bits])

    def store_to(self, other, offset):
        for i, bit in enumerate(self.bits):
            other[offset + i] = other[offset + i].store(bit)

    def rotl(self, other):
        assert isinstance(other, Int)
        result = self
        for i, bit in enumerate(other.bits):
            if (1 << i) % len(self.bits) == 0:
                break
            p = len(self.bits) - (1 << i)
            shifted = Int(result.bits[p:] + result.bits[:p])
            result = Int(
                [bit.select(x, y) for x, y in zip(shifted.bits, result.bits)]
            ).save()
        return result

    def __add__(self, other):
        assert self.same_kind(other)
        result = []
        carry = Const(False)
        for x, y in zip(self.bits, other.bits):
            result.append(x ^ y ^ carry)
            carry = ((x & y) | (x & carry) | (y & carry)).save()
        return Int(result)

    def __eq__(self, other):
        assert self.same_kind(other)
        result = Const(True)
        for x, y in zip(self.bits, other.bits):
            result = result & (x == y)
        return result

    def __xor__(self, other):
        assert self.same_kind(other)
        return Int([x ^ y for x, y in zip(self.bits, other.bits)])


def done(expr):
    flag_correct = Tag("flag_correct", expr)

    tags = {flag_correct.uid: flag_correct}

    tag_child = {}
    in_deg = {}
    queue = [flag_correct]
    head = 0
    while head < len(queue):
        cur = queue[head]
        head += 1
        if tags[cur].expr is None:
            continue

        parents = set()
        for tag in tags[cur].expr.tags():
            tags[tag.uid] = tag
            parents.add(tag.uid)
        in_deg[cur] = len(parents)

        for tag in parents:
            if tag not in tag_child:
                tag_child[tag] = []
                queue.append(tag)
            tag_child[tag].append(cur)

    tag_by_names = {}
    for tag in sorted(tags.keys()):
        name = tags[tag].get_name()
        if name in tag_by_names:
            for prev in tag_by_names[name]:
                if tag not in tag_child[prev]:
                    tag_child[prev].append(tag)
                    in_deg[tag] += 1

            tag_by_names[name].append(tag)
        else:
            tag_by_names[name] = [tag]

    layer = []
    for tag in tags:
        if in_deg.get(tag, 0) == 0:
            layer.append(tag)

    layers = []
    while layer:
        layers.append(layer)
        new_layer = []
        for tag in layer:
            for child in tag_child.get(tag, []):
                in_deg[child] -= 1
                if in_deg[child] == 0:
                    new_layer.append(child)

        layer = new_layer

    pc = Tag("check_flag")
    rules = []
    for i, layer in enumerate(layers[1:]):
        new_pc = Tag(f"pc{i}")
        names = []
        for tag in layer:
            names.append(tags[tag].get_name())
        assert len(names) == len(set(names))
        for tag in sorted(layer):
            tag = tags[tag]
            cond = (tag.expr & pc).serialize()
            rules.append(f"{cond} -> {tag}")
            sources[tag.uid] = cond
            # for expr in tag.expr.unwrap("or"):
            # cond = (expr & pc).serialize()
            # f.write(f"{cond} -> {tag}\n")
            if (tag.slot and i) or random.randint(0, 2) == 0:
                cond = (~tag.expr & pc).serialize()
                rules.append(f"{cond} -> -{tag}")

            current_version[tag.get_name()] = tag.uid
            pcs[tag.uid] = new_pc

        rules.append(f"{pc} -> {new_pc}, -{pc}")
        pc = new_pc

    assert not splitted
    # for tag, split in splitted.items():
    # rules.append(f"{sources[tag]} -> {split}")

    rules.pop()

    with open("implications_sane.txt", "w") as f:
        for rule in rules:
            f.write(f"{rule}\n")


flag = []
for i in range(256):
    tag = Tag(f"flag_bin_{i:02x}")
    flag.append(Tag(None, tag, slot=True))
    flag[-1].name = f"f{i}"


def load(idx, base=1, offset=0):
    result = Const(False)
    for i in range(2 ** len(idx.bits)):
        t = i * base + offset
        if t >= len(flag):
            break
        result = result | ((idx == Int.const(i, len(idx.bits))) & flag[t])
    return Tag(f"load{tag_count}", result)


def load_int(idx, size):
    return Int([load(idx, size, i) for i in range(size)])


def store(idx, bit, base=1, offset=0):
    for i in range(2 ** len(idx.bits)):
        t = i * base + offset
        if t >= len(flag):
            break
        flag[t] = flag[t].store(
            (idx == Int.const(i, len(idx.bits))).select(bit, flag[t])
        )


def store_int(idx, value):
    for i, bit in enumerate(value.bits):
        store(idx, bit, len(value.bits), i)


seed = 24
random.seed(seed)


def swap(i1, i2):
    t1 = Tag(None, flag[i1])
    t2 = Tag(None, flag[i2])

    flag[i1] = flag[i1].store(t2)
    flag[i2] = flag[i2].store(t1)


mask = 0xFFFFFFFF

for i in range(10):
    for _ in range(64):
        i1 = random.randint(0, 255)
        i2 = i1 ^ random.randint(1, 255)

        swap(i1, i2)

    for _ in range(6):
        i1 = random.randint(0, 7)
        i2 = i1 ^ random.randint(1, 7)
        i1 = Int.const(i1, 3)
        i2 = Int.const(i2, 3)

        a = load_int(i1, 32)
        b = load_int(i2, 32)

        a = (a ^ b).rotl(b) + Int.const(random.randint(0, mask), 32)
        b = (b ^ a).rotl(a) + Int.const(random.randint(0, mask), 32)

        store_int(i1, a)
        store_int(i2, b)

answer = b"TPCTF{8o0RU__!S_tUR!n6_COmPlEtE}"
result = bytearray(answer)


def set_bit(idx, bit):
    result[idx // 8] = (result[idx // 8] & ~(1 << (idx % 8))) | (bit << (idx % 8))


def get_bit(idx):
    return (result[idx // 8] >> (idx % 8)) & 1


def swap(i1, i2):
    t1 = get_bit(i1)
    t2 = get_bit(i2)

    set_bit(i1, t2)
    set_bit(i2, t1)


def rotl(a, b):
    b %= 32
    return ((a << b) | (a >> (32 - b))) & mask


random.seed(seed)
rs = []
for i in range(10):
    r1 = []
    for _ in range(64):
        i1 = random.randint(0, 255)
        i2 = i1 ^ random.randint(1, 255)

        swap(i1, i2)

        r1.append((i1, i2))

    r2 = []
    for _ in range(6):
        i1 = random.randint(0, 7)
        i2 = i1 ^ random.randint(1, 7)

        a = int.from_bytes(result[i1 * 4 : i1 * 4 + 4], "little")
        b = int.from_bytes(result[i2 * 4 : i2 * 4 + 4], "little")

        t1 = random.randint(0, mask)
        t2 = random.randint(0, mask)

        a = (rotl(a ^ b, b) + t1) & mask
        b = (rotl(b ^ a, a) + t2) & mask

        result[i1 * 4 : i1 * 4 + 4] = a.to_bytes(4, "little")
        result[i2 * 4 : i2 * 4 + 4] = b.to_bytes(4, "little")

        r2.append((i1, i2, t1, t2))

    rs.append((r1, r2))


flag_correct = Const(True)
for i in range(len(result) * 8):
    bit = bool(result[i // 8] & (1 << (i % 8)))
    flag_correct = flag_correct & (flag[i] if bit else ~flag[i])

for r1, r2 in reversed(rs):
    for i1, i2, t1, t2 in reversed(r2):
        a = int.from_bytes(result[i1 * 4 : i1 * 4 + 4], "little")
        b = int.from_bytes(result[i2 * 4 : i2 * 4 + 4], "little")

        b = rotl((b - t2) & mask, 32 - a) ^ a
        a = rotl((a - t1) & mask, 32 - b) ^ b

        result[i1 * 4 : i1 * 4 + 4] = a.to_bytes(4, "little")
        result[i2 * 4 : i2 * 4 + 4] = b.to_bytes(4, "little")

    for i1, i2 in reversed(r1):
        swap(i1, i2)

print(result)

done(flag_correct)
