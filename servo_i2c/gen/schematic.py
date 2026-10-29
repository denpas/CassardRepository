"""Schéma électrique SERVO-I2C-NANO - dessin IEC sur format A4 paysage (matplotlib)"""
import os, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Polygon as MPoly, Circle, FancyArrowPatch

OUT = os.path.join(os.path.dirname(__file__), "..", "docs", "img")
os.makedirs(OUT, exist_ok=True)
LW = 1.1
FS = 7.5

fig = plt.figure(figsize=(11.69, 8.27))
ax = fig.add_axes([0, 0, 1, 1]); ax.set_xlim(0, 297); ax.set_ylim(0, 210); ax.axis("off")

def W(*pts, lw=LW, c="k"):
    xs, ys = zip(*pts); ax.plot(xs, ys, color=c, lw=lw, solid_capstyle="round")

def dot(x, y): ax.add_patch(Circle((x, y), 0.9, color="k"))

def txt(x, y, s, fs=FS, ha="center", va="center", **k): ax.text(x, y, s, fontsize=fs, ha=ha, va=va, **k)

def res_v(x, y1, y2, ref, val, side="r"):
    """résistance verticale IEC entre y1 (haut) et y2 (bas)"""
    m = (y1 + y2) / 2
    W((x, y1), (x, m + 4)); W((x, m - 4), (x, y2))
    ax.add_patch(Rectangle((x - 1.6, m - 4), 3.2, 8, fill=False, lw=LW))
    if side == "r": txt(x + 3, m, f"{ref}\n{val}", ha="left")
    else: txt(x - 3, m, f"{ref}\n{val}", ha="right")

def res_h(x1, x2, y, ref, val):
    m = (x1 + x2) / 2
    W((x1, y), (m - 4, y)); W((m + 4, y), (x2, y))
    ax.add_patch(Rectangle((m - 4, y - 1.6), 8, 3.2, fill=False, lw=LW))
    txt(m, y + 3.6, f"{ref} {val}")

def cap_v(x, y1, y2, ref, val, polar=False):
    m = (y1 + y2) / 2
    W((x, y1), (x, m + 1)); W((x, m - 1), (x, y2))
    W((x - 3, m + 1), (x + 3, m + 1)); W((x - 3, m - 1), (x + 3, m - 1), lw=2.2 if polar else LW)
    if polar: txt(x - 4.2, m + 3, "+", fs=8)
    txt(x + 4, m, f"{ref}\n{val}", ha="left")

def diode_shape(x, y, d, led=False):
    """diode orientée selon d ('down' ou 'right'), anode en amont"""
    if d == "down":
        ax.add_patch(MPoly([(x - 2.5, y + 2), (x + 2.5, y + 2), (x, y - 2)], closed=True, fill=False, lw=LW))
        W((x - 2.5, y - 2), (x + 2.5, y - 2))
        if led:
            for o in (0, 2.2):
                ax.add_patch(FancyArrowPatch((x + 3, y + 1 - o), (x + 6, y + 3 - o), arrowstyle="-|>", mutation_scale=6, lw=0.8))
    else:
        ax.add_patch(MPoly([(x - 2, y + 2.5), (x - 2, y - 2.5), (x + 2, y)], closed=True, fill=False, lw=LW))
        W((x + 2, y - 2.5), (x + 2, y + 2.5))

def gnd(x, y):
    W((x, y), (x, y - 2))
    for i, w in enumerate((3, 2, 1)):
        W((x - w, y - 2 - i * 1.1), (x + w, y - 2 - i * 1.1))

def tag(x, y, s, d="r"):
    """étiquette de net ; d = côté vers lequel pointe l'étiquette"""
    w = 3.0 + 1.75 * len(s)
    if d == "r":
        pts = [(x, y), (x + 2, y + 2), (x + w, y + 2), (x + w, y - 2), (x + 2, y - 2)]
        tx = x + 2 + (w - 2) / 2
    else:
        pts = [(x, y), (x - 2, y + 2), (x - w, y + 2), (x - w, y - 2), (x - 2, y - 2)]
        tx = x - 2 - (w - 2) / 2
    ax.add_patch(MPoly(pts, closed=True, fill=False, lw=0.9, ec="#1f4e9e"))
    txt(tx, y, s, fs=6.5, color="#1f4e9e")

