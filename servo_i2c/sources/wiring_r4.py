"""Schéma de câblage : Arduino UNO R4 WiFi (maître) -> carte SERVO-I2C NANO (J4)"""
import os, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Rectangle, Circle

OUT = os.path.join(os.path.dirname(__file__), "..", "docs", "img")
os.makedirs(OUT, exist_ok=True)

fig = plt.figure(figsize=(11.69, 6.2))
ax = fig.add_axes([0, 0, 1, 1]); ax.set_xlim(0, 297); ax.set_ylim(0, 158); ax.axis("off")
T = lambda x, y, s, **k: ax.text(x, y, s, ha=k.pop("ha", "center"), va=k.pop("va", "center"), fontsize=k.pop("fs", 8), **k)

ax.text(148.5, 150, "Câblage du maître Arduino UNO R4 WiFi vers la carte SERVO-I2C NANO",
        ha="center", fontsize=11, weight="bold")

# --- UNO R4 WiFi
ax.add_patch(FancyBboxPatch((14, 30), 92, 100, boxstyle="round,pad=0,rounding_size=4", fc="#e8f1fb", ec="#1f4e9e", lw=1.4))
T(60, 122, "Arduino UNO R4 WiFi", fs=10, weight="bold", color="#1f4e9e")
T(60, 116, "RA4M1 (logique 5 V) + ESP32-S3 (WiFi)", fs=7, color="#1f4e9e")
ax.add_patch(Rectangle((14, 92), 10, 12, fc="#c9d3df", ec="#1f4e9e")); T(30, 98, "USB-C", fs=7, ha="left")
ax.add_patch(Rectangle((14, 40), 12, 12, fc="#c9d3df", ec="#1f4e9e")); T(30, 46, "Jack 6-24 V", fs=7, ha="left")
ax.add_patch(Rectangle((44, 62), 36, 22, fc="#d4dbe3", ec="#1f4e9e")); T(62, 73, "Matrice LED\n12 x 8", fs=7)
ax.add_patch(Rectangle((44, 88), 12, 6, fc="#f7e6b5", ec="#9e7a1f")); T(59, 91, "Qwiic 3,3 V : ne pas utiliser", fs=6.5, color="#9e1f1f", ha="left")
# broches
pins = [("SCL", 112), ("SDA", 104), ("5V", 52), ("GND", 44)]
py = {}
for name, y in pins:
    ax.add_patch(Rectangle((100, y - 2.2), 6, 4.4, fc="#333", ec="#333"))
    T(98, y, name, ha="right", fs=8, weight="bold")
    py[name] = (106, y)
T(85, 108.5, "en-tête\nSDA/SCL\n(ou A4/A5)", fs=6, color="#555", ha="right")
T(85, 48, "POWER", fs=6, color="#555", ha="right")

# --- Carte servo
ax.add_patch(FancyBboxPatch((190, 30), 94, 100, boxstyle="round,pad=0,rounding_size=4", fc="#e6f2ea", ec="#0b5d2a", lw=1.4))
T(237, 122, "Carte SERVO-I2C NANO", fs=10, weight="bold", color="#0b5d2a")
T(237, 116, "esclave I2C, adresse 0x40", fs=7, color="#0b5d2a")
ax.add_patch(Rectangle((190, 58), 16, 40, fc="#f4f4f4", ec="#0b5d2a")); T(198, 102, "J4 JST XH", fs=7, weight="bold")
j4 = {"GND": 92, "5V": 84, "SDA": 76, "SCL": 66}
lab = {"GND": "1 GND", "5V": "2 +5V", "SDA": "3 SDA", "SCL": "4 SCL"}
for k, y in j4.items():
    ax.add_patch(Circle((193, y), 1.8, fc="#d9ad55", ec="#7a5a12"))
    T(209, y, lab[k], ha="left", fs=8)
ax.add_patch(Rectangle((236, 70), 34, 22, fc="#dfe9e2", ec="#0b5d2a")); T(253, 81, "Arduino Nano\n(enfiché)", fs=7)
T(237, 60, "JP2 fermé : pull-ups 4k7", fs=7, color="#9e1f1f", weight="bold")
ax.add_patch(Rectangle((266, 38), 18, 10, fc="#c9d3df", ec="#0b5d2a")); T(262, 43, "J1 alim servos\n4,8-8,4 V", fs=6.5, ha="right")
T(225, 36, "J2 / J3 : servos", fs=7)

# --- fils
colors = {"GND": "#222222", "5V": "#d62728", "SDA": "#1f77b4", "SCL": "#e7a414"}
for k, c in colors.items():
    x0, y0 = py[k]; y1 = j4[k]
    xm = 140 + {"GND": 0, "5V": 6, "SDA": 12, "SCL": 18}[k]
    ax.plot([x0, xm, xm, 193], [y0, y0, y1, y1], color=c, lw=2.4, solid_capstyle="round", solid_joinstyle="round")
    T(x0 + 8, y0 + 2.8, k, fs=7, color=c, weight="bold", ha="left")

# --- notes
notes = ("Fil de liaison : câble JST XH 4 points vers fils Dupont mâles, 30 cm max à 100 kHz.\n"
         "Le 5V du R4 alimente le Nano (≈ 30 mA). Les servos restent alimentés par J1 (alimentation séparée).\n"
         "Option une seule alimentation 7-8,4 V : la brancher à la fois sur le jack du R4 et sur J1 de la carte.")
T(148.5, 14, notes, fs=7.5, color="#333")

for ext in ("png", "pdf"):
    fig.savefig(os.path.join(OUT, f"cablage_uno_r4_wifi.{ext}"), dpi=220 if ext == "png" else None)
print("ok")
