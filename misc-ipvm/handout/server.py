import asyncio
import asyncio.subprocess as sp
import httpx
import os

from base58 import BITCOIN_ALPHABET
from contextlib import asynccontextmanager
from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import StreamingResponse, FileResponse
from pydantic import BaseModel, model_validator, AfterValidator
from starlette.background import BackgroundTask
from string import ascii_letters, digits
from tempfile import TemporaryDirectory

from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256
from Crypto.PublicKey import RSA

from typing import Annotated

IPFS_API = "http://ipfs:5001"
IPFS_API_MULTIADDR = "/dns4/ipfs/tcp/5001"

key = RSA.generate(2048)


@asynccontextmanager
async def lifespan(app: FastAPI):
    async with httpx.AsyncClient(base_url=IPFS_API) as client:
        yield {"client": client}


app = FastAPI(lifespan=lifespan)


# ---- Reverse Proxy ----


async def _reverse_proxy(request: Request):
    client = request.state.client
    url = httpx.URL(path=request.url.path, query=request.url.query.encode())
    headers = [(k, v) for k, v in request.headers.raw if k != b"host"]
    req = client.build_request(
        request.method, url, headers=headers, content=request.stream()
    )
    r = await client.send(req, stream=True)
    return StreamingResponse(
        r.aiter_raw(),
        status_code=r.status_code,
        headers=r.headers,
        background=BackgroundTask(r.aclose),
    )


app.add_route("/api/v0/{path:path}", _reverse_proxy, ["POST"])


# ---- Main API ----


class IPVMError(HTTPException):
    def __init__(self, message: str):
        super().__init__(status_code=400, detail=message)


class Config(BaseModel):
    name: str
    author: str | None = None
    version: str | None = None
    description: str | None = None

    entrypoint: str = "_start"

    @model_validator(mode="after")
    def verify(self):
        if not (
            all(c in (ascii_letters + "_") for c in self.entrypoint)
            and 0 < len(self.entrypoint) <= 10
        ):
            raise ValueError("Invalid entrypoint")

        return self


def verify_cid(cid):
    cs = cid.encode()
    if not (all(c in BITCOIN_ALPHABET for c in cs) and len(cs) == 46):
        raise ValueError("Invalid CID")

    return cid


Cid = Annotated[str, AfterValidator(verify_cid)]


async def check_output(cmd, stdout=sp.PIPE, stderr=sp.DEVNULL, timeout=None, **kwargs):
    proc = await sp.create_subprocess_exec(*cmd, stdout=stdout, stderr=stderr, **kwargs)
    stdout, _ = await asyncio.wait_for(proc.communicate(), timeout=timeout)
    if proc.returncode != 0:
        raise IPVMError(f"Command {cmd[0]} failed with return code {proc.returncode}")

    return stdout


async def ipfs_call(args):
    return await check_output(["ipfs", "--api", IPFS_API_MULTIADDR, *args], timeout=30)


async def ipfs_read(path):
    return await ipfs_call(["cat", path])


async def check_package(cid, allowed: set[str]):
    content = (await ipfs_call(["ls", cid])).decode()
    total_size = 0
    for line in content.splitlines():
        _, size, filename = line.split(maxsplit=2)
        assert filename in allowed, f"Invalid file: {filename}"
        total_size += int(size)

    if total_size > 128 * 1024:
        raise IPVMError("Package too large")


class BuildRequest(BaseModel):
    cid: Cid


@app.post("/build")
async def build_request(request: BuildRequest):
    cid = request.cid
    Config.model_validate_json(await ipfs_read(f"{cid}/config.json"))

    await check_package(cid, {"config.json", "main.wat", "main.wasm"})

    with TemporaryDirectory() as td:
        await ipfs_call(["get", cid, "-o", td])
        if os.path.exists(f"{td}/main.wat"):
            await check_output(
                ["wat2wasm", "main.wat", "-o", "main.wasm"],
                cwd=td,
                timeout=5,
            )
            os.remove(f"{td}/main.wat")

        if not os.path.exists(f"{td}/main.wasm"):
            raise IPVMError("No wasm file found")

        await check_output(
            ["wasmtime", "compile", "main.wasm"],
            cwd=td,
            timeout=5,
        )
        os.remove(f"{td}/main.wasm")

        with open(f"{td}/main.cwasm", "rb") as f:
            h = SHA256.new(f.read())
        signature = pkcs1_15.new(key).sign(h)

        with open(f"{td}/main.cwasm.sig", "wb") as f:
            f.write(signature)

        output = (await ipfs_call(["add", "-r", td])).decode()
        line = output.strip().splitlines()[-1]
        package_cid = line.split()[1]

    return {
        "cid": package_cid,
    }


class RunRequest(BaseModel):
    cid: Cid
    args: str = ""

    @model_validator(mode="after")
    def verify(self):
        if not (all(a in (digits + " ") for a in self.args) and len(self.args) <= 20):
            raise ValueError("Invalid args")

        return self


@app.post("/run")
async def run_request(request: RunRequest):
    cid = request.cid
    config = Config.model_validate_json(await ipfs_read(f"{cid}/config.json"))

    await check_package(cid, {"config.json", "main.cwasm", "main.cwasm.sig"})

    signature = await ipfs_read(f"{cid}/main.cwasm.sig")
    h = SHA256.new(await ipfs_read(f"{cid}/main.cwasm"))
    pkcs1_15.new(key).verify(h, signature)

    with TemporaryDirectory() as td:
        await ipfs_call(["get", cid, "-o", td])
        output = await check_output(
            [
                "wasmtime",
                "run",
                "--allow-precompiled",
                "--invoke",
                config.entrypoint,
                "main.cwasm",
                *request.args.split(),
            ],
            cwd=td,
            timeout=5,
        )

    return {"output": output.decode()}


@app.get("/")
async def read_index():
    return FileResponse("index.html")
