"""Rendus réalistes et plan d'assemblage à partir de la géométrie de board.py"""
import os, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle, FancyBboxPatch
from shapely.geometry import Point
from shapely.ops import unary_union
import board as B
import gerbers as G

OUT = os.path.join(os.path.dirname(__file__), "..", "docs", "img")
os.makedirs(OUT, exist_ok=True)

def fill(ax, geom, **kw):
    gs = list(geom.geoms) if hasattr(geom, "geoms") else [geom]
    for g in gs:
        if g.is_empty or g.geom_type != "Polygon":
            continue
        from matplotlib.path import Path
        from matplotlib.patches import PathPatch
        verts, codes = [], []
        for ring in [g.exterior] + list(g.interiors):
            c = list(ring.coords)
            verts += c; codes += [Path.MOVETO] + [Path.LINETO] * (len(c) - 2) + [Path.CLOSEPOLY]
        ax.add_patch(PathPatch(Path(verts, codes), **kw))

def setup(ax, mirror=False):
    ax.set_xlim((B.W + 1, -1) if mirror else (-1, B.W + 1)); ax.set_ylim(-3.2, B.H + 1)
    ax.set_aspect("equal"); ax.axis("off")

def realistic(side):
    L = "F" if side == "top" else "B"
    fig, ax = plt.subplots(figsize=(6, 8.4))
    setup(ax, mirror=(side == "bottom"))
    fill(ax, B.outline, fc="#0b5d2a", ec="none")
    cu = G.copper_geom(L)
    fill(ax, cu, fc="#2f8a4a", ec="none")
    mask = G.mask_openings(L)
    fill(ax, cu.intersection(mask), fc="#d4af37", ec="none")
    silk = G.silk_top() if side == "top" else G.silk_bottom()
    clip = B.outline.buffer(-0.2).difference(mask.buffer(0.12))
    fill(ax, silk.intersection(clip), fc="white", ec="none")
    holes = [B.drill_poly(p["drill"], p["x"], p["y"]) for p in B.pads if not p["smd"]]
    holes += [B.drill_poly(v["drill"], v["x"], v["y"]) for v in B.vias]
    holes += [Point(x, y).buffer(d / 2, 24) for x, y, d in B.holes]
    fill(ax, unary_union(holes), fc="#1a1a1a", ec="none")
    ax.set_title(f"SERVO-I2C NANO - face {'dessus' if side == 'top' else 'dessous'} (vue {'de dessus' if side=='top' else 'de dessous'})", fontsize=9)
    ax.annotate("", xy=(0, -2.4), xytext=(B.W, -2.4), arrowprops=dict(arrowstyle="<->", lw=0.8))
    ax.text(B.W / 2, -3.1, f"{B.W:.1f} mm", ha="center", fontsize=8)
    fig.savefig(os.path.join(OUT, f"pcb_{side}.png"), dpi=220, bbox_inches="tight")
    plt.close(fig)

def assembly():
    fig, ax = plt.subplots(figsize=(7, 9.6))
    setup(ax)
    fill(ax, B.outline, fc="#f4f4f4", ec="black", lw=1)
    for p in B.pads:
        fill(ax, B.pad_poly(p), fc="#c9a227" if p["smd"] else "#b0b0b0", ec="none")
    for x, y, d in B.holes:
        ax.add_patch(Circle((x, y), d / 2, fc="white", ec="black", lw=0.8))
    # contours composants
    ax.add_patch(Rectangle((0.73, 0.0), 17.78, 43.18, fc="none", ec="#1f4e9e", lw=1.2, ls="--"))
    ax.add_patch(Rectangle((7.62 - 0.0 + 0.3, -2.54), 7.62 - 0.6, 10.16 - 2.0, fc="none", ec="#1f4e9e", lw=0.8))
    ax.text(9.6, 22, "U1  Arduino Nano\n(sur barrettes femelles)", rotation=90, ha="center", va="center", color="#1f4e9e", fontsize=8)
    ax.text(11.2, -1.5, "USB", color="#1f4e9e", fontsize=7, ha="center")
    ax.add_patch(Rectangle((B.Jx - 4.5, B.Jy - 0.8), 9, 14.5, fc="none", ec="#9e1f1f", lw=1.2))
    ax.text(B.Jx + 0.2, 44.5, "J1\nJack DC\n5,5x2,1", ha="center", fontsize=7, color="#9e1f1f")
    ax.add_patch(Rectangle((B.XHX - 2.35, B.XH_Y[0] - 2.45), 5.75, 12.4, fc="none", ec="#9e1f1f", lw=1.2))
    ax.text(B.XHX + 0.6, 31.0, "J4 I2C", ha="center", fontsize=7, color="#9e1f1f")
    for ref, row in B.SERVO_ROWS.items():
        ax.add_patch(Rectangle((B.SXS[0] - 1.27, row - 1.27), 7.62, 2.54, fc="none", ec="#9e1f1f", lw=1.0))
        ax.text(B.SXS[1], row + 1.75, f"{ref}  Servo {'1' if ref == 'J2' else '2'}  (S + -)", ha="center", fontsize=6.5, color="#9e1f1f")
    ax.add_patch(Circle((B.C1x, B.C1y), 3.15, fc="none", ec="#9e1f1f", lw=1.0))
    ax.text(B.C1x, B.C1y - 0.1, "C1\n470µF", ha="center", va="center", fontsize=6, color="#9e1f1f")
    ax.text(B.C1x + 2.6, B.C1y + 2.6, "+", fontsize=9, color="#9e1f1f")
    smd_lbl = {"Q1": (36.10, 31.3), "R1": (30.10, 31.5), "D1": (31.90, 28.3), "JP1": (32.20, 24.9),
               "R2": (37.0, 22.0), "D2": (37.0, 19.2), "R7": (30.4, 18.2), "D3": (30.4, 14.0), "R5": (24.50, 12.3), "R6": (24.50, 13.4),
               "R3": (13.5, 20.4), "R4": (13.5, 25.4), "JP2": (10.3, 26.8), "C2": (24.30, 35.2)}
    for ref, (x, y) in smd_lbl.items():
        ax.text(x, y, ref, fontsize=6.5, ha="center", va="center", color="black", weight="bold")
    ax.set_title("Plan d'assemblage - face composants", fontsize=10)
    fig.savefig(os.path.join(OUT, "assembly_top.png"), dpi=220, bbox_inches="tight")
    plt.close(fig)

if __name__ == "__main__":
    realistic("top"); realistic("bottom"); assembly()
    print(os.listdir(OUT))
