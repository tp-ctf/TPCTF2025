flag = "TPCTF{YoU_AR3_SO_5m@R7_TO_cRACk_Th1$_m@9iC_f1le}"

import string
import random

table = list(set(string.ascii_letters + string.digits + string.punctuation) - set("{}"))
mg_file = open("tpctf.magic", "w")

class Node:
    def __init__(self):
        self.children = {}

def insert(node, s):
    if not s:
        return
    if s[0] not in node.children:
        node.children[s[0]] = Node()
    insert(node.children[s[0]], s[1:])

def output(node, depth):
    keys = list(node.children.keys())
    random.shuffle(keys)
    for k in keys:
        v = node.children[k]
        if v.children:
            mg_file.write(depth * ">" + str(4 + depth) + "\tbyte\t" + str(ord(k)) + "\n")
            output(v, depth + 1)
        else:
            mg_file.write(depth * ">" + str(4 + depth) + "\tbyte\t" + str(ord(k)) + "\t" + ("Congratulations! You got the flag." if k == '}' else "Try again.") + "\n")
    mg_file.write(depth * ">" + "0\tdefault\tx\tTry again.\n")

rt = Node()
insert(rt, flag[6:])

for i in range(1000):
    s = "".join(random.choices(table, k=random.randint(1, 50)))
    insert(rt, s)

mg_file.write("0\tstring\tTPCTF\n")
mg_file.write(">5\tbyte\t" + str(ord('{')) +"\n")
output(rt, 2)
mg_file.write(">0\tdefault\tx\tTry again.\n")

import os

os.system("file -C -m tpctf.magic")
