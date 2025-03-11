from Crypto.Random import get_random_bytes
from base64 import b64encode
from socket import socket, AF_INET, SOCK_STREAM
from threading import Thread, Lock
from typing import Dict, List, Tuple

PORT = 1337

nonce = get_random_bytes(12)

sock = socket(AF_INET, SOCK_STREAM)
sock.bind(('', PORT))
sock.listen(5)

messages: List[Tuple[bytes, socket]] = []
clients: Dict[socket, int] = {}
mutex = Lock()

def forward_messages() -> None:
    with mutex:
        for client, pos in list(clients.items()):
            try:
                for message, sender in messages[pos:]:
                    if sender != client:
                        client.sendall(message)
                clients[client] = len(messages)
            except (ConnectionError, BrokenPipeError):
                del clients[client]

def handle_client(client: socket, addr) -> None:
    global messages
    try:
        client.sendall(nonce)
        forward_messages()
        while True:
            message = client.recv(4096)
            if not message:
                break
            with mutex:
                messages.append((message, client))
            forward_messages()
            print(f'Received message from {addr}')
    except (ConnectionError, BrokenPipeError):
        pass
    finally:
        with mutex:
            clients.pop(client, None)
        client.close()
        print(f'Connection from {addr} closed')

try:
    while True:
        client, addr = sock.accept()
        print(f'New connection from {addr}')
        with mutex:
            clients[client] = 0
        Thread(target=handle_client, args=(client, addr), daemon=True).start()
except KeyboardInterrupt:
    pass
finally:
    sock.close()

with open('messages.txt', 'w') as f:
    stream = b''.join(m for m, _ in messages)
    print(b64encode(stream).decode(), file=f)
