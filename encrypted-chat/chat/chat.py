from subprocess import Popen, PIPE
from typing import Dict
from time import time
import json

with open('messages.json') as f:
    messages = json.load(f)

start = time()
pool = [Popen(['python', '../src/client.py', 'localhost'], stdin=PIPE, stdout=PIPE) for _ in range(10)]
processes: Dict[str, Popen] = {}

for message in messages:
    while time() - start < message['time']:
        pass
    sender = message['sender']
    if sender not in processes:
        processes[sender] = pool.pop()
    process = processes[sender]
    process.stdin.write(message['message'].encode() + b'\n')
    process.stdin.flush()

for process in processes.values():
    process.terminate()
    process.wait()

print(processes['Alice'].stdout.read())
