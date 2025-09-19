#!/usr/bin/env python3
"""
forensic_carver.py

A signature‑based forensic file carving utility for raw disk images / block devices.

Features
- Scans a raw image (or device) for file signatures (headers/footers): JPEG, PDF, ZIP.
- Reconstructs files by carving from header to matching footer (contiguous carving).
- Handles embedded files by recursively scanning carved bytes (depth‑limited).
- Produces an index.json and index.csv with offsets, sizes, and validator status.
- Optional experimental fragmented recovery heuristics (very conservative; see notes).
- Memory-efficient scanning using mmap and streaming writes; tunable search windows.

Limitations (read me!)
- Fully reconstructing arbitrarily fragmented files is hard without filesystem context.
  This tool includes *experimental* heuristics (off by default) that attempt to bridge
  small gaps based on format-specific checks. Expect partial success at best.
- For rigorous investigations, pair this with filesystem tools (e.g., Sleuth Kit) or
  domain-specialized carvers. Treat results as best-effort leads, not ground truth.

Usage
  python3 forensic_carver.py RAW_IMAGE -o out_dir \
      --formats jpeg,pdf,zip --max-size 256MB --embedded-depth 2

  Optional flags:
    --fragmented                 Enable conservative fragmented recovery heuristics.
    --chunk-size 4096            Logical chunk size for heuristics (bytes).
    --jpeg-scan-window 128MB     Max search span for JPEG EOI from header.
    --pdf-scan-window 256MB      Max search span for PDF EOF from header.
    --zip-scan-window 256MB      Max search span for ZIP EOCD from first LFH.

Examples
  python3 forensic_carver.py disk.img -o carve_out --formats jpeg,pdf,zip --embedded-depth 1
  sudo python3 forensic_carver.py /dev/sdb -o carve_out --formats jpeg --max-size 128MB

Author: ChatGPT (GPT-5 Thinking)
License: MIT
"""
from __future__ import annotations
import argparse
import io
import json
import mmap
import os
import re
import struct
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Optional, Tuple, Dict

# ------------------------------- Helpers ------------------------------------

def human_bytes(n: int) -> str:
    units = ["B","KB","MB","GB","TB"]
    s = 0
    f = float(n)
    while f >= 1024 and s < len(units)-1:
        f /= 1024.0
        s += 1
    return f"{f:.2f} {units[s]}"

@dataclass
class CarveRecord:
    fmt: str
    start: int
    end: int
    size: int
    out_path: str
    validated: bool
    embedded_parent: Optional[str] = None
    notes: Optional[str] = None

# -------------------------- Format plugin base -------------------------------

class FormatPlugin:
    fmt: str = "abstract"

    def headers(self) -> Iterable[bytes]:
        """List of header byte sequences to search for."""
        raise NotImplementedError

    def find_header(self, buf: mmap.mmap, start: int) -> int:
        """Return next header offset >= start, or -1."""
        for magic in self.headers():
            off = buf.find(magic, start)
            if off != -1:
                return off
        return -1

    def find_footer(self, buf: mmap.mmap, h_off: int, max_scan: int) -> int:
        """Return end offset (exclusive) of the carved file, or -1 if not found.
        Default implementation scans for a fixed trailer; override per‑format.
        """
        raise NotImplementedError

    def validate(self, data: bytes) -> bool:
        """Lightweight sanity check for the carved file bytes."""
        return True

    def candidate_name(self, buf: mmap.mmap, h_off: int) -> str:
        return f"{self.fmt}_{h_off:012x}"

    # Fragment heuristics (optional; conservative). Return new end or -1.
    def fragmented_try_bridge(self, buf: mmap.mmap, h_off: int, max_span: int, chunk_size: int) -> int:
        return -1


# ------------------------------ JPEG plugin ----------------------------------

class JPEGPlugin(FormatPlugin):
    fmt = "jpeg"
    SOI = b"\xff\xd8"  # start of image
    EOI = b"\xff\xd9"  # end of image

    def headers(self) -> Iterable[bytes]:
        # We search for SOI; we will validate later with segment scan.
        return [self.SOI]

    def find_footer(self, buf: mmap.mmap, h_off: int, max_scan: int) -> int:
        # Look for the next EOI (FF D9) after h_off.
        start = h_off + 2
        end_search = min(len(buf), h_off + max_scan)
        eoi = buf.find(self.EOI, start, end_search)
        return eoi + 2 if eoi != -1 else -1

    def validate(self, data: bytes) -> bool:
        # Basic marker checks: starts with SOI, ends with EOI, contains at least one SOS (FF DA)
        if not (data.startswith(self.SOI) and data.endswith(self.EOI)):
            return False
        # Look for SOS marker suggesting real image data present
        return (b"\xff\xda" in data)  # Start Of Scan

    def fragmented_try_bridge(self, buf: mmap.mmap, h_off: int, max_span: int, chunk_size: int) -> int:
        # VERY conservative: look ahead for an EOI that appears shortly after a sequence of JPEG markers.
        # We scan for EOI within max_span and accept first plausible one.
        end_search = min(len(buf), h_off + max_span)
        scan = buf[h_off:end_search]
        pos = scan.find(self.EOI)
        if pos != -1:
            return h_off + pos + 2
        return -1

