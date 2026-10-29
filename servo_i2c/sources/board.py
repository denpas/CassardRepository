"""
Générateur de la carte SERVO-I2C-NANO v1.0
Coordonnées en mm, origine en bas à gauche, vue de dessus (Y vers le haut).
Produit la géométrie (pads, pistes, vias, plans de masse), un DRC, et les
données utilisées par gerbers.py / docs.
"""
import math
from shapely.geometry import Point, LineString, Polygon, box, MultiPolygon
from shapely.ops import unary_union
from shapely import affinity

DX = 3.5                   # décalage du bloc connecteurs (dégagement / Nano)
W, H = 34.0 + DX, 49.0     # dimensions carte
CLEAR = 0.30               # isolement plan de masse
MIN_CLEAR = 0.20           # DRC piste/pad
EDGE_CLEAR = 0.40          # cuivre / bord
RES = 16

# ----------------------------------------------------------------- pads
pads = []      # dict: ref, num, net, x, y, shape, w, h, drill(None|d|(dx,dy)), smd
parts = []     # dict: ref, value, fp, x, y, rot, side, smd, lcsc, desc

def pad(ref, num, net, x, y, shape, w, h, drill=None, smd=False):
    pads.append(dict(ref=ref, num=str(num), net=net, x=x, y=y, shape=shape,
                     w=w, h=h, drill=drill, smd=smd))

def part(ref, value, fp, x, y, rot=0, smd=False, lcsc="", desc="", mfr=""):
    parts.append(dict(ref=ref, value=value, fp=fp, x=x, y=y, rot=rot,
                      smd=smd, lcsc=lcsc, desc=desc, mfr=mfr))

# --- U1 Arduino Nano (sur barrettes femelles 1x15) -----------------------
X0, Y0 = 2.0, 39.37
nano_left = ["D1/TX", "D0/RX", "RST", "GND", "D2", "D3", "D4", "D5", "D6",
             "D7", "D8", "D9", "D10", "D11", "D12"]
nano_right = ["D13", "3V3", "AREF", "A0", "A1", "A2", "A3", "A4", "A5",
              "A6", "A7", "+5V", "RST", "GND", "VIN"]          # pins 16..30
nano_net = {"GND": "GND", "+5V": "+5V", "A0": "SERVO1_MCU", "A1": "SERVO2_MCU", "A3": "LED_ACT",
            "A4": "SDA", "A5": "SCL"}
NANO_PIN = {}
for i, name in enumerate(nano_left):          # pins 1..15, x=0, yk=2.54*i
    num = i + 1
    x, y = X0, Y0 - 2.54 * i
    pad("U1", num, nano_net.get(name, f"NC_U1_{num}"), x, y,
        "rect" if num == 1 else "circle", 1.6, 1.6, 1.0)
    NANO_PIN[name if name not in NANO_PIN else name + "_2"] = (x, y)
for i, name in enumerate(nano_right):         # pin16 at yk=35.56 ... pin30 at 0
    num = 16 + i
    x, y = X0 + 15.24, Y0 - 35.56 + 2.54 * i
    pad("U1", num, nano_net.get(name, f"NC_U1_{num}"), x, y, "circle", 1.6, 1.6, 1.0)
    NANO_PIN[name if name not in NANO_PIN else name + "_2"] = (x, y)
part("U1", "Arduino Nano v3 (ATmega328P)", "2x PinSocket_1x15_P2.54mm",
     X0 + 7.62, Y0 - 17.78, 0, desc="Module Arduino Nano, monté sur 2 barrettes femelles 1x15",
     mfr="Arduino A000005 (ou compatible)")

# --- J1 Jack DC 5,5 x 2,1 mm (CUI PJ-002A), ouverture vers le bord haut -----
Jx, Jy = 28.8, 35.3
pad("J1", 1, "VIN_RAW", Jx, Jy, "rect", 3.5, 3.5, (3.0, 1.0))
pad("J1", 2, "GND", Jx, Jy + 6.0, "rrect", 3.5, 3.0, (3.0, 1.0))
pad("J1", 3, "GND", Jx - 4.7, Jy + 3.0, "rrect", 3.5, 3.5, (1.0, 3.0))
part("J1", "Jack DC 5.5x2.1mm", "BarrelJack_CUI_PJ-002A_Horizontal", Jx, Jy, 90,
     lcsc="C2961147 (PJ-002AH, à vérifier)", desc="Alimentation servos 4,8-8,4 V, centre +",
     mfr="CUI/Same Sky PJ-002A")

