"""BOM (nomenclature) et fichier de placement (CPL) au format JLCPCB"""
import csv, os
import board as B

OUT = os.path.join(os.path.dirname(__file__), "..", "fabrication")
os.makedirs(os.path.join(OUT, "assembly"), exist_ok=True)

# lignes complémentaires (non placées sur la carte)
extra = [dict(ref="U1-SOCK", value="Barrette femelle 1x15 2,54 mm (x2)", fp="PinSocket_1x15_P2.54mm_Vertical",
              smd=False, lcsc="C2977589 (à vérifier)", desc="Support du Nano (hauteur 8,5 mm)", mfr="Générique")]

def groups():
    g = {}
    for p in B.parts + extra:
        key = (p["value"], p["fp"])
        g.setdefault(key, []).append(p)
    return g

def write():
    # BOM complète
    rows = []
    with open(os.path.join(OUT, "assembly", "BOM_complete.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, delimiter=";")
        w.writerow(["Repères", "Qté", "Valeur", "Empreinte", "Montage", "Référence fabricant", "LCSC", "Description"])
        for (val, fp), ps in sorted(groups().items(), key=lambda kv: kv[1][0]["ref"]):
            refs = ",".join(sorted(p["ref"] for p in ps))
            qty = 2 if refs == "U1-SOCK" else len(ps)
            mont = "Aucun (pont à souder)" if fp.startswith("SolderJumper") else ("CMS" if ps[0]["smd"] else "Traversant")
            row = [refs, qty, val, fp, mont, ps[0]["mfr"], ps[0]["lcsc"], ps[0]["desc"]]
            w.writerow(row); rows.append(row)
    # BOM JLCPCB (CMS uniquement)
    with open(os.path.join(OUT, "assembly", "BOM_JLCPCB.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["Comment", "Designator", "Footprint", "LCSC Part #"])
        for (val, fp), ps in groups().items():
            if ps[0]["smd"] and not fp.startswith("SolderJumper"):
                w.writerow([val, ",".join(sorted(p["ref"] for p in ps)), fp, ps[0]["lcsc"].split()[0]])
    # CPL JLCPCB
    with open(os.path.join(OUT, "assembly", "CPL_JLCPCB.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
        for p in B.parts:
            if p["smd"] and not p["fp"].startswith("SolderJumper"):
                w.writerow([p["ref"], f"{p['x']:.3f}mm", f"{p['y']:.3f}mm", "Top", p["rot"]])
    return rows

if __name__ == "__main__":
    for r in write():
        print(r)
