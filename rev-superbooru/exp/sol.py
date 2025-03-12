from tqdm import tqdm
import spd

SPECIAL_CHARS = "(),/-"

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

    def tags(self):
        yield self.tag


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


# with open("implications.txt") as f:
#     imps = []
#     for i, line in enumerate(f):
#         line = line.strip()
#         if not line:
#             continue

#         imps.append(parse_implication(line))

imps = []
for line in spd.lines('implications.txt'):
    imps.append(parse_implication(line))

imps = imps[6:]

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

used = set()
queue = ["flag_correct"]
head = 0
while head < len(queue):
    cur = queue[head]
    head += 1
    for i in who_implies.get(cur, []):
        imp = imps[i]
        for tag in imp.condition.tags():
            if tag not in used:
                used.add(tag)
                queue.append(tag)

all_tags = set()
for imp in imps:
    all_tags.update(imp.condition.tags())
    all_tags.update(Group("and", imp.consequence).tags())

unused = all_tags - used - {"hooray", "flag_correct"}
for imp in imps:
    imp.consequence = [
        tag
        for tag in imp.consequence
        if not (isinstance(tag, Tag) and tag.tag in unused)
        and not (isinstance(tag, Neg) and tag.query.tag in unused)
    ]

with open("mapping.txt", "w") as f:
    for name in used:
        if not name.startswith("flag") and name != "check_flag":
            name_map[name] = f"t{len(name_map)}"
            print(f"{name_map[name]} = {name}", file=f)

with open("implications_new.txt", "w") as f:
    for imp in imps:
        print(imp, file=f)