# --- J2 / J3 connecteurs servo 1x3 2,54 mm (S, +, -) ------------------------
SERVO_ROWS = {"J2": 9.5, "J3": 15.5}
SXS = [20.6, 23.14, 25.68]
for ref, sy in SERVO_ROWS.items():
    sig = "SERVO1_SIG" if ref == "J2" else "SERVO2_SIG"
    pad(ref, 1, sig, SXS[0], sy, "rect", 1.7, 1.7, 1.0)
    pad(ref, 2, "VSERVO", SXS[1], sy, "circle", 1.7, 1.7, 1.0)
    pad(ref, 3, "GND", SXS[2], sy, "circle", 1.7, 1.7, 1.0)
    part(ref, "Servo " + ("1" if ref == "J2" else "2"), "PinHeader_1x03_P2.54mm_Vertical",
         SXS[1], sy, 0, lcsc="C35464 (à vérifier)", desc="Barrette mâle 1x3, ordre S / + / -",
         mfr="Générique")

# --- J4 bus I2C, JST XH 4 pts vertical (GND, +5V, SDA, SCL) ------------------
XHX = 22.5
XH_Y = [20.5, 23.0, 25.5, 28.0]
for i, (net) in enumerate(["GND", "+5V", "SDA", "SCL"]):
    pad("J4", i + 1, net, XHX, XH_Y[i], "rrect" if i == 0 else "oval", 1.95, 1.7, 0.95)
part("J4", "I2C (GND/+5V/SDA/SCL)", "JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical",
     XHX, sum(XH_Y) / 4, 90, lcsc="C158012 (à vérifier)", desc="Connecteur JST XH 4 points", mfr="JST B4B-XH-A")

# --- C1 470 uF radial 6,3 mm ----------------------------------------------
C1x, C1y = 26.3, 3.8
pad("C1", 2, "GND", C1x - 1.25, C1y, "circle", 1.6, 1.6, 0.8)
pad("C1", 1, "VSERVO", C1x + 1.25, C1y, "rect", 1.6, 1.6, 0.8)
part("C1", "470uF 10V", "CP_Radial_D6.3mm_P2.50mm", C1x, C1y, 180,
     lcsc="C134793 (à vérifier)", desc="Réservoir rail servos, électrolytique 6,3x11 mm", mfr="Générique 105°C low-ESR")

# --- SMD -----------------------------------------------------------------
def r0603(ref, val, x, y, rot, n1, n2, lcsc, desc=""):
    if rot == 0:
        pad(ref, 1, n1, x - 0.825, y, "rrect", 0.8, 0.95, smd=True)
        pad(ref, 2, n2, x + 0.825, y, "rrect", 0.8, 0.95, smd=True)
    else:  # 90 : pad1 en bas
        pad(ref, 1, n1, x, y - 0.825, "rrect", 0.95, 0.8, smd=True)
        pad(ref, 2, n2, x, y + 0.825, "rrect", 0.95, 0.8, smd=True)
    part(ref, val, "R_0603_1608Metric", x, y, rot, True, lcsc, desc, "UNI-ROYAL 0603WAF")

# Q1 P-MOSFET protection inversion (rot 90 : drain vers le haut)
Qx, Qy = 29.8, 31.2
pad("Q1", 1, "Q1_G", Qx - 0.95, Qy - 1.0, "rect", 0.8, 0.9, smd=True)
pad("Q1", 2, "VSERVO", Qx + 0.95, Qy - 1.0, "rect", 0.8, 0.9, smd=True)
pad("Q1", 3, "VIN_RAW", Qx, Qy + 1.0, "rect", 0.8, 0.9, smd=True)
part("Q1", "AO3401A", "SOT-23", Qx, Qy, 90, True, "C15127",
     "P-MOSFET -30V -4A, protection inversion de polarité", "Alpha&Omega AO3401A")
