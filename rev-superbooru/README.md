
[简体中文](https://mivik.moe/2025/solution/tpctf-2025/#superbooru)

This challenge took inspiration from the booru image hosting service. Booru is a type of image hosting service that allows users to upload images and tag them. This challenge designed a simple booru with static tag rules, allowing to add/remove tags automatically based on rules. The rules are in the form of `condition -> consequence`, and their EBNF expression is as follows:

```
TAG = /\\w+/
ATOM = TAG | GROUP | NEG
GROUP = "(" CONDITION ")"
NEG = "-" ATOM

OR_TERM = ATOM ("/" ATOM)+
AND_TERM = OR_TERM ("," OR_TERM)+

CONDITION = ATOM | OR_TERM | AND_TERM
CONSEQUENCE = (NEG? TAG) ("," (NEG? TAG))*
```

Here are some examples of valid rules:

- `dog, male -> male_with_dog, -pet_only`: If the tags contain `dog` and `male`, then automatically add the tag `male_with_dog` and remove the tag `pet_only`.
- `(dog / cat), -male -> pet_only, animal_only`: If the tags contain `dog` or `cat` and do not contain `male`, then automatically add the tags `pet_only` and `animal_only`.

Well it looks good! Let's check the handout:

![BRUH](superbooru.png)

Note that here `i` is different from Ukrainian `і`.

If `consequence` does not contain `-`, the challenge would be simple. We can use z3 to define the tags as the disjunction of their implications and solve the constraint `flag_correct`. The annoying part is the `-`. How are we even supposed to add a tag and then remove it?

According to the code, the rules are applied round by round. In each round, rules that will be applied will be collected first, and be applied at the end of the round. By running flag check multiple times, we can notice an interesting insight: the number of rounds is almost fixed (around 2476 rounds). This hints that the rules might have a fixed set of logic. Additionally, the author kindly mentioned in the code:

```
# It's guaranteed that the same implication applied
# multiple times will not change the result
```

This means that the same rule can be applied at most once, and even if it is applied multiple times, it will not change the result. In fact, if we record when these rules are applied, we will find that they are basically fixed. Then we can have a rough guess that these rules are divided into many "layers", and within each layer, the rules either do not apply or apply in the same round (as the other rules in this layer).

But how EXACTLY are these rules divided into layers? As the hint released in the second half of the competition says, starting from the initial `check_flag` tag, there will be a chain of tags like `check_flag -> -check_flag, new_flag1`, `new_flag1 -> -new_flag1, new_flag2`, and so on. For the nth layer of rules, we add the condition of `new_flag{n}` to achieve this layered structure.

Given this information, we can write a script to simplify the rules and remove the unnecessary tags used for obfuscation. (Check <exp/sol.py>).

Then we extract the tag chain starting from `check_flag` (denoted as PC chain). After that, we can attach the layer information to the consequences. For example, for the following rules:

```
check_flag -> -check_flag, new_flag1
new_flag1, a -> c
new_flag1, -a -> -c

new_flag1 -> -new_flag1, new_flag2
new_flag2, b -> a
new_flag2, -b -> -a

new_flag2 -> -new_flag2, new_flag3
new_flag3, c -> b
new_flag3, -c -> -b
```

They're actually equivalent to executing `c = a, a = b, b = c` in order. After attaching the layer information, we have:

```
a1 = a0, b1 = b0, c1 = a0
a2 = b1, b2 = b1, c2 = c1
a3 = a2, b3 = c2, c3 = c2
```

So basically we can convert the problem into SMT then. To avoid the model from being way too complex, we only preserve the tags that have changed in each round (e.g., `a1, b1, b2, c2, a3, c3` in the example above will be discarded). Finally, we can directly use z3 to solve it. On this basis, we can also use z3 to verify that the solution is unique. See <exp/sol2.py>.

> P.S. For generation of `implications.txt`, see <problem/gen.py> and <problem/obfuscate.py>.