def jumper_v(x, y1, y2, ref, note):
    m = (y1 + y2) / 2
    W((x, y1), (x, m + 1.8)); W((x, m - 1.8), (x, y2))
    ax.add_patch(matplotlib.patches.Wedge((x, m + 1.8), 2.2, 180, 360, fill=False, lw=LW))
    ax.add_patch(matplotlib.patches.Wedge((x, m - 1.8), 2.2, 0, 180, fill=False, lw=LW))
    txt(x + 4, m, f"{ref}\n{note}", ha="left")

def jumper_h(x1, x2, y, ref, note):
    m = (x1 + x2) / 2
    W((x1, y), (m - 1.8, y)); W((m + 1.8, y), (x2, y))
    ax.add_patch(matplotlib.patches.Wedge((m - 1.8, y), 2.2, 270, 90, fill=False, lw=LW))
    ax.add_patch(matplotlib.patches.Wedge((m + 1.8, y), 2.2, 90, 270, fill=False, lw=LW))
    txt(m, y + 4.5, f"{ref} {note}")

def connector(x, y_top, pins, ref, desc, side="r", pitch=6):
    """boîte connecteur ; pins = [(num, nom)] ; renvoie positions des broches"""
    h = pitch * len(pins) + 2
    ax.add_patch(Rectangle((x, y_top - h), 16, h, fill=False, lw=LW))
    txt(x + 8, y_top + 5.5, f"{ref}", fs=8.5, weight="bold")
    txt(x + 8, y_top + 2.3, desc, fs=6)
    pos = {}
    for i, (n, name) in enumerate(pins):
        py = y_top - 4 - i * pitch
        if side == "r":
            W((x + 16, py), (x + 20, py)); txt(x + 15, py, name, ha="right", fs=6.5); txt(x + 17.5, py + 1.6, n, fs=5.5)
            pos[n] = (x + 20, py)
        else:
            W((x - 4, py), (x, py)); txt(x + 1, py, name, ha="left", fs=6.5); txt(x - 1.8, py + 1.6, n, fs=5.5)
            pos[n] = (x - 4, py)
    return pos

def block(x, y, w, h, title):
    ax.add_patch(Rectangle((x, y), w, h, fill=False, lw=0.7, ls=(0, (4, 3)), ec="#777"))
    txt(x + 2, y + h - 3.5, title, ha="left", fs=8.5, weight="bold", color="#333")

# cadre
ax.add_patch(Rectangle((5, 5), 287, 200, fill=False, lw=1.4))

