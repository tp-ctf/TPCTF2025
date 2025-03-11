from Crypto.Cipher import ChaCha20
from base64 import b64decode
from select import select
import os
import socket
import sys

PORT = 1337

key = os.environ.get('CHAT_KEY')
assert key is not None, 'CHAT_KEY environment variable must be set'
key = b64decode(key)
assert len(key) == 32, 'Key must be 32 bytes long'

if len(sys.argv) != 2:
    print(f'Usage: python client.py <server_address>')
    sys.exit(1)
server_address = sys.argv[1]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((server_address, PORT))

nonce = sock.recv(12)
cipher = ChaCha20.new(key=key, nonce=nonce)

sock.setblocking(False)

try:
    while True:
        readable, _, _ = select([sock, sys.stdin], [], [])
        if sock in readable:
            data = sock.recv(4096)
            if not data:
                break
            data = cipher.encrypt(data)
            print(data.decode(errors='replace'), end='', flush=True)
        if sys.stdin in readable:
            data = sys.stdin.readline()
            data = cipher.encrypt(data.encode())
            sock.sendall(data)
finally:
    sock.close()