r0603("R1", "10k", 27.2, 30.2, 0, "GND", "Q1_G", "C25804", "Rappel grille Q1")

# D1 Schottky VSERVO -> JP1 (option alim Nano par le rail servo)
pad("D1", 1, "D1_K", 28.4 - 1.65, 27.0, "rect", 0.9, 1.2, smd=True)
pad("D1", 2, "VSERVO", 28.4 + 1.65, 27.0, "rect", 0.9, 1.2, smd=True)
part("D1", "B5819W", "D_SOD-123", 28.4, 27.0, 0, True, "C8598",
     "Schottky 40V 1A (option JP1)", "Changjiang B5819W")
# JP1 cavalier à souder (ouvert par défaut) D1_K <-> +5V, vertical
pad("JP1", 2, "D1_K", 26.75, 25.55, "rrect", 1.5, 1.0, smd=True)
pad("JP1", 1, "+5V", 26.75, 24.25, "rrect", 1.5, 1.0, smd=True)
part("JP1", "SJ ouvert", "SolderJumper-2_P1.3mm_Open", 26.75, 24.9, 90, True, "",
     "Pont à souder : alim. Nano depuis rail servo (via D1)")

# LED témoin rail servo
r0603("R2", "1k", 32.3, 22.0, 90, "LED_A", "VSERVO", "C21190", "Limitation LED")
pad("D2", 1, "GND", 32.3, 19.2 - 0.7875, "rrect", 0.95, 0.875, smd=True)   # cathode en bas
pad("D2", 2, "LED_A", 32.3, 19.2 + 0.7875, "rrect", 0.95, 0.875, smd=True)
part("D2", "LED verte", "LED_0603_1608Metric", 32.3, 19.2, 90, True, "C72043",
     "Témoin présence alim servos", "Hubei KENTO 19-217/GHC")

# Résistances série servos
r0603("R5", "220R", 19.0, 10.6, 90, "SERVO1_SIG", "SERVO1_MCU", "C22962", "Protection sortie servo 1")
r0603("R6", "220R", 19.0, 14.8, 90, "SERVO2_MCU", "SERVO2_SIG", "C22962", "Protection sortie servo 2")

# Pull-ups I2C (sous le Nano) + JP2
r0603("R3", "4k7", 13.5, NANO_PIN["A4"][1], 0, "PU", "SDA", "C23162", "Pull-up SDA (via JP2)")
r0603("R4", "4k7", 13.5, NANO_PIN["A5"][1], 0, "PU", "SCL", "C23162", "Pull-up SCL (via JP2)")
pad("JP2", 1, "PU", 12.675, 26.15, "rrect", 1.5, 1.0, smd=True)
pad("JP2", 2, "+5V", 12.675, 27.45, "rrect", 1.5, 1.0, smd=True)
part("JP2", "SJ ouvert", "SolderJumper-2_P1.3mm_Open", 12.675, 26.8, 90, True, "",
     "Pont à souder : active les pull-ups I2C 4k7")

# C2 découplage +5V
pad("C2", 1, "+5V", 19.85, 33.6, "rrect", 1.0, 1.45, smd=True)
pad("C2", 2, "GND", 21.75, 33.6, "rrect", 1.0, 1.45, smd=True)
part("C2", "10uF 25V X5R", "C_0805_2012Metric", 20.8, 33.6, 0, True, "C15850",
     "Découplage +5V Nano", "Samsung CL21A106KAYNNNE")

# --- trous de fixation M2.5 (NPTH) --------------------------------------------
holes = [(3.2, 46.2, 2.7), (31.8, 2.6, 2.7)]
HOLE_KEEPOUT = 2.6

# ----------------------------------------------------------------- pistes
traces = []    # net, layer, width, pts
vias = []      # net, x, y, drill, dia

def tr(net, w, *pts, layer="F"):
    traces.append(dict(net=net, layer=layer, w=w, pts=list(pts)))

def via(net, x, y):
    vias.append(dict(net=net, x=x, y=y, drill=0.4, dia=0.8))