# =============== 1. ALIMENTATION SERVOS =============== #
block(9, 112, 140, 89, "1 - ALIMENTATION SERVOS (indépendante du Nano)")
j1 = connector(14, 178, [("1", "+ centre"), ("2", "- manchon"), ("3", "contact SW")], "J1", "Jack DC 5,5x2,1 mm")
# J1 masse
W(j1["2"], (40, j1["2"][1])); W(j1["3"], (40, j1["3"][1])); W((40, j1["2"][1]), (40, j1["3"][1])); dot(40, j1["2"][1])
gnd(40, j1["3"][1])
# Q1 P-MOSFET (drain gauche, source droite, grille en bas)
yw = j1["1"][1]
W(j1["1"], (58, yw)); txt(46, yw + 2.5, "VIN_RAW", fs=6)
W((58, yw), (62, yw), (62, yw - 4)); W((78, yw - 4), (78, yw), (90, yw))
for a, b in ((62, 66.5), (68, 72), (73.5, 78)): W((a, yw - 4), (b, yw - 4), lw=2)
W((70, yw - 4), (70, yw - 1.5), (78, yw - 1.5))
ax.add_patch(FancyArrowPatch((70, yw - 4.2), (70, yw - 1.3), arrowstyle="-|>", mutation_scale=7, lw=0.8))
W((62, yw - 6.5), (78, yw - 6.5)); W((70, yw - 6.5), (70, yw - 12))
txt(70, yw + 4.5, "Q1  AO3401A (P-MOSFET)", fs=6.5)
txt(61, yw - 2, "D", fs=5.5); txt(80, yw - 2, "S", fs=5.5); txt(72, yw - 9, "G", fs=5.5)
txt(70, yw - 33, "Protection contre\nl'inversion de polarité", fs=6, style="italic", color="#555")
res_v(70, yw - 12, yw - 26, "R1", "10k")
gnd(70, yw - 26)
dot(90, yw)
W((90, yw), (125, yw)); tag(125, yw, "VSERVO")
cap_v(95, yw, yw - 22, "C1", "470µF 10V", polar=True); gnd(95, yw - 22); dot(95, yw)
res_v(112, yw, yw - 14, "R2", "1k")
diode_shape(112, yw - 19, "down", led=True); W((112, yw - 14), (112, yw - 17)); W((112, yw - 21), (112, yw - 25))
txt(119, yw - 19, "D2 LED\nverte", ha="left"); gnd(112, yw - 25); dot(112, yw)
txt(79, 118, "Entrée 4,8 à 8,4 V (selon servos), 2,5 A max (jack)", fs=6.5)

# =============== 2. SORTIES SERVOS =============== #
block(153, 112, 135, 89, "2 - SORTIES SERVOS")
for i, (yt, mcu, ref, rr, sig) in enumerate([(182, "A0", "J2  SERVO 1", "R5", "SERVO1_SIG"), (146, "A1", "J3  SERVO 2", "R6", "SERVO2_SIG")]):
    pj = connector(250, yt, [("1", "S (signal)"), ("2", "+ VSERVO"), ("3", "- GND")], ref, "barrette 1x3 2,54", side="l")
    tag(170, pj["1"][1], f"Nano {mcu}", d="l")
    res_h(170, 220, pj["1"][1], rr, "220R"); W((220, pj["1"][1]), pj["1"]); txt(234, pj["1"][1] + 2.2, sig, fs=5.5)
    tag(232, pj["2"][1], "VSERVO", d="l"); W((232, pj["2"][1]), pj["2"])
    W(pj["3"], (240, pj["3"][1])); gnd(240, pj["3"][1])
txt(220, 118, "Ordre standard Futaba/JR : signal - plus - moins", fs=6.5)

# =============== 3. BUS I2C =============== #
block(9, 9, 140, 99, "3 - BUS I2C (esclave) ET ALIMENTATION LOGIQUE")
pj4 = connector(14, 82, [("1", "GND"), ("2", "+5V"), ("3", "SDA"), ("4", "SCL")], "J4", "JST XH 4 pts, pas 2,5")
W(pj4["1"], (38, pj4["1"][1])); gnd(38, pj4["1"][1])
y5 = pj4["2"][1]; ysda = pj4["3"][1]; yscl = pj4["4"][1]
W(pj4["2"], (120, y5)); tag(120, y5, "+5V")
W(pj4["3"], (120, ysda)); tag(120, ysda, "SDA")
W(pj4["4"], (120, yscl)); tag(120, yscl, "SCL")
# pull-ups
for x, yy, ref in ((82, ysda, "R3"), (96, yscl, "R4")):
    dot(x, yy); res_v(x, 90, 74, ref, "4k7", side="l" if ref == "R3" else "r")
