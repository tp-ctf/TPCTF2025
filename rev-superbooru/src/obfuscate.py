from tqdm import tqdm
import random

SPECIAL_CHARS = "(),/-"

names = set()
name_map = {}


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

    def transform(self, f):
        return f(self)


class Tag(Query):
    def __init__(self, tag: str):
        self.tag = tag

    def __str__(self):
        if self.tag in name_map:
            return name_map[self.tag]
        return self.tag

    def __eq__(self, other):
        return isinstance(other, Tag) and self.tag == other.tag

    def __hash__(self):
        return hash(self.tag)

    def simplify(self):
        return self


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

    def transform(self, f):
        return f(Neg(self.query.transform(f)))


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
                if item in negs:
                    return Group("and" if self.type == "or" else "or", [])
                negs.add(~item)

                queries.append(item)

        return Group(self.type, queries)

    def transform(self, f):
        return f(Group(self.type, [q.transform(f) for q in self.queries]))


def take_atom(lexer):
    token = next(lexer)
    if token == Token("("):
        return take_expr(lexer)
    elif token == Token("-"):
        return ~take_atom(lexer)
    elif isinstance(token, str):
        if not token.startswith("flag") and token != "check_flag":
            names.add(token)
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
    def __init__(self, condition, consequence: list[Tag]):
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


with open("implications_sane.txt") as f:
    imps = []
    for i, line in enumerate(f):
        line = line.strip()
        if not line:
            continue

        imps.append(parse_implication(line))

head, imps, tail = imps[:256], imps[256:-1], imps[-1:]

random.shuffle(imps)

for imp in tqdm(imps):
    imp.condition = imp.condition.simplify()


def obfuscate(query: Query, trues):
    if random.random() < 0.6:
        return query

    def tr(query):
        if random.random() < 0.2:
            queries = [query] + random.choices(trues, k=random.randint(1, 3))
            random.shuffle(queries)
            return Group("and", queries)

        return query

    return query.transform(tr)


all_tags = [Tag(name) for name in names]

dummy_tags = [Tag(f"_{i}") for i in range(256)]
names.update(t.tag for t in dummy_tags)

true = dummy_tags[0] | ~dummy_tags[0]

trues = [true]
for _ in range(12):
    if random.random() < 0.3:
        queries = [random.choice(trues)]
    else:
        c = random.choice(all_tags)
        queries = [c | ~c]
    for _ in range(random.randint(1, 4)):
        if random.random() < 0.4:
            queries.append(random.choice(all_tags))
        else:
            queries.append(random.choice(dummy_tags))

    random.shuffle(queries)
    trues.append(Group("or", queries))

print("\n".join(map(str, trues)))

dummy_cons = dummy_tags
dummy_cons += [~t for t in dummy_tags]

for imp in tqdm(imps):
    imp.condition = obfuscate(imp.condition, trues)
    imp.consequence += random.sample(dummy_cons, k=random.randint(1, 20))

used = set()
max_len = len(names).bit_length()
for name in names:
    while True:
        obf = "".join(random.choices("iі", k=max_len))
        if obf not in used:
            used.add(obf)
            break

    # if name.startswith("_"):
    # continue
    name_map[name] = obf


with open("implications.txt", "w") as f:
    f.write("""books -> book
ocean -> sea
sea -> ocean
male, -female -> solo_male, -male_and_female
female, -male -> solo_female, -male_and_female
sky / street -> outdoors

""")
    helper = Implication(Tag("flag_correct"), [Tag("hooray")])
    for imp in head + [helper] + imps + tail:
        print(imp, file=f)