A0 = NANO_PIN["A0"]; A1 = NANO_PIN["A1"]; A4 = NANO_PIN["A4"]; A5 = NANO_PIN["A5"]
P5V = NANO_PIN["+5V"]

# Puissance servos
tr("VIN_RAW", 1.5, (Qx, Qy + 1.0), (Qx, Jy))
tr("VSERVO", 1.5, (Qx + 0.95, Qy - 1.0), (30.5, Qy - 1.0), (30.5, 7.4), (SXS[1], 7.4),
   (SXS[1], SERVO_ROWS["J3"]))
tr("VSERVO", 1.5, (C1x + 1.25, 7.4), (C1x + 1.25, C1y))
tr("VSERVO", 0.4, (30.5, 22.825), (32.3, 22.825))          # vers R2
tr("LED_A", 0.3, (32.3, 21.175), (32.3, 19.9875))
tr("GND", 0.4, (32.3, 18.4125), (32.3, 17.0)); via("GND", 32.3, 17.0)
tr("Q1_G", 0.4, (28.025, 30.2), (Qx - 0.95, Qy - 1.0))
tr("GND", 0.4, (26.375, 30.2), (26.4, 28.6)); via("GND", 26.4, 28.6)

# Option JP1
tr("D1_K", 0.4, (26.75, 27.0), (26.75, 25.55))
tr("+5V", 0.5, (26.75, 24.25), (25.3, 24.25))

# +5V : J4 -> Nano 5V, C2
tr("+5V", 0.5, (XHX, XH_Y[1]), (25.3, XH_Y[1]), (25.3, P5V[1]), P5V)
tr("+5V", 0.5, (19.85, P5V[1]), (19.85, 33.6))
tr("GND", 0.5, (21.75, 33.6), (21.75, 35.2)); via("GND", 21.75, 35.2)

# I2C
tr("SDA", 0.3, A4, (20.3, A4[1]), (20.3, XH_Y[2]), (XHX, XH_Y[2]))
tr("SCL", 0.3, A5, (19.2, A5[1]), (19.2, XH_Y[3]), (XHX, XH_Y[3]))
tr("SDA", 0.3, A4, (14.325, A4[1]))
tr("SCL", 0.3, A5, (14.325, A5[1]))
tr("PU", 0.3, (12.675, A4[1]), (12.675, 26.15))
tr("+5V", 0.4, (12.675, 27.45), (12.675, P5V[1]), P5V)

# Servos signaux
tr("SERVO1_MCU", 0.3, A0, (19.0, 11.425))
tr("SERVO1_SIG", 0.3, (19.0, 9.775), (SXS[0], SERVO_ROWS["J2"]))
tr("SERVO2_MCU", 0.3, A1, (19.0, 13.975))
tr("SERVO2_SIG", 0.3, (19.0, 15.625), (SXS[0], SERVO_ROWS["J3"]))

# Vias de couture GND (relient les plans haut/bas)
for (x, y) in [(5.0, 2.0), (12.0, 2.0), (19.5, 2.0), (8.5, 5.0), (3.9, 36.0),
               (5.0, 42.0), (13.0, 42.0), (20.0, 45.0), (15.5, 38.0), (33.0, 26.0),
               (33.0, 12.0), (28.5, 12.5), (3.9, 15.0), (3.9, 28.0),
               (23.5, 4.8), (25.6, 21.8)]:
    via("GND", x, y)

# --- LED d'activité (A3) : s'allume à chaque commande transmise à un servo ---
LEDX = 28.1   # coordonnée avant décalage DX
r0603("R7", "1k", LEDX, 18.225, 90, "LED_ACT_A", "LED_ACT", "C21190", "Limitation LED activité")
pad("D3", 1, "GND", LEDX, 15.6 - 0.7875, "rrect", 0.95, 0.875, smd=True)    # cathode en bas
pad("D3", 2, "LED_ACT_A", LEDX, 15.6 + 0.7875, "rrect", 0.95, 0.875, smd=True)
part("D3", "LED jaune", "LED_0603_1608Metric", LEDX, 15.6, 90, True, "C72038 (à vérifier)",
     "Témoin d'activité : commande transmise à un servo", "Hubei KENTO 19-217/Y5C")
