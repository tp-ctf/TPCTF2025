# nanonymous spam

## Description

Due to the influx of spam messages, the administrator has temporarily closed the anonymous message board.
Can you identify the sender behind the spam messages?

## Flag

`TPCTF{finally_the_criminal5_wh0_publi5hed_the5e_5pam_were_arre5ted}`

## Writeup  

This challenge is essentially a reverse-engineering problem involving an IP address obfuscation algorithm. The anonymous message board in the question converts users' IP addresses into unique four-word combinations through a special algorithm. Our task is to understand this algorithm and reverse it to uncover the "culprit" behind the spam messages.  

First, to effectively reverse-engineer the algorithm, we needed to collect sufficient sample data. Through various attempts, we discovered a web vulnerability that could be exploited: the backend prioritizes the `X-Real-IP` header when fetching the client IP, but the reverse proxy failed to configure it properly.  

```python  
def get_client_ip():  
    # Try X-Real-IP first  
    if request.headers.get('X-Real-IP'):  
        return request.headers.get('X-Real-IP')  
    ...  
```  

This is a common configuration oversight—the administrator forgot to add the `proxy_set_header X-Real-IP $remote_addr;` directive in the Nginx configuration, allowing clients to spoof this header. Exploiting this vulnerability, we could simulate requests from arbitrary IP addresses, thereby obtaining a large number of mappings between IP addresses and their encoded counterparts.  

By systematically forging different IP addresses and observing their corresponding encodings, we found:  

1. When enumerating IPs in the format `X.0.0.0` (where X ranges from 1 to 255), the encoded words appeared random, with no obvious pattern.  
2. However, when enumerating IPs in the format `0.0.0.X`, the word changes exhibited a certain regularity.  
3. By comparing the encodings for `0.0.0.255` and `0.0.1.0`, we noticed only one word changed, indicating that **the encoding is related to the integer representation of the IP address rather than the individual digits**.  
4. Further testing confirmed that the number of possible words for each position was 273, 313, 513, and 103, respectively.  
5. The final layer of complexity lay in the word ordering, where the permutation pattern was determined by the first word.  

Once we understood how the algorithm worked, we could:  

1. Extract all encoded IPs of the spam senders from the webpage.  
2. Reverse the algorithm to convert these encodings back to the original IP addresses.  
3. Interpret each part of the IP address as ASCII characters.  
4. Concatenate these characters to obtain the complete flag.  

## Trivia  

The anonymous messages appearing on the board were excerpted from the comment section of my long-abandoned blog, with URLs and other real information removed before being used in the challenge.  
