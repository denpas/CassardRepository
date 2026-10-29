import sys, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from shapely.geometry import MultiPolygon
import board as B

def draw(ax, geom, **kw):
    gs = list(geom.geoms) if hasattr(geom, "geoms") else [geom]
    for g in gs:
        if g.is_empty: continue
        x, y = g.exterior.xy; ax.fill(x, y, **kw)
        for i in g.interiors:
            x, y = i.xy; ax.fill(x, y, color="white")

fig, axs = plt.subplots(1, 2, figsize=(14, 10))
for ax, L, col in ((axs[0], "F", "#c83737"), (axs[1], "B", "#3770c8")):
    feats, pour = B.copper(L)
    draw(ax, pour, color=col, alpha=0.35, lw=0)
    for n, p in feats:
        draw(ax, p, color=col if n != "GND" else "#555", alpha=0.9, lw=0)
    for p in B.pads:
        if not p["smd"]:
            draw(ax, B.drill_poly(p["drill"], p["x"], p["y"]), color="k")
    for v in B.vias:
        draw(ax, B.drill_poly(v["drill"], v["x"], v["y"]), color="k")
    for x, y, d in B.holes:
        draw(ax, B.drill_poly(d, x, y), color="k")
    if L == "F":
        for pt in B.parts:
            ax.text(pt["x"], pt["y"], pt["ref"], fontsize=7, ha="center", color="navy")
        for p in B.pads:
            if p["ref"] in ("J4", "J1"):
                ax.text(p["x"], p["y"], p["net"], fontsize=5, color="k")
    ax.plot([0, B.W, B.W, 0, 0], [0, 0, B.H, B.H, 0], "k-")
    ax.set_aspect("equal"); ax.set_title(L); ax.grid(True, lw=0.3)
plt.savefig(sys.argv[1] if len(sys.argv) > 1 else "/tmp/claude-0/preview.png", dpi=110, bbox_inches="tight")
