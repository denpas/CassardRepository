"""Écriture des Gerber RS-274X (X2) + Excellon à partir de board.py"""
import os, datetime
from shapely.geometry import Polygon, MultiPolygon, LineString, Point, box
from shapely.ops import unary_union
from shapely import affinity
from matplotlib.textpath import TextPath
from matplotlib.font_manager import FontProperties
import board as B

NAME = "SERVO_I2C_NANO"
REV = "1.1"
OUT = os.path.join(os.path.dirname(__file__), "..", "fabrication", "gerber")
FONT = FontProperties(family="DejaVu Sans", weight="bold")

# ------------------------------------------------------------- texte
def text_poly(s, x, y, size=1.1, rot=0, anchor="c", mirror=False):
    tp = TextPath((0, 0), s, size=size, prop=FONT)
    rings = [Polygon(r) for r in tp.to_polygons() if len(r) >= 3]
    g = Polygon()
    for r in rings:
        if r.is_valid and r.area > 0:
            g = g.symmetric_difference(r)
    g = g.buffer(0.02, 4)
    minx, miny, maxx, maxy = g.bounds
    cx = {"c": (minx + maxx) / 2, "l": minx, "r": maxx}[anchor]
    g = affinity.translate(g, -cx, -(miny + maxy) / 2)
    if mirror:
        g = affinity.scale(g, -1, 1, origin=(0, 0))
    g = affinity.rotate(g, rot, origin=(0, 0))
    return affinity.translate(g, x, y)

def line(*pts, w=0.15):
    return LineString(pts).buffer(w / 2, 8)

def rect_outline(x0, y0, x1, y1, w=0.15):
    return line((x0, y0), (x1, y0), (x1, y1), (x0, y1), (x0, y0), w=w)

# ------------------------------------------------------------- sérigraphie
def silk_top():
    s = []
    T = lambda *a, **k: s.append(text_poly(*a, **k))
    # Arduino Nano : contour + repères
    s.append(rect_outline(0.73, 0.2, 18.51, 43.18))
    T("ARDUINO NANO", 7.0, 21.5, 1.4, rot=90)
    T("USB", 9.62, 1.6, 1.1)
    s.append(line((7.4, 1.7), (8.0, 0.7))); s.append(line((8.6, 1.7), (8.0, 0.7)))   # flèche
    T("U1", 9.62, 40.6, 1.1)
    T("1", 3.7, 39.37, 0.9)
    # titre
    T("SERVO-I2C NANO", 13.3, 47.7, 1.25)
    T(f"Rev {REV}  2026-10", 13.6, 45.9, 0.95)
    # alimentation
    T("SERVO PWR", 24.50, 43.4, 0.9)
    T("4.8-8.4V", 24.50, 42.1, 0.9)
    T("J1", 25.70, 46.8, 0.9)
    s.append(line((35.40, 39.0), (36.50, 39.0)))               # + centre symbole
    s.append(rect_outline(B.Jx - 4.5, B.Jy - 0.8, B.Jx + 4.5, B.H - 0.2))
    # servos
    T("SERVO1", 26.60, 11.55, 0.95)
    T("SERVO2", 26.60, 17.45, 0.95)
    for row in B.SERVO_ROWS.values():
        for x, lab in zip(B.SXS, ["S", "+", "-"]):
            T(lab, x, row - 1.55, 0.95)
        s.append(rect_outline(B.SXS[0] - 1.35, row - 1.35, B.SXS[2] + 1.35, row + 1.35, 0.12))
    # I2C
    for y, lab in zip(B.XH_Y, ["GND", "5V", "SDA", "SCL"]):
        T(lab, 28.25, y, 0.75)
    T("I2C J4", 26.10, 30.25, 0.85)
    s.append(rect_outline(B.XHX - 2.35, B.XH_Y[0] - 2.45, B.XHX + 3.4, B.XH_Y[3] + 2.45, 0.12))
    # C1
    s.append(Point(B.C1x, B.C1y).buffer(3.25, 32).exterior.buffer(0.075, 8))
    T("+", B.C1x + 2.9, B.C1y + 2.7, 1.2)
    T("C1", B.C1x - 0.1, 6.3, 0.85)
    # petits composants
    T("Q1", 35.90, 31.0, 0.8)
    T("R1", 30.00, 31.4, 0.75)
    T("D1", 31.90, 28.2, 0.75)
    s.append(line((30.85, 26.25), (30.85, 27.75), w=0.2))     # barre cathode D1
    T("JP1", 32.10, 24.9, 0.75)
    T("ACT", 31.6, 20.55, 0.75)
    T("PWR", 35.80, 15.7, 0.7)
    T("R5", 22.50, 12.55, 0.7)
    T("R6", 22.50, 13.25, 0.7)
    T("C2", 24.30, 35.0, 0.75)
    T("JP2", 10.4, 26.8, 0.75)
    T("R3", 13.5, 20.4, 0.7)
    T("R4", 13.5, 25.35, 0.7)
    return unary_union(s)

