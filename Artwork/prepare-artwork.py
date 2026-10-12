"""Verify the original user-supplied artwork before compilation."""
from pathlib import Path
import hashlib

root = Path(__file__).resolve().parent
expected = {
    'VelvetNights': 'be6cdc535175b88da885f05fb1da16d6c0d8246831fd409c4fa7c52d7e948917',
    'MidnightHorizon': '50b65c9aa6e93767fb4c3f3f1dbc1960fcb7942c8357e36d8527cd4a451b3b62',
}
for name, digest in expected.items():
    destination = root / (name + '.png')
    if not destination.exists():
        raise RuntimeError('Missing artwork: ' + name)
    content = destination.read_bytes()
    if hashlib.sha256(content).hexdigest() != digest:
        raise RuntimeError('Artwork checksum mismatch: ' + name)
    print(name + ': original PNG checksum verified')