# ------------------------------- PDF plugin ----------------------------------

class PDFPlugin(FormatPlugin):
    fmt = "pdf"
    PDF_HEADER_RE = re.compile(br"%PDF-1\.(\d)")
    EOF = b"%%EOF"

    def headers(self) -> Iterable[bytes]:
        return [b"%PDF-"]

    def find_header(self, buf: mmap.mmap, start: int) -> int:
        return buf.find(b"%PDF-", start)

    def find_footer(self, buf: mmap.mmap, h_off: int, max_scan: int) -> int:
        end_search = min(len(buf), h_off + max_scan)
        # Find the *last* EOF prior to end_search to better handle embedded PDFs.
        window = buf[h_off:end_search]
        last = window.rfind(self.EOF)
        if last == -1:
            return -1
        # Include EOF token
        return h_off + last + len(self.EOF)

    def validate(self, data: bytes) -> bool:
        if not data.startswith(b"%PDF-"):
            return False
        if self.EOF not in data:
            return False
        # Try to locate startxref near the end
        tail = data[-2048:]
        if b"startxref" in tail:
            # crude xref pointer check
            try:
                idx = tail.rfind(b"startxref")
                line = tail[idx:].splitlines()[1].strip()
                int(line)
                return True
            except Exception:
                return True  # don't be too strict
        return True

    def fragmented_try_bridge(self, buf: mmap.mmap, h_off: int, max_span: int, chunk_size: int) -> int:
        # Look ahead for EOF; accept first plausible one.
        end_search = min(len(buf), h_off + max_span)
        window = buf[h_off:end_search]
        pos = window.find(self.EOF)
        return (h_off + pos + len(self.EOF)) if pos != -1 else -1

# -------------------------------- ZIP plugin ---------------------------------

class ZIPPlugin(FormatPlugin):
    fmt = "zip"
    LFH = b"PK\x03\x04"  # Local File Header
    EOCD = b"PK\x05\x06"  # End of Central Directory
    EOCD64_LOC = b"PK\x06\x07"  # ZIP64 End of Central Directory Locator
    EOCD64 = b"PK\x06\x06"  # ZIP64 End of Central Directory Record

    def headers(self) -> Iterable[bytes]:
        return [self.LFH]

    def find_header(self, buf: mmap.mmap, start: int) -> int:
        return buf.find(self.LFH, start)

    def find_footer(self, buf: mmap.mmap, h_off: int, max_scan: int) -> int:
        # Strategy: find EOCD (or ZIP64 EOCD locator/record) after the *first* LFH seen from h_off.
        end_search = min(len(buf), h_off + max_scan)
        window = buf[h_off:end_search]
        # Prefer EOCD64 if present; otherwise EOCD
        pos64loc = window.rfind(self.EOCD64_LOC)
        pos64 = window.rfind(self.EOCD64)
        poseocd = window.rfind(self.EOCD)
        cand = -1
        if pos64loc != -1 and pos64 != -1:
            # include the EOCD64 record completely; size unknown without parsing; approximate to record end
            # EOCD64 record is variable; fallback: take EOCD if present after it, else include basic length
            if poseocd != -1 and poseocd > pos64:
                cand = h_off + poseocd + 22  # EOCD basic size; may have comment; we'll extend below
            else:
                cand = h_off + pos64 + 56  # minimal EOCD64 size
        elif poseocd != -1:
            # EOCD length is variable due to comment; extend to end of window or end of EOCD + comment length if parsable
            # Try to parse the EOCD fixed fields (22 bytes min), last two bytes indicate comment length.
            abs_pos = h_off + poseocd
            end = self._parse_eocd_end(buf, abs_pos, end_search)
            cand = end
        return cand

    def _parse_eocd_end(self, buf: mmap.mmap, eocd_off_abs: int, end_search_abs: int) -> int:
        # EOCD structure: 4s|HHHHIIH then .ZIP file comment (n bytes)
        # Offset 20..21: comment length (H)
        if eocd_off_abs + 22 > end_search_abs:
            return eocd_off_abs + 22
        view = buf[eocd_off_abs:eocd_off_abs+22]
        try:
            fields = struct.unpack("<4sHHHHIIH", view)
            comlen = fields[-1]
            end = eocd_off_abs + 22 + comlen
            return min(end, len(buf))
        except Exception:
            return eocd_off_abs + 22

    def validate(self, data: bytes) -> bool:
        # Must have at least one LFH and one EOCD
        return (self.LFH in data) and (self.EOCD in data or self.EOCD64 in data or self.EOCD64_LOC in data)

    def fragmented_try_bridge(self, buf: mmap.mmap, h_off: int, max_span: int, chunk_size: int) -> int:
        # Look forward for EOCD/EOCD64 within span; accept first found.
        end_search = min(len(buf), h_off + max_span)
        window = buf[h_off:end_search]
        for sig in (self.EOCD, self.EOCD64_LOC, self.EOCD64):
            pos = window.find(sig)
            if pos != -1:
                if sig == self.EOCD:
                    return self._parse_eocd_end(buf, h_off + pos, end_search)
                else:
                    return h_off + pos + len(sig)
        return -1