tr("LED_ACT", 0.3, NANO_PIN["A3"], (LEDX, NANO_PIN["A3"][1]))
tr("LED_ACT_A", 0.3, (LEDX, 17.4), (LEDX, 15.6 + 0.7875))
tr("GND", 0.4, (LEDX, 15.6 - 0.7875), (LEDX, 13.4)); via("GND", LEDX, 13.4)

# --- décalage du bloc connecteurs vers la droite (tout ce qui est à droite du Nano) ---
def sx(x):
    return x + DX if x > 18.6 else x
for p in pads: p["x"] = sx(p["x"])
for p in parts: p["x"] = sx(p["x"])
for v in vias: v["x"] = sx(v["x"])
for t in traces: t["pts"] = [(sx(x), y) for x, y in t["pts"]]
holes = [(sx(x), y, d) for x, y, d in holes]
Jx, Qx, C1x, XHX, LEDX = sx(Jx), sx(Qx), sx(C1x), sx(XHX), sx(LEDX)
SXS = [sx(x) for x in SXS]

# ---------------------------------------------------------------- géométrie
def pad_poly(p, expand=0.0):
    w, h = p["w"] + 2 * expand, p["h"] + 2 * expand
    if p["shape"] == "circle":
        return Point(p["x"], p["y"]).buffer(w / 2, RES)
    if p["shape"] == "oval":
        r = min(w, h) / 2
        if w >= h:
            ls = LineString([(p["x"] - w / 2 + r, p["y"]), (p["x"] + w / 2 - r, p["y"])])
        else:
            ls = LineString([(p["x"], p["y"] - h / 2 + r), (p["x"], p["y"] + h / 2 - r)])
        return ls.buffer(r, RES)
    b = box(p["x"] - w / 2, p["y"] - h / 2, p["x"] + w / 2, p["y"] + h / 2)
    if p["shape"] == "rrect":
        r = min(w, h) * 0.25
        return b.buffer(-r, RES).buffer(r, RES)
    return b

def trace_poly(t):
    return LineString(t["pts"]).buffer(t["w"] / 2, RES) if len(t["pts"]) > 1 else Point(t["pts"][0]).buffer(t["w"]/2, RES)

def via_poly(v, expand=0.0):
    return Point(v["x"], v["y"]).buffer(v["dia"] / 2 + expand, RES)

def drill_poly(d, x, y):
    if isinstance(d, tuple):
        dx, dy = d
        r = min(dx, dy) / 2
        if dx >= dy:
            return LineString([(x - dx / 2 + r, y), (x + dx / 2 - r, y)]).buffer(r, RES)
        return LineString([(x, y - dy / 2 + r), (x, y + dy / 2 - r)]).buffer(r, RES)
    return Point(x, y).buffer(d / 2, RES)

outline = box(0, 0, W, H)

def features(layer):
    """liste (net, polygon) du cuivre hors plans pour une couche 'F' ou 'B'"""
    out = []
    for p in pads:
        if p["smd"] and layer == "B":
            continue
        out.append((p["net"], pad_poly(p)))
    for t in traces:
        if t["layer"] == layer:
            out.append((t["net"], trace_poly(t)))
    for v in vias:
        out.append((v["net"], via_poly(v)))
    return out