def silk_bottom():
    s = []
    T = lambda txt, x, y, size=1.0: s.append(text_poly(txt, x, y, size, mirror=True, anchor="c"))
    lines = [("SERVO-I2C NANO", 1.15), (f"Rev {REV} - 2026", 0.85), ("", 0),
             ("J1 servos 4.8-8.4V", 0.85), ("centre +", 0.85),
             ("J2/J3 : S + -", 0.85), ("S1=A0  S2=A1", 0.85),
             ("J4 : GND 5V SDA SCL", 0.85), ("JP1 : Nano par J1", 0.85),
             ("(VSERVO <= 5.5V)", 0.85), ("JP2 : pull-ups 4k7", 0.85),
             ("Defaut : Nano", 0.85), ("alim. par J4 +5V", 0.85)]
    y = 37.0
    for txt, sz in lines:
        if txt:
            T(txt, 9.62, y, sz)
        y -= 2.0
    return unary_union(s)

# ------------------------------------------------------------- couches
def mask_openings(layer):
    g = []
    for p in B.pads:
        if p["smd"] and layer == "B":
            continue
        g.append(B.pad_poly(p, 0.05))
    for x, y, d in B.holes:
        g.append(Point(x, y).buffer(d / 2 + 0.1, B.RES))
    return unary_union(g)

def paste():
    return unary_union([B.pad_poly(p) for p in B.pads if p["smd"]])

def copper_geom(layer):
    feats, pour = B.copper(layer)
    return unary_union([p for _, p in feats] + [pour])

# ------------------------------------------------------------- writer
def fmt(v):
    return f"{int(round(v * 1e6))}"

def polys_of(g):
    if g.is_empty:
        return []
    if isinstance(g, Polygon):
        return [g]
    return [p for p in getattr(g, "geoms", []) if isinstance(p, Polygon)]

def region(ring):
    c = list(ring.coords)
    out = ["G36*", f"X{fmt(c[0][0])}Y{fmt(c[0][1])}D02*"]
    for x, y in c[1:]:
        out.append(f"X{fmt(x)}Y{fmt(y)}D01*")
    out.append("G37*")
    return out

def write_gerber(fname, function, geom, polarity="Positive", outline=False):
    now = datetime.datetime(2026, 10, 27, 9, 0).isoformat()
    L = [f"%TF.GenerationSoftware,Claude,servo_i2c_gen,{REV}*%",
         f"%TF.CreationDate,{now}*%",
         f"%TF.ProjectId,{NAME},0001,rev{REV}*%",
         f"%TF.SameCoordinates,Original*%",
         f"%TF.FileFunction,{function}*%",
         f"%TF.FilePolarity,{polarity}*%",
         "%FSLAX46Y46*%", "%MOMM*%", "G04 Generated for SERVO-I2C-NANO*",
         "%ADD10C,0.100000*%", "%LPD*%", "G01*", "D10*"]
    if outline:
        c = [(0, 0), (B.W, 0), (B.W, B.H), (0, B.H), (0, 0)]
        L.append(f"X{fmt(c[0][0])}Y{fmt(c[0][1])}D02*")
        for x, y in c[1:]:
            L.append(f"X{fmt(x)}Y{fmt(y)}D01*")
    else:
        ps = sorted(polys_of(geom), key=lambda p: -Polygon(p.exterior).area)
        for p in ps:
            p = p.simplify(0.002)
            L.append("%LPD*%")
            L += region(p.exterior)
            if p.interiors:
                L.append("%LPC*%")
                for r in p.interiors:
                    L += region(r)
        L.append("%LPD*%")
    L.append("M02*")
    with open(os.path.join(OUT, fname), "w") as f:
        f.write("\n".join(L) + "\n")

