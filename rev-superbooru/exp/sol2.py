from tqdm import tqdm
from z3 import Solver, Bool, BoolVal, And, Or, sat, is_true, unsat

SPECIAL_CHARS = "(),/-"

cur_pc = None


def format_z3(pc, tag):
    return Bool(f"{pc}_{tag}")


class Token:
    def __init__(self, value):
        self.value = value

    def __eq__(self, other):
        return isinstance(other, Token) and self.value == other.value

    def __repr__(self):
        return f"Token({self.value!r})"


class Lexer:
    def __init__(self, text: str):
        self.text = text
        self.pos = 0

    def __iter__(self):
        return self

    def char(self):
        if self.pos >= len(self.text):
            raise StopIteration
        return self.text[self.pos]

    def __next__(self):
        while self.char().isspace():
            self.pos += 1

        ch = self.char()
        if ch in SPECIAL_CHARS:
            self.pos += 1
            return Token(ch)

        start = self.pos
        while not (ch.isspace() or ch in SPECIAL_CHARS):
            self.pos += 1
            if self.pos >= len(self.text):
                break
            ch = self.char()

        return self.text[start : self.pos]


class Query:
    def __and__(self, other):
        return Group("and", [self, other])

    def __or__(self, other):
        return Group("or", [self, other])

    def __invert__(self):
        if isinstance(self, Neg):
            return self.query
        return Neg(self)

    def unwrap(self, type):
        return [self]

    def simplify(self):
        pass


class Tag(Query):
    def __init__(self, tag: str):
        self.tag = tag

    def __str__(self):
        return self.tag

    def __eq__(self, other):
        return isinstance(other, Tag) and self.tag == other.tag

    def __hash__(self):
        return hash(self.tag)

    def simplify(self):
        return self

    def tags(self):
        yield self.tag

    def to_z3(self):
        assert cur_pc is not None
        if self.tag in pc_order:
            return BoolVal(True)
        if self.tag.startswith("flag_bin_") or self.tag == "check_flag":
            return Bool(self.tag)

        for i in reversed(tag_pcs.get(self.tag, [])):
            if i < cur_pc:
                return format_z3(i, self.tag)

        return BoolVal(False)


class Neg(Query):
    def __init__(self, query: Query):
        self.query = query

    def __str__(self):
        return f"-{self.query}"

    def __eq__(self, other):
        return isinstance(other, Neg) and self.query == other.query

    def __hash__(self):
        return hash(self.query)

    def simplify(self):
        if isinstance(self.query, Neg):
            return self.query.query.simplify()
        # if isinstance(self.query, Group) and self.query.type == "and":
        # return Group("or", [~query for query in self.query.queries]).simplify()
        return ~self.query.simplify()

    def tags(self):
        return self.query.tags()

    def to_z3(self):
        return ~self.query.to_z3()


class Group(Query):
    def __init__(self, type: str, queries: list[Query]):
        self.type = type
        self.queries = queries

    def __str__(self):
        assert self.queries
        sep = ", " if self.type == "and" else " / "
        return f"({sep.join(map(str, self.queries))})"

    def unwrap(self, type):
        if self.type == type:
            result = []
            for query in self.queries:
                result.extend(query.unwrap(type))
            return result
        return [self]

    def __eq__(self, other):
        return (
            isinstance(other, Group)
            and self.type == other.type
            and self.queries == other.queries
        )

    def __hash__(self):
        return hash((self.type, tuple(self.queries)))

    def simplify(self):
        negs = set()
        queries = []
        for query in self.queries:
            for item in query.simplify().unwrap(self.type):
                if isinstance(item, Group) and not item.queries:
                    assert item.type != self.type
                    return item
                if item in negs:
                    return Group("and" if self.type == "or" else "or", [])
                negs.add(~item)

                queries.append(item)

        if len(queries) == 1:
            return queries[0]

        return Group(self.type, queries)

    def tags(self):
        for query in self.queries:
            yield from query.tags()

    def to_z3(self):
        queries = [query.to_z3() for query in self.queries]
        return And(queries) if self.type == "and" else Or(queries)