def make_pour(layer, net="GND"):
    feats = features(layer)
    area = outline.buffer(-EDGE_CLEAR)
    keep = []
    for n, poly in feats:
        if n != net:
            keep.append(poly.buffer(CLEAR, RES))
    for (x, y, d) in holes:
        keep.append(Point(x, y).buffer(HOLE_KEEPOUT, RES))
    # dégagement autour des pads GND traversants (thermiques)
    therm = []
    for p in pads:
        if p["net"] == net and not p["smd"]:
            pp = pad_poly(p)
            ring = pp.buffer(CLEAR, RES)
            keep.append(ring)
            # 4 rayons
            sw = 0.5
            L = max(p["w"], p["h"]) / 2 + CLEAR + 0.3
            spokes = unary_union([
                box(p["x"] - sw / 2, p["y"] - L, p["x"] + sw / 2, p["y"] + L),
                box(p["x"] - L, p["y"] - sw / 2, p["x"] + L, p["y"] + sw / 2)])
            therm.append(spokes.intersection(ring.buffer(0.05)))
    pour = area.difference(unary_union(keep))
    pour = unary_union([pour] + [t for t in therm])
    # rayons ne doivent pas toucher d'autres nets
    others = unary_union([poly.buffer(CLEAR, RES) for n, poly in feats if n != net])
    pour = pour.difference(others)
    # garder les îlots reliés à du cuivre GND
    gnd_cu = unary_union([poly for n, poly in feats if n == net])
    polys = list(pour.geoms) if isinstance(pour, MultiPolygon) else [pour]
    kept = [g for g in polys if g.area > 0.5 and g.intersects(gnd_cu)]
    return unary_union(kept)

def copper(layer):
    feats = features(layer)
    pour = make_pour(layer)
    return feats, pour

# ---------------------------------------------------------------- DRC
def drc(verbose=True):
    errors = []
    layer_items = {}
    for L in ("F", "B"):
        feats, pour = copper(L)
        items = feats + [("GND", pour)]
        layer_items[L] = items
        # union par net
        bynet = {}
        for n, p in items:
            bynet.setdefault(n, []).append(p)
        un = {n: unary_union(v) for n, v in bynet.items()}
        nets = list(un)
        for i in range(len(nets)):
            for j in range(i + 1, len(nets)):
                d = un[nets[i]].distance(un[nets[j]])
                if d < MIN_CLEAR - 1e-6:
                    errors.append(f"[{L}] isolement {nets[i]} / {nets[j]} = {d:.3f} mm")
        allcu = unary_union([p for _, p in items])
        if not outline.buffer(-EDGE_CLEAR + 0.05).contains(allcu):
            errors.append(f"[{L}] cuivre trop près du bord")
        for (x, y, d) in holes:
            if allcu.distance(Point(x, y)) < d / 2 + 0.4:
                errors.append(f"[{L}] cuivre trop près du trou ({x},{y})")
    # connectivité : un composant par net (via pads traversants et vias)
    import networkx_lite as nx
    g = nx.Graph()
    nodes = []
    for L, items in layer_items.items():
        for k, (n, p) in enumerate(items):
            polys = list(p.geoms) if isinstance(p, MultiPolygon) else [p]
            for m, pp in enumerate(polys):
                nid = (L, k, m)
                nodes.append((nid, n, pp))
                g.add(nid)
    # liens intra-couche
    for a in range(len(nodes)):
        for b in range(a + 1, len(nodes)):
            ia, na, pa = nodes[a]; ib, nb, pb = nodes[b]
            if ia[0] == ib[0] and na == nb and pa.intersects(pb):
                g.link(ia, ib)
    # liens inter-couches via trous métallisés
    drills = [(p["net"], Point(p["x"], p["y"])) for p in pads if not p["smd"]] + \
             [(v["net"], Point(v["x"], v["y"])) for v in vias]
    for n, pt in drills:
        f = [i for i, nn, pp in nodes if nn == n and i[0] == "F" and pp.contains(pt)]
        b = [i for i, nn, pp in nodes if nn == n and i[0] == "B" and pp.contains(pt)]
        for x in f + b:
            for y in f + b:
                g.link(x, y)
    for net in sorted(set(n for _, n, _ in nodes)):
        if net.startswith("NC_"):
            continue
        ids = [i for i, n, _ in nodes if n == net]
        # ignorer les fragments de pads seuls sur couche B pour nets sans piste B (normal: THT)
        comps = g.components(ids)
        if len(comps) > 1:
            errors.append(f"net {net} non connecté : {len(comps)} groupes")
    # chaque pad SMD/THT de net non-NC doit appartenir au groupe principal (déjà couvert)
    if verbose:
        print("DRC :", "OK" if not errors else f"{len(errors)} erreur(s)")
        for e in errors:
            print("  -", e)
    return errors

if __name__ == "__main__":
    drc()
