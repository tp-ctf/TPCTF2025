from dag_pb2 import *
from base58 import b58decode
import subprocess as sp
import requests as rq


ip, port = '127.0.0.1 8000'.split()
base = f"http://{ip}:{port}"
ipfs = ["ipfs", "--api", f"/ip4/{ip}/tcp/{port}"]


def add(path):
    output = sp.check_output(ipfs + ["add", "-r", path]).decode().strip()
    line = output.splitlines()[-1]
    return line.split()[1]


built = rq.post(f"{base}/build", json={"cid": add("build")}).json()
output = sp.check_output(ipfs + ["block", "get", built["cid"]])

node = PBNode()
node.ParseFromString(output)

exp = add("exp.cwasm")
node.Links.insert(2, PBLink(Hash=b58decode(exp), Name="main.cwasm", Tsize=13483))

p = sp.Popen(ipfs + ["block", "put", "--format=v0"], stdin=sp.PIPE, stdout=sp.PIPE)
p.stdin.write(node.SerializeToString())
p.stdin.close()
modified = p.stdout.read().decode().strip()

output = rq.post(f"{base}/run", json={"cid": modified, "args": "1"}).json()
print(output)