def write_drill(fname, items, plated):
    tools = {}
    for d, x, y, slot in items:
        tools.setdefault(d, []).append((x, y, slot))
    L = ["M48", f"; {NAME} rev {REV} - {'PTH' if plated else 'NPTH'}",
         "; FORMAT={-:-/ absolute / metric / decimal}",
         f"; #@! TF.FileFunction,{'Plated' if plated else 'NonPlated'},1,2,{'PTH' if plated else 'NPTH'}",
         "FMAT,2", "METRIC"]
    keys = sorted(tools)
    for i, d in enumerate(keys, 1):
        L.append(f"T{i}C{d:.3f}")
    L += ["%", "G90", "G05"]
    for i, d in enumerate(keys, 1):
        L.append(f"T{i}")
        for x, y, slot in tools[d]:
            if slot:
                (x1, y1), (x2, y2) = slot
                L.append(f"X{x1:.3f}Y{y1:.3f}G85X{x2:.3f}Y{y2:.3f}")
            else:
                L.append(f"X{x:.3f}Y{y:.3f}")
    L += ["T0", "M30"]
    with open(os.path.join(OUT, fname), "w") as f:
        f.write("\n".join(L) + "\n")

def drill_items():
    pth, npth = [], []
    for p in B.pads:
        if p["smd"]:
            continue
        d = p["drill"]
        if isinstance(d, tuple):
            dx, dy = d
            w = min(dx, dy); l = max(dx, dy) - w
            if dx >= dy:
                slot = ((p["x"] - l / 2, p["y"]), (p["x"] + l / 2, p["y"]))
            else:
                slot = ((p["x"], p["y"] - l / 2), (p["x"], p["y"] + l / 2))
            pth.append((w, p["x"], p["y"], slot))
        else:
            pth.append((d, p["x"], p["y"], None))
    for v in B.vias:
        pth.append((v["drill"], v["x"], v["y"], None))
    for x, y, d in B.holes:
        npth.append((d, x, y, None))
    return pth, npth

def build():
    os.makedirs(OUT, exist_ok=True)
    clip_top = B.outline.buffer(-0.2).difference(mask_openings("F").buffer(0.12))
    clip_bot = B.outline.buffer(-0.2).difference(mask_openings("B").buffer(0.12))
    st = silk_top().intersection(clip_top)
    sb = silk_bottom().intersection(clip_bot)
    write_gerber(f"{NAME}-F_Cu.gtl", "Copper,L1,Top", copper_geom("F"))
    write_gerber(f"{NAME}-B_Cu.gbl", "Copper,L2,Bot", copper_geom("B"))
    write_gerber(f"{NAME}-F_Mask.gts", "Soldermask,Top", mask_openings("F"), "Negative")
    write_gerber(f"{NAME}-B_Mask.gbs", "Soldermask,Bot", mask_openings("B"), "Negative")
    write_gerber(f"{NAME}-F_Paste.gtp", "Paste,Top", paste())
    write_gerber(f"{NAME}-F_Silkscreen.gto", "Legend,Top", st)
    write_gerber(f"{NAME}-B_Silkscreen.gbo", "Legend,Bot", sb)
    write_gerber(f"{NAME}-Edge_Cuts.gm1", "Profile,NP", None, outline=True)
    pth, npth = drill_items()
    write_drill(f"{NAME}-PTH.drl", pth, True)
    write_drill(f"{NAME}-NPTH.drl", npth, False)
    return st, sb

if __name__ == "__main__":
    build()
    print("\n".join(sorted(os.listdir(OUT))))
