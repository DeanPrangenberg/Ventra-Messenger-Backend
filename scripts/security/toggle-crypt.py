#!/usr/bin/env python3
import argparse
import json
import os
import sys
import base64
import time
from pathlib import Path

try:
    from argon2.low_level import hash_secret_raw, Type
    from cryptography.hazmat.primitives.ciphers.aead import AESGCM
except ImportError:
    print("Missing dependencies. Install with: pip install cryptography argon2-cffi", file=sys.stderr)
    sys.exit(1)

try:
    from dotenv import load_dotenv
except ImportError:
    print("Missing dependency. Install with: pip install python-dotenv", file=sys.stderr)
    load_dotenv = None

WAIT_ON_ERROR = True

def derive_key(password: bytes, salt: bytes) -> bytes:
    return hash_secret_raw(password, salt, time_cost=3, memory_cost=65536, parallelism=1, hash_len=32, type=Type.ID)

def encrypt_file(path: Path, password: str):
    if not path.is_file():
        print(f"Error: File not found: {path}", file=sys.stderr)
        return

    try:
        with open(path, "rb") as f:
            plaintext = f.read()
    except Exception as e:
        print(f"Error: Cannot read file {path}: {e}", file=sys.stderr)
        return

    salt = os.urandom(16)
    iv = os.urandom(12)
    key = derive_key(password.encode("utf-8"), salt)
    aesgcm = AESGCM(key)
    ciphertext = aesgcm.encrypt(iv, plaintext, None)

    out = {
        "salt": salt.hex(),
        "iv": iv.hex(),
        "ciphertext": base64.b64encode(ciphertext).decode("ascii"),
        "originalFile": str(path.resolve()),
    }

    # build encrypted filename: drop existing suffix and add .encJson
    if path.suffix:
        enc_name = path.with_suffix(".encJson")
    else:
        enc_name = path.with_name(f"{path.name}.encJson")
    outfile = enc_name
    counter = 1
    while outfile.exists():
        outfile = enc_name.with_name(f"{enc_name.stem}.{counter}.encJson")
        counter += 1

    try:
        with open(outfile, "w", encoding="utf-8") as f:
            json.dump(out, f, indent=2)
    except Exception as e:
        print(f"Error: Failed to write JSON file {outfile}: {e}", file=sys.stderr)
        return

    # secure delete original if possible
    if shutil_which("shred"):
        try:
            os.system(f'shred -u "{path}"')
        except Exception:
            try:
                path.unlink()
            except Exception:
                pass
    else:
        try:
            path.unlink()
        except Exception:
            pass

    print(f"Encrypted {path} to {outfile}")

def decrypt_file(path: Path, password: str):
    if not path.is_file():
        print(f"Error: File not found: {path}", file=sys.stderr)
        return

    try:
        with open(path, "r", encoding="utf-8") as f:
            obj = json.load(f)
    except Exception as e:
        print(f"Error: Failed to parse JSON {path}: {e}", file=sys.stderr)
        return

    required = {"salt", "iv", "ciphertext", "originalFile"}
    if not required.issubset(obj.keys()):
        print(f"Error: JSON missing required fields in {path}", file=sys.stderr)
        return

    try:
        salt = bytes.fromhex(obj["salt"])
        iv = bytes.fromhex(obj["iv"])
        ciphertext = base64.b64decode(obj["ciphertext"])
        original_path = Path(obj["originalFile"])
    except Exception as e:
        print(f"Error: Invalid field encoding in {path}: {e}", file=sys.stderr)
        return

    key = derive_key(password.encode("utf-8"), salt)
    aesgcm = AESGCM(key)

    outpath = original_path
    counter = 1
    while outpath.exists():
        outpath = original_path.with_name(f"{original_path.name}.{counter}")
        counter += 1

    try:
        plaintext = aesgcm.decrypt(iv, ciphertext, None)
    except Exception:
        print(f"Error: Decryption failed - wrong password or corrupted file {path}", file=sys.stderr)
        return

    try:
        outpath.parent.mkdir(parents=True, exist_ok=True)
        with open(outpath, "wb") as f:
            f.write(plaintext)
    except Exception as e:
        print(f"Error: Failed to write decrypted file {outpath}: {e}", file=sys.stderr)
        return

    # remove the .encJson after successful decryption
    try:
        path.unlink()
    except Exception:
        pass

    print(f"Decrypted {path} to {outpath} and removed {path.name}")

def shutil_which(cmd):
    from shutil import which
    return which(cmd)

def main():
    global WAIT_ON_ERROR

    # Find and load .env file from project root
    if load_dotenv:
        script_dir = Path(__file__).resolve().parent
        env_path = script_dir.parent.parent / '.env'
        if env_path.is_file():
            load_dotenv(dotenv_path=env_path)
        else:
            print(f"Warning: .env file not found at '{env_path}', relying on existing environment variables.", file=sys.stderr)

    parser = argparse.ArgumentParser(description="Encrypt/decrypt file with Argon2id + AES-256-GCM.")
    parser.add_argument("files", nargs="+", help="File(s) to encrypt/decrypt")
    parser.add_argument("--no-wait", action="store_true", help="Don't wait 10s after errors")
    args = parser.parse_args()

    if args.no_wait:
        WAIT_ON_ERROR = False

    password = None
    dev_mode = os.environ.get("DEV_MODE_SCRIPTS") == "1"

    if dev_mode:
        password = os.environ.get("DEV_ENCRYPTION_PASSWORD")
        if not password:
            print("Error: DEV_MODE_SCRIPTS is 1 but DEV_ENCRYPTION_PASSWORD is not set.", file=sys.stderr)
            sys.exit(1)
        print("DEV MODE: Using password from environment variable.")
    else:
        import getpass
        password = getpass.getpass("Password: ")

    for f in args.files:
        p = Path(f)
        try:
            # Check if file is likely encrypted JSON
            with open(p, "r", encoding="utf-8") as check:
                content = json.load(check)
            if isinstance(content, dict) and {"salt", "iv", "ciphertext", "originalFile"}.issubset(content.keys()):
                decrypt_file(p, password)
            else:
                encrypt_file(p, password)
        except (json.JSONDecodeError, UnicodeDecodeError):
            # If it's not valid JSON or text, assume it's a file to be encrypted
            encrypt_file(p, password)
        except Exception as e:
            print(f"Unhandled error on {p}: {e}", file=sys.stderr)
        finally:
            if WAIT_ON_ERROR and not dev_mode:
                time.sleep(1)

if __name__ == "__main__":
    main()