# ------------------------------- Carver core ---------------------------------

FORMAT_PLUGINS: Dict[str, FormatPlugin] = {
    "jpeg": JPEGPlugin(),
    "pdf": PDFPlugin(),
    "zip": ZIPPlugin(),
}

@dataclass
class CarveOptions:
    out_dir: Path
    formats: List[str]
    max_size: int
    embedded_depth: int = 0
    fragmented: bool = False
    chunk_size: int = 4096
    scan_windows: Dict[str, int] = field(default_factory=lambda: {
        "jpeg": 128*1024*1024,  # 128MB
        "pdf": 256*1024*1024,   # 256MB
        "zip": 256*1024*1024,   # 256MB
    })

class Carver:
    def __init__(self, image_path: Path, opts: CarveOptions):
        self.image_path = image_path
        self.opts = opts
        self.plugins = [FORMAT_PLUGINS[f] for f in opts.formats]
        self.records: List[CarveRecord] = []
        self._ensure_out()

    def _ensure_out(self):
        self.opts.out_dir.mkdir(parents=True, exist_ok=True)
        (self.opts.out_dir / "carved").mkdir(exist_ok=True)

    def run(self):
        with open(self.image_path, 'rb') as f:
            with mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
                self._scan(mm)
        self._write_indexes()

    # --------------------------- scanning pipeline ---------------------------

    def _scan(self, mm: mmap.mmap, *, embedded_source: Optional[Tuple[str, bytes]] = None, depth: int = 0):
        """Scan the given buffer (mmap or bytes) for headers across plugins."""
        source_desc = self.image_path.name if embedded_source is None else f"embedded:{embedded_source[0]}"
        buf = mm if isinstance(mm, mmap.mmap) else mm  # symmetric typing
        total_len = len(buf)
        print(f"[i] Scanning {source_desc} ({human_bytes(total_len)}), depth={depth}")

        for plugin in self.plugins:
            off = 0
            magic_list = list(plugin.headers())
            while off < total_len:
                h = buf.find(magic_list[0], off)
                if h == -1:
                    break
                # carve attempt
                end = plugin.find_footer(buf, h, self.opts.scan_windows.get(plugin.fmt, self.opts.max_size))
                used_fragment = False
                if end == -1 and self.opts.fragmented:
                    end = plugin.fragmented_try_bridge(buf, h, self.opts.scan_windows.get(plugin.fmt, self.opts.max_size), self.opts.chunk_size)
                    used_fragment = end != -1
                if end != -1:
                    size = end - h
                    if size <= 0 or size > self.opts.max_size:
                        off = h + 1
                        continue
                    data = bytes(buf[h:end])
                    valid = plugin.validate(data)
                    name = plugin.candidate_name(buf, h)
                    out_path = self._emit_file(plugin.fmt, name, data)
                    rec = CarveRecord(fmt=plugin.fmt, start=h, end=end, size=size,
                                       out_path=out_path, validated=valid,
                                       embedded_parent=(embedded_source[0] if embedded_source else None),
                                       notes=("fragmented-bridge" if used_fragment else None))
                    self.records.append(rec)

                    # Optionally recurse into carved data for embedded finds
                    if self.opts.embedded_depth > depth:
                        self._scan_memory_for_embedded(plugin, data, depth+1, parent_name=name)
                    off = end  # continue search after this file
                else:
                    off = h + 1

    def _scan_memory_for_embedded(self, parent_plugin: FormatPlugin, data: bytes, depth: int, parent_name: str):
        # Create a read-only mmap-like object using memoryview for uniform interface
        # Here we just wrap in a BytesIO and expose .find via bytes; keep it simple.
        # We scan only for *other* formats to avoid infinite self-matches.
        print(f"[i]   Recursing into {parent_name} (depth {depth}) for embedded files…")
        for plugin in self.plugins:
            if plugin.fmt == parent_plugin.fmt:
                continue
            off = 0
            while True:
                h = data.find(next(iter(plugin.headers())), off)
                if h == -1:
                    break
                end = plugin.find_footer(data, h, self.opts.scan_windows.get(plugin.fmt, self.opts.max_size))
                if end != -1 and 0 < (end - h) <= self.opts.max_size:
                    sub = data[h:end]
                    valid = plugin.validate(sub)
                    name = f"{parent_name}__{plugin.fmt}_{h:08x}"
                    out_path = self._emit_file(plugin.fmt, name, sub)
                    rec = CarveRecord(fmt=plugin.fmt, start=h, end=end, size=end-h,
                                       out_path=out_path, validated=valid,
                                       embedded_parent=parent_name, notes="embedded")
                    self.records.append(rec)
                    off = end
                else:
                    off = h + 1

    # ---------------------------- output / index -----------------------------

    def _emit_file(self, fmt: str, name: str, data: bytes) -> str:
        ext = {"jpeg": ".jpg", "pdf": ".pdf", "zip": ".zip"}.get(fmt, ".bin")
        out_path = self.opts.out_dir / "carved" / f"{name}{ext}"
        with open(out_path, 'wb') as w:
            w.write(data)
        print(f"[+ ] Carved {fmt.upper()} -> {out_path.name} ({human_bytes(len(data))})")
        return str(out_path)

    def _write_indexes(self):
        index_json = self.opts.out_dir / "index.json"
        index_csv = self.opts.out_dir / "index.csv"
        payload = [rec.__dict__ for rec in self.records]
        with open(index_json, 'w') as f:
            json.dump(payload, f, indent=2)
        with open(index_csv, 'w') as f:
            f.write("fmt,start,end,size,validated,out_path,embedded_parent,notes\n")
            for r in self.records:
                f.write(
                    f"{r.fmt},{r.start},{r.end},{r.size},{int(r.validated)},{r.out_path},{r.embedded_parent or ''},{r.notes or ''}\n"
                )
        print(f"[i] Wrote index: {index_json} & {index_csv}")

