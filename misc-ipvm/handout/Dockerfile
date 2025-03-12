FROM debian:12-slim AS compiler

RUN apt update && apt install -y gcc && rm -rf /var/lib/apt/lists/*

COPY readflag.c /
RUN gcc /readflag.c -o /readflag && rm /readflag.c

FROM python:3.12-slim

RUN apt update && apt install -y wget xz-utils && rm -rf /var/lib/apt/lists/*

RUN wget https://dist.ipfs.tech/kubo/v0.33.2/kubo_v0.33.2_linux-amd64.tar.gz && \
    tar xvf kubo_v0.33.2_linux-amd64.tar.gz && \
    kubo/install.sh && \
    rm -rf kubo_v0.33.2_linux-amd64.tar.gz kubo

RUN wget https://github.com/WebAssembly/wabt/releases/download/1.0.36/wabt-1.0.36-ubuntu-20.04.tar.gz && \
    tar xvf wabt-1.0.36-ubuntu-20.04.tar.gz && \
    cp wabt-1.0.36/bin/wat2wasm /usr/local/bin/ && \
    rm -rf wabt-1.0.36-ubuntu-20.04.tar.gz wabt-1.0.36

RUN wget https://github.com/bytecodealliance/wasmtime/releases/download/v30.0.1/wasmtime-v30.0.1-x86_64-linux.tar.xz && \
    tar xvf wasmtime-v30.0.1-x86_64-linux.tar.xz && \
    cp wasmtime-v30.0.1-x86_64-linux/wasmtime /usr/local/bin/ && \
    rm -rf wasmtime-v30.0.1-x86_64-linux.tar.xz wasmtime-v30.0.1-x86_64-linux

RUN groupadd -r ctf && \
    useradd -m -g ctf ctf && \
    mkdir /app/ && \
    chown -R ctf:ctf /app

RUN pip3 install base58 pycryptodome "fastapi[standard]"
COPY server.py index.html /app/

COPY --chmod=600 flag /
COPY --from=compiler /readflag /
RUN chmod +s /readflag

WORKDIR /app

COPY --chmod=755 docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
