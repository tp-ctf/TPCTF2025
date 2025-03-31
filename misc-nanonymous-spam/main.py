from ipaddress import IPv4Address
from random import Random
from typing import Any, Dict, List

from flask import Flask, abort, jsonify, render_template, request

app = Flask(__name__)

# Initialize random generator with fixed seed for reproducible results
RANDOM_SEED = 'NanoApe'
random_generator = Random(RANDOM_SEED)

# Load dictionary file and capitalize first letter of each word
with open('dict.txt') as f:
    word_list = f.read().strip().split('\n')
    word_list = [word[:1].upper() + word[1:].lower() for word in word_list]

# Define lengths for four word groups
group_lengths = [273, 313, 513, 103]

# Create word indices and shuffle them randomly
word_indices = list(range(len(word_list)))
random_generator.shuffle(word_indices)

# Split indices into four groups
word_groups = []
start_index = 0
for length in group_lengths:
    end_index = start_index + length
    assert end_index <= len(word_list)
    word_groups.append(word_indices[start_index:end_index])
    start_index = end_index

# Create random shuffle patterns for the first group
shuffle_patterns = []
for _ in range(group_lengths[0]):
    pattern = list(range(4))
    random_generator.shuffle(pattern)
    shuffle_patterns.append(pattern)

print(shuffle_patterns)


def encode_ip(ip: IPv4Address) -> str:
    """
    Encode an IP address into a unique four-word string.

    Args:
        ip: IPv4Address object to encode

    Returns:
        String representation of the encoded IP
    """
    ip_int = int(ip)
    selected_indices = [None, None, None, None]
    shuffle_pattern = None

    # Extract indices from integer IP address
    for i in range(3, -1, -1):
        selected_indices[i] = word_groups[i][ip_int % group_lengths[i]]
        if i == 0:
            shuffle_pattern = shuffle_patterns[ip_int % group_lengths[i]]
        ip_int //= group_lengths[i]

    # Rearrange words according to shuffle pattern and join
    result = ''.join(
        word_list[selected_indices[shuffle_pattern[i]]] for i in range(4))
    return result


SPAM_CONTENT = [
    "All over 40 mature porn featuring milf and mature xxx pictures\nhttps://xxxxxxxxx.xxxxx.net/?alexandrea-briana",
    "Goshen women hot hookups goshen women single personals\nhttps://xxxxxxxxx.xxxxx.com/?kiera-delaney",
    "lolita madness guild guild forums gaia online\nhttps://xxxxxxxxx.xxxxx.com/?reina-felicity",
    "Leading porn sites caught by new eu law to police online content\nhttps://xxxxxxxxx.xxxxx.net/?megan-madison",
    "Real homemade cuckold wife like to film other men fucking her\nhttps://xxxxxxxxx.xxxxx.net/?rebekah-noemi",
]

FLAG = "TPCTF{finally_the_criminal5_wh0_publi5hed_the5e_5pam_were_arre5ted}"


# Sample messages with encoded IPs
messages: List[Dict[str, str]] = []

for i in range(0, len(FLAG), 4):
    chunk = FLAG[i:i+4].ljust(4)
    ip_parts = [ord(c) for c in chunk]
    ip_str = '.'.join(map(str, ip_parts))
    ip = IPv4Address(ip_str)
    messages.append({
        "ip": encode_ip(IPv4Address(ip)),
        # "ip": ip,
        "content": random_generator.choice(SPAM_CONTENT)
    })


def get_client_ip():
    """Get the real client IP when behind a proxy"""
    # Try X-Real-IP first
    if request.headers.get('X-Real-IP'):
        return request.headers.get('X-Real-IP')
    # Try X-Forwarded-For next
    elif request.headers.get('X-Forwarded-For'):
        # The leftmost IP in the list is the original client
        ip = request.headers.get('X-Forwarded-For').split(',')[0].strip()
        return ip
    # Fall back to remote_addr if no proxy headers are present
    else:
        return request.remote_addr


@app.route('/')
def index():
    """Render the main page with messages and user IP."""
    # Get user IP
    if hasattr(request, 'headers'):
        user_ip = get_client_ip()  # Use our custom function if not using ProxyFix
    else:
        user_ip = request.remote_addr

    # Validate IP address
    try:
        ip_obj = IPv4Address(user_ip)
        encoded_ip = encode_ip(ip_obj)
    except ValueError:
        # Return a proper 400 error response
        return abort(400, description="Invalid Request")

    return render_template('index.html', messages=messages, user_ip=encoded_ip)


@app.route('/api/submit', methods=['POST'])
def submit_message():
    """Handle message submission API."""
    # Get the content from the form
    content = request.form.get('content', '')

    # Validate content (optional)
    if not content or len(content) > 1000:
        return jsonify({"status": "error", "message": "Invalid content"}), 400

    # Here you would normally store the message
    # For now, just return success
    return jsonify({"status": "success", "message": "Submission successful"})


if __name__ == '__main__':
    app.run(port=7777)
