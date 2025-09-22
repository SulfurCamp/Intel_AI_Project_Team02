# my_consumer_same_process.py — consumer for on‑chip NMS pipeline
# - Consumes payloads from object_detection.py → inference_callback
# - Robust to {accuracy|score} fields and prints top‑K with throttling
# - Optional JSONL logging and last‑payload getters for other modules

from __future__ import annotations
from typing import Dict, Any, List, Optional
import atexit, os, time, json, threading

_last_payload: Optional[Dict[str, Any]] = None
_last_print_ts: Optional[float] = None
_lock = threading.RLock()

# ====== Runtime knobs (env) ======
PRINT_EVERY       = int(os.getenv("PRINT_EVERY", "30"))       # print every N frames (0 → disable)
PRINT_EVERY_SECS  = float(os.getenv("PRINT_EVERY_SECS", "0"))  # or every N seconds (0 → disable)
MAX_PRINT_DETS    = int(os.getenv("MAX_PRINT_DETS", "5"))      # cap printed dets per line
QUIET             = os.getenv("QUIET", "0") == "1"             # suppress console prints
PRINT_EMPTY       = os.getenv("PRINT_EMPTY", "0") == "1"       # print {} when no detections
LOG_JSONL         = os.getenv("LOG_JSONL", "")                 # path to write JSONL logs (append)

# lazily opened file handle
_log_fh = None

def _open_log_if_needed():
    global _log_fh
    if LOG_JSONL and _log_fh is None:
        os.makedirs(os.path.dirname(LOG_JSONL) or ".", exist_ok=True)
        _log_fh = open(LOG_JSONL, "a", buffering=1)
    return _log_fh


def _score_of(d: Dict[str, Any]) -> float:
    s = d.get("accuracy", d.get("score", 0.0))
    try:
        return float(s)
    except Exception:
        return 0.0


def _normalize_det(d: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """Return canonical detection dict or None if invalid.
    Canonical fields:
      object_label: str
      accuracy: float  (alias of score)
      start: {x:int, y:int}
      end:   {x:int, y:int}
    """
    try:
        label = d.get("object_label")
        if label is None:
            # fallback names
            label = d.get("label") or d.get("class_name") or f"class_{int(d.get('class_id', 0))}"
        sc = _score_of(d)
        st = d.get("start") or {}
        ed = d.get("end") or {}
        x1, y1 = int(st.get("x", 0)), int(st.get("y", 0))
        x2, y2 = int(ed.get("x", 0)), int(ed.get("y", 0))
        return {
            "object_label": str(label),
            "accuracy": float(sc),
            "start": {"x": x1, "y": y1},
            "end":   {"x": x2, "y": y2},
        }
    except Exception:
        return None


def _maybe_print(frame_id: int, dets: List[Dict[str, Any]]) -> None:
    if QUIET:
        return

    global _last_print_ts
    now = time.time()

    # time‑based throttle has priority
    if PRINT_EVERY_SECS > 0:
        if _last_print_ts is not None and (now - _last_print_ts) < PRINT_EVERY_SECS:
            return
        _last_print_ts = now
    elif PRINT_EVERY > 1:
        if (frame_id % PRINT_EVERY) != 0:
            return

    if not dets:
        if PRINT_EMPTY:
            print({})
        return

    topk = sorted(dets, key=_score_of, reverse=True)[:max(0, MAX_PRINT_DETS)] if MAX_PRINT_DETS > 0 else dets
    for d in topk:
        out = {
            "object_label": d.get("object_label"),
            "accuracy": round(_score_of(d), 3),
            "start": d.get("start"),
            "end":   d.get("end"),
        }
        print(out)


def _maybe_log_jsonl(payload: Dict[str, Any]) -> None:
    fh = _open_log_if_needed()
    if not fh:
        return
    try:
        # keep it small; only essential fields
        rec = {
            "t": round(time.time(), 3),
            "frame_id": int(payload.get("frame_id", 0)),
            "image_size": payload.get("image_size", {}),
            "detections": [
                _normalize_det(d) for d in (payload.get("detections") or []) if _normalize_det(d)
            ],
        }
        fh.write(json.dumps(rec, ensure_ascii=False) + "\n")
    except Exception:
        pass


def on_frame_dict(payload: Dict[str, Any]) -> None:
    """
    Expected payload shape from object_detection.py:
      {
        "frame_id": int,
        "image_size": {"w": int, "h": int},
        "detections": [
           {"object_label": str, "accuracy"|"score": float,
            "start": {"x": int, "y": int}, "end": {"x": int, "y": int}}
        ]
      }
    """
    global _last_payload
    with _lock:
        _last_payload = payload

    dets = payload.get("detections", []) or []
    # normalize only for printing/logging (we keep original payload as‑is)
    norm = [_normalize_det(d) for d in dets]
    norm = [d for d in norm if d]

    _maybe_print(int(payload.get("frame_id") or 0), norm)
    _maybe_log_jsonl(payload)


# ---------- helpers for other modules/tests ----------

def get_last_payload() -> Optional[Dict[str, Any]]:
    with _lock:
        return _last_payload.copy() if isinstance(_last_payload, dict) else None


def get_last_detections() -> List[Dict[str, Any]]:
    p = get_last_payload() or {}
    dets = p.get("detections", []) or []
    return dets if isinstance(dets, list) else []


@atexit.register
def _print_last_summary():
    if QUIET:
        return
    p = get_last_payload()
    if not p:
        return
    dets = p.get("detections", []) or []
    print("\n[summary at exit] frame:", p.get("frame_id"), "detections:", len(dets))