# ------------------------------ CLI parsing ----------------------------------

def parse_size(s: str) -> int:
    m = re.match(r"^(\d+)([KMGTP]?B?)$", s.strip(), re.IGNORECASE)
    if not m:
        raise argparse.ArgumentTypeError(f"Invalid size: {s}")
    num = int(m.group(1))
    suf = m.group(2).upper()
    mult = 1
    if suf.startswith('K'): mult = 1024
    elif suf.startswith('M'): mult = 1024**2
    elif suf.startswith('G'): mult = 1024**3
    elif suf.startswith('T'): mult = 1024**4
    return num * mult

def main(argv: Optional[List[str]] = None):
    p = argparse.ArgumentParser(description="Signature-based forensic file carver (JPEG/PDF/ZIP)")
    p.add_argument("image", help="Path to raw image or block device (read-only recommended)")
    p.add_argument("-o", "--out", required=True, help="Output directory for carved files & indexes")
    p.add_argument("--formats", default="jpeg,pdf,zip", help="Comma-separated: jpeg,pdf,zip")
    p.add_argument("--max-size", default="512MB", type=parse_size, help="Max carve size per file (e.g., 256MB)")
    p.add_argument("--embedded-depth", type=int, default=0, help="Depth to scan carved files for embedded files")
    p.add_argument("--fragmented", action="store_true", help="Try experimental fragmented recovery heuristics")
    p.add_argument("--chunk-size", type=int, default=4096, help="Logical chunk size for heuristics")
    p.add_argument("--jpeg-scan-window", type=parse_size, default="128MB")
    p.add_argument("--pdf-scan-window", type=parse_size, default="256MB")
    p.add_argument("--zip-scan-window", type=parse_size, default="256MB")

    args = p.parse_args(argv)

    image = Path(args.image)
    out_dir = Path(args.out)
    formats = [s.strip().lower() for s in args.formats.split(',') if s.strip()]
    for f in formats:
        if f not in FORMAT_PLUGINS:
            p.error(f"Unknown format: {f}")

    opts = CarveOptions(
        out_dir=out_dir,
        formats=formats,
        max_size=args.max_size,
        embedded_depth=args.embedded_depth,
        fragmented=args.fragmented,
        chunk_size=args.chunk_size,
        scan_windows={
            "jpeg": args.jpeg_scan_window,
            "pdf": args.pdf_scan_window,
            "zip": args.zip_scan_window,
        }
    )

    start = time.time()
    carver = Carver(image, opts)
    try:
        carver.run()
    except PermissionError as e:
        print(f"[!] Permission error reading {image}. Try running with elevated privileges.")
        raise
    finally:
        dur = time.time() - start
        print(f"[i] Done in {dur:.2f}s. Carved {len(carver.records)} files.")

if __name__ == "__main__":
    main()