def take_atom(lexer):
    token = next(lexer)
    if token == Token("("):
        return take_expr(lexer)
    elif token == Token("-"):
        return ~take_atom(lexer)
    elif isinstance(token, str):
        return Tag(token)
    else:
        raise ValueError(f"Unexpected {token}")


def take_expr(lexer):
    stack = [take_atom(lexer)]
    while True:
        try:
            token = next(lexer)
        except StopIteration:
            break

        if token == Token("/"):
            value = take_atom(lexer)
            stack[-1] = stack[-1] | value
        elif token == Token(","):
            stack.append(take_atom(lexer))
        elif token == Token(")"):
            break
        else:
            raise ValueError(f"Unexpected {token}")

    return Group("and", stack)


def parse_query(query: str):
    lexer = Lexer(query)
    return take_expr(lexer)


class Implication:
    def __init__(self, condition, consequence: list[str]):
        self.condition = condition
        self.consequence = consequence

    def __str__(self):
        cond = str(self.condition)
        if cond.startswith("("):
            cond = cond[1:-1]
        cons = ", ".join(map(str, self.consequence))
        return f"{cond} -> {cons}"


def parse_implication(implication: str) -> Implication:
    lhs, rhs = implication.split("->")
    return Implication(parse_query(lhs), parse_query(rhs).unwrap("and"))


with open("implications_new.txt") as f:
    imps = []
    for i, line in enumerate(f):
        line = line.strip()
        if not line:
            continue

        imps.append(parse_implication(line))

for imp in tqdm(imps):
    imp.condition = imp.condition.simplify()

who_implies = {}
who_implies_neg = {}
for i, imp in enumerate(imps):
    for tag in imp.consequence:
        if isinstance(tag, Tag):
            who_implies.setdefault(tag.tag, []).append(i)
        elif isinstance(tag, Neg):
            who_implies_neg.setdefault(tag.query.tag, []).append(i)

pcs = ["check_flag"]
while True:
    pc = pcs[-1]
    if pc not in who_implies_neg:
        break
    assert len(who_implies_neg[pc]) == 1
    imp = imps[who_implies_neg[pc][0]]
    assert len(imp.consequence) == 2
    other = (
        imp.consequence[0]
        if isinstance(imp.consequence[1], Neg)
        else imp.consequence[0]
    )
    assert isinstance(other, Tag)
    pcs.append(other.tag)

print(len(pcs))

pc_order = {pc: i for i, pc in enumerate(pcs)}

important_imps = []
tag_pcs = {}
for imp in tqdm(imps):
    if isinstance(imp.condition, Tag) and imp.condition.tag in pcs:
        continue
    assert len(imp.consequence) == 1
    tag = imp.consequence[0]
    if isinstance(tag, Neg):
        continue

    tag = tag.tag
    if tag == "hooray":
        continue

    pc = None
    for t in imp.condition.tags():
        if t in pc_order:
            assert pc is None
            pc = t

    assert pc
    pc = pc_order[pc]
    imp.pc = pc
    important_imps.append(imp)
    tag_pcs.setdefault(tag, []).append(pc)

for pcs in tag_pcs.values():
    pcs.sort()

defs = {}

solver = Solver()
for imp in tqdm(important_imps):
    tag = imp.consequence[0].tag
    pc = imp.pc

    cur_pc = pc

    key = (pc, tag)
    val = defs.setdefault(key, BoolVal(False))
    defs[key] = val | imp.condition.to_z3()

for (pc, tag), val in defs.items():
    solver.add(format_z3(pc, tag) == val)

pcs = tag_pcs["flag_correct"]
assert len(pcs) == 1
solver.add(format_z3(pcs[0], "flag_correct"))

assert solver.check() == sat
model = solver.model()

ors = []

bits = []
flags = set()
for i in range(256):
    fl = f"flag_bin_{i:02x}"
    bits.append("01"[int(is_true(model[Bool(fl)]))])
    ors.append(Bool(fl) != is_true(model[Bool(fl)]))
    if is_true(model[Bool(fl)]):
        flags.add(fl)

solver.add(Or(*ors))
assert solver.check() == unsat

print(flags)

chs = []
for i in range(32):
    bs = bits[i * 8 : (i + 1) * 8]
    chs.append(chr(int("".join(reversed(bs)), 2)))

print("".join(chs))