W((82, 74), (82, ysda)); W((96, 74), (96, yscl))
W((82, 90), (82, 93), (96, 93), (96, 90)); dot(89, 93)
W((89, 93), (89, 95))
jumper_v(89, 95, 99.5, "", "")
txt(93, 97.2, "JP2 (ouvert) : active les pull-ups 4k7", ha="left", fs=6)
W((89, 99.5), (89, 101)); tag(89, 101, "+5V")
# option JP1
tag(26, 24, "VSERVO", d="l"); W((26, 24), (40, 24)); diode_shape(44, 24, "right"); W((46, 24), (58, 24))
txt(44, 28.5, "D1 B5819W", fs=6.5)
jumper_h(58, 76, 24, "JP1", "(ouvert)"); W((76, 24), (84, 24)); tag(84, 24, "+5V")
txt(79, 14.5, "Par défaut le Nano est alimenté par le +5V du bus (J4).\nJP1 fermé : alimentation par J1 via D1 - uniquement si VSERVO <= 5,5 V\net sans +5V présent sur J4.", fs=6, color="#9e1f1f")

# =============== 4. NANO =============== #
block(153, 9, 135, 99, "4 - MICROCONTRÔLEUR")
ux, uy, uw, uh = 205, 30, 40, 62
ax.add_patch(Rectangle((ux, uy), uw, uh, fill=False, lw=1.4))
txt(ux + uw / 2, uy + uh + 6, "U1  Arduino Nano v3", fs=8.5, weight="bold")
txt(ux + uw / 2, uy + uh + 2.5, "ATmega328P - 16 MHz - logique 5 V", fs=6)
left = [("19", "A0", "Nano A0"), ("20", "A1", "Nano A1"), ("23", "A4 / SDA", "SDA"), ("24", "A5 / SCL", "SCL")]
for i, (n, name, net) in enumerate(left):
    py = uy + uh - 8 - i * 10
    W((ux - 6, py), (ux, py)); txt(ux + 1.5, py, name, ha="left", fs=7); txt(ux - 3, py + 1.8, n, fs=5.5)
    tag(ux - 6, py, net, d="l")
# LED d'activité sur A3
py = uy + uh - 8 - 4 * 10
W((ux - 6, py), (ux, py)); txt(ux + 1.5, py, "A3", ha="left", fs=7); txt(ux - 3, py + 1.8, "22", fs=5.5)
W((ux - 6, py), (190, py)); res_h(170, 190, py, "R7", "1k")
W((170, py), (163, py), (163, py - 4)); diode_shape(163, py - 7, "down", led=True)
W((163, py - 9), (163, py - 12)); gnd(163, py - 12)
txt(171, py - 8, "D3 LED jaune « ACT »\nflash à chaque commande servo", ha="left", fs=6)
right = [("27", "+5V", "+5V"), ("4, 29", "GND", None), ("30", "VIN", "nc"), ("", "USB", "programmation")]
for i, (n, name, net) in enumerate(right):
    py = uy + uh - 8 - i * 10
    W((ux + uw, py), (ux + uw + 6, py)); txt(ux + uw - 1.5, py, name, ha="right", fs=7); txt(ux + uw + 3, py + 1.8, n, fs=5.5)
    if net == "+5V":
        W((ux + uw + 6, py), (276, py)); dot(268, py); tag(276, py, "+5V")
        cap_v(268, py, py - 14, "C2", "10µF"); gnd(268, py - 14)
    elif net is None: gnd(ux + uw + 6, py)
    else: txt(ux + uw + 7, py, net, ha="left", fs=6, style="italic")
txt(220, 17, "Autres broches non connectées (disponibles sur le Nano).\nLe Nano est enfiché sur 2 barrettes femelles 1x15.", fs=6, color="#555")

# cartouche
ax.add_patch(Rectangle((200, 5), 92, 0.01, lw=0))
ax.add_patch(Rectangle((153, 5), 0.01, 0.01, lw=0))
ax.add_patch(Rectangle((186, 191.5), 102, 0.01, lw=0))
ax.text(150, 203.5, "SERVO-I2C NANO - Schéma électrique - Rév. 1.1 - 27/10/2026 - Feuille 1/1",
        fontsize=9, ha="center", va="center", weight="bold")

fig.savefig(os.path.join(OUT, "schematic.png"), dpi=220)
fig.savefig(os.path.join(OUT, "schematic.pdf"))
fig.savefig(os.path.join(OUT, "schematic.svg"))
print("ok")
