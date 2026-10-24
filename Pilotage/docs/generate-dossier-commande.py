#!/usr/bin/env python3
"""Build the editable, widescreen Cassard control-system presentation."""

from __future__ import annotations

from pathlib import Path
from xml.sax.saxutils import escape
from zipfile import ZIP_DEFLATED, ZipFile


OUTPUT = Path(__file__).resolve().parent / "Dossier commande.pptx"
EMU = 914400
SLIDE_W = 13_335_000
SLIDE_H = 7_500_000
P = "http://schemas.openxmlformats.org/presentationml/2006/main"
A = "http://schemas.openxmlformats.org/drawingml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
REL = "http://schemas.openxmlformats.org/package/2006/relationships"
CT = "http://schemas.openxmlformats.org/package/2006/content-types"

NAVY = "10233F"
NAVY2 = "19375C"
BLUE = "2474A6"
SKY = "6AA4C7"
GOLD = "E6A744"
INK = "18283B"
MUTED = "718096"
LINE = "E1E8EF"
PAPER = "FFFFFF"
CANVAS = "F3F6F9"
PALE_BLUE = "EAF3F8"
PALE_GOLD = "FFF5E3"
PALE_GREEN = "EAF5EF"
GREEN = "23835B"
RED = "B84A4A"
FONT = "Aptos"


def tag(ns: str, name: str) -> str:
    return f"{{{ns}}}{name}"


def xml_bytes(element) -> bytes:
    import xml.etree.ElementTree as ET
    return ET.tostring(element, encoding="utf-8", xml_declaration=True)


def inches(value: float) -> int:
    return round(value * EMU)


class Slide:
    def __init__(self, title: str, chapter: str, number: int, section: str = "DOSSIER DE COMMANDE"):
        self.title = title
        self.chapter = chapter
        self.number = number
        self.section = section
        self.items: list[str] = []
        self.shape_id = 2
        self.add_shape(0, 0, 13.333, 7.5, CANVAS, name="Fond")
        if number != 1:
            self.add_shape(0, 0, 13.333, 0.82, NAVY, name="Bandeau")
            self.add_shape(0.46, 0.19, 0.42, 0.42, GOLD, radius=True, name="Marque")
            self.add_text(0.46, 0.19, 0.42, 0.4, "C", 17, NAVY, True, align="ctr", valign="ctr")
            self.add_text(1.0, 0.2, 2.0, 0.19, "CASSARD", 11, PAPER, True)
            self.add_text(1.0, 0.42, 2.0, 0.15, "SYSTÈME DE PILOTAGE", 7, "B9C9DA", True)
            self.add_text(10.5, 0.27, 2.35, 0.18, chapter.upper(), 8, "D7E1ED", True, align="r")
            self.add_text(0.65, 1.04, 9.3, 0.43, title, 25, NAVY, True)
            self.add_shape(0.65, 7.06, 12.03, 0.012, LINE)
            self.add_text(0.67, 7.14, 8.5, 0.15, "CASSARD  ·  PILOTAGE EMBARQUÉ  ·  OCTOBRE 2026", 7, MUTED, True)
            self.add_text(12.0, 7.13, 0.6, 0.16, f"{number:02d}", 8, BLUE, True, align="r")
            self.add_shape(0.65, 7.035, 12.03 * number / 10, 0.035, BLUE, radius=True)

    def add_shape(
        self,
        x: float,
        y: float,
        w: float,
        h: float,
        fill: str | None,
        *,
        line: str | None = None,
        radius: bool = False,
        ellipse: bool = False,
        name: str = "Forme",
        rotation: int = 0,
        transparency: int | None = None,
    ) -> None:
        shape_id = self.shape_id
        self.shape_id += 1
        geom = "ellipse" if ellipse else ("roundRect" if radius else "rect")
        rotation_attr = f' rot="{rotation * 60000}"' if rotation else ""
        xfrm = f'<a:xfrm{rotation_attr}><a:off x="{inches(x)}" y="{inches(y)}"/><a:ext cx="{inches(w)}" cy="{inches(h)}"/></a:xfrm>'
        if fill:
            alpha_xml = f'<a:alpha val="{100-transparency}000"/>' if transparency else ""
            fill_xml = f'<a:solidFill><a:srgbClr val="{fill}">{alpha_xml}</a:srgbClr></a:solidFill>'
        else:
            fill_xml = "<a:noFill/>"
        line_xml = f'<a:ln w="12700"><a:solidFill><a:srgbClr val="{line}"/></a:solidFill></a:ln>' if line else "<a:ln><a:noFill/></a:ln>"
        self.items.append(
            f'<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="{escape(name)}"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr>{xfrm}<a:prstGeom prst="{geom}"><a:avLst/></a:prstGeom>{fill_xml}{line_xml}</p:spPr></p:sp>'
        )

    def add_text(
        self,
        x: float,
        y: float,
        w: float,
        h: float,
        value: str,
        size: float = 12,
        color: str = INK,
        bold: bool = False,
        *,
        name: str = "Texte",
        align: str = "l",
        valign: str = "t",
        margin: float = 0.02,
    ) -> None:
        shape_id = self.shape_id
        self.shape_id += 1
        paragraphs = []
        for line in value.split("\n"):
            paragraphs.append(
                f'<a:p><a:pPr algn="{align}" marL="0" indent="0"/>'
                f'<a:r><a:rPr lang="fr-FR" sz="{round(size * 100)}" b="{1 if bold else 0}">'
                f'<a:solidFill><a:srgbClr val="{color}"/></a:solidFill>'
                f'<a:latin typeface="{FONT}"/><a:ea typeface="{FONT}"/><a:cs typeface="{FONT}"/>'
                f'</a:rPr><a:t xml:space="preserve">{escape(line)}</a:t></a:r>'
                f'<a:endParaRPr lang="fr-FR" sz="{round(size * 100)}"/></a:p>'
            )
        inset = inches(margin)
        self.items.append(
            f'<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="{escape(name)}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{inches(x)}" y="{inches(y)}"/>'
            f'<a:ext cx="{inches(w)}" cy="{inches(h)}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/><a:ln><a:noFill/></a:ln></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="{inset}" rIns="{inset}" tIns="{inset}" bIns="{inset}" anchor="{valign}"/>'
            f'<a:lstStyle/>{"".join(paragraphs)}</p:txBody></p:sp>'
        )

    def add_line(self, x: float, y: float, w: float, h: float, color: str = LINE, thickness: float = 0.018) -> None:
        self.add_shape(x, y, max(w, thickness), max(h, thickness), color, name="Ligne")

    def add_card(self, x: float, y: float, w: float, h: float, *, fill: str = PAPER, outline: str = LINE) -> None:
        self.add_shape(x, y, w, h, fill, line=outline, radius=True, name="Panneau")

    def add_label(self, x: float, y: float, value: str, w: float = 3.0, color: str = BLUE) -> None:
        self.add_text(x, y, w, 0.18, value.upper(), 8, color, True)

    def add_bullet(self, x: float, y: float, value: str, *, w: float = 5.2, color: INK, size: float = 12) -> None:
        self.add_shape(x, y + 0.11, 0.075, 0.075, GOLD, ellipse=True, name="Repère")
        self.add_text(x + 0.2, y, w - 0.2, 0.43, value, size, color)

    def to_xml(self) -> bytes:
        inner = "".join(self.items)
        xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
            '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/>'
            '<p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr><a:xfrm>'
            '<a:off x="0" y="0"/><a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/>'
            f'<a:chExt cx="{SLIDE_W}" cy="{SLIDE_H}"/></a:xfrm></p:grpSpPr>{inner}'
            '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'
        )
        return xml.encode("utf-8")


def page_card(slide: Slide, x: float, y: float, index: str, title: str, desc: str, accent: str = BLUE) -> None:
    slide.add_card(x, y, 3.82, 1.52)
    slide.add_label(x + 0.2, y + 0.17, index, color=accent)
    slide.add_text(x + 0.2, y + 0.43, 3.42, 0.33, title, 15, NAVY, True)
    slide.add_text(x + 0.2, y + 0.88, 3.4, 0.45, desc, 10, MUTED)


def cover_slide() -> Slide:
    s = Slide("Dossier de commande", "Présentation", 1)
    s.add_shape(0, 0, 13.333, 7.5, NAVY)
    s.add_shape(0.7, 0.7, 0.12, 5.95, GOLD)
    s.add_shape(1.08, 0.7, 0.42, 0.42, GOLD, radius=True)
    s.add_text(1.08, 0.7, 0.42, 0.4, "C", 17, NAVY, True, align="ctr", valign="ctr")
    s.add_text(1.65, 0.74, 2.1, 0.22, "CASSARD", 12, PAPER, True)
    s.add_text(1.65, 0.99, 2.4, 0.18, "SYSTÈME DE PILOTAGE", 8, "B9C9DA", True)
    s.add_text(1.12, 2.08, 8.2, 0.64, "Dossier de commande", 35, PAPER, True)
    s.add_text(1.15, 2.96, 8.2, 0.52, "Architecture · interfaces · protocole", 20, "C7D7E7")
    s.add_text(1.15, 3.66, 7.3, 0.65, "Une console de pilotage embarquée, conçue autour\nd'un contrôle clair et d'une architecture modulaire.", 13, "D7E1ED")
    s.add_shape(1.15, 5.08, 2.42, 0.44, GOLD, radius=True)
    s.add_text(1.27, 5.2, 2.2, 0.2, "RÉVISION 2.0  ·  OCTOBRE 2026", 9, NAVY, True, align="ctr")
    s.add_text(1.15, 6.58, 8.5, 0.2, "PROPULSION  /  DIRECTION  /  TOURELLE  /  SUPERVISION", 8, "B9C9DA", True)
    s.add_shape(9.8, 2.35, 2.35, 0.08, GOLD)
    s.add_shape(10.25, 2.45, 1.5, 0.08, "7AA1BD")
    s.add_shape(10.56, 2.56, 0.92, 0.08, "7AA1BD")
    s.add_shape(10.86, 2.66, 0.36, 0.08, "7AA1BD")
    s.add_shape(9.55, 2.42, 0.42, 0.28, GOLD, radius=True)
    s.add_shape(10.25, 2.04, 0.34, 0.29, BLUE, radius=True)
    s.add_shape(10.37, 1.77, 0.08, 0.35, "B9C9DA")
    for index in range(5):
        s.add_shape(9.1 + index * 0.62, 5.2, 0.035, 1.2, "294769")
    s.add_text(9.15, 6.55, 3.2, 0.2, "DOCUMENTATION TECHNIQUE", 8, "B9C9DA", True)
    return s


def overview_slide() -> Slide:
    s = Slide("Un pilotage modulaire, une commande unifiée", "Vue d'ensemble", 2)
    cards = [
        ("3", "cartes", "Un contrôleur central et deux modules dédiés"),
        ("2", "esclaves I²C", "Propulsion (2) et tourelle (1)"),
        ("7", "écrans", "De la connexion à la console de diagnostic"),
        ("0", "télémétrie", "Pas de retour de mesures des modules à l'interface"),
    ]
    for i, (number, title, detail) in enumerate(cards):
        x = 0.66 + i * 3.04
        s.add_card(x, 1.78, 2.82, 1.27)
        s.add_text(x + 0.2, 1.96, 0.72, 0.6, number, 31, BLUE if i < 3 else GOLD, True)
        s.add_text(x + 0.93, 2.02, 1.65, 0.3, title, 13, NAVY, True)
        s.add_text(x + 0.93, 2.39, 1.65, 0.44, detail, 9, MUTED)

    s.add_card(0.66, 3.34, 7.66, 2.92)
    s.add_label(0.91, 3.57, "Chaîne de commande")
    nodes = [
        (0.96, "OPÉRATEUR", "Navigateur"),
        (3.08, "CENTRAL", "PHPoC + Uno"),
        (5.83, "BUS I²C", "Routage adressé"),
    ]
    for x, head, title in nodes:
        s.add_card(x, 4.15, 1.74, 0.88, fill=PALE_BLUE, outline="D5E5EF")
        s.add_text(x + 0.11, 4.3, 1.5, 0.17, head, 8, BLUE, True, align="ctr")
        s.add_text(x + 0.11, 4.59, 1.5, 0.22, title, 11, NAVY, True, align="ctr")
    s.add_text(2.77, 4.42, 0.26, 0.2, "→", 19, GOLD, True, align="ctr")
    s.add_text(5.04, 4.42, 0.37, 0.2, "→", 19, GOLD, True, align="ctr")
    s.add_text(1.02, 5.35, 6.9, 0.55, "WebSocket text.phpoc\nTrames validées puis transmises au module destinataire", 10, MUTED, align="ctr")

    s.add_card(8.58, 3.34, 4.1, 2.92, fill=NAVY, outline=NAVY)
    s.add_label(8.9, 3.62, "Périmètre de cette version", color=GOLD)
    s.add_text(8.9, 4.07, 3.38, 1.55, "Pilotage moteur et gouvernail\nCommande des axes de tourelle\nConsole de diagnostic\nInterface web responsive", 12, PAPER)
    s.add_text(8.9, 5.7, 3.3, 0.38, "Sons/lumières : commandes non implémentées", 9, "E9C98A", True)
    return s


def architecture_slide() -> Slide:
    s = Slide("Architecture technique", "Système", 3)
    s.add_text(0.68, 1.56, 11.8, 0.35, "Les trames du navigateur sont validées par le central, puis routées vers le module I²C concerné.", 12, MUTED)
    s.add_card(0.72, 2.33, 2.56, 2.18)
    s.add_label(0.96, 2.61, "01 · Interface")
    s.add_text(0.96, 2.96, 2.05, 0.61, "Console web\nPHP / HTML / CSS / JS", 17, NAVY, True)
    s.add_text(0.96, 3.8, 2.02, 0.5, "WebSocket\n/_Cassard · text.phpoc", 10, MUTED)

    s.add_card(4.08, 2.13, 3.14, 2.58, fill=NAVY, outline=NAVY)
    s.add_label(4.39, 2.48, "02 · Nœud central", color=GOLD)
    s.add_text(4.39, 2.93, 2.56, 0.72, "PHPoC + Arduino Uno", 19, PAPER, True)
    s.add_text(4.39, 3.85, 2.54, 0.54, "Parse · valide · convertit\net route les commandes", 11, "D7E1ED")

    s.add_card(8.07, 2.22, 4.48, 0.83, fill=PALE_BLUE, outline="D5E5EF")
    s.add_label(8.34, 2.39, "03 · I²C  adresse 2")
    s.add_text(8.34, 2.62, 3.94, 0.29, "Module propulsion · deux moteurs + direction", 11, NAVY, True)
    s.add_card(8.07, 3.37, 4.48, 0.83, fill=PALE_BLUE, outline="D5E5EF")
    s.add_label(8.34, 3.54, "03 · I²C  adresse 1")
    s.add_text(8.34, 3.77, 3.94, 0.29, "Module tourelle · azimut + élévation", 11, NAVY, True)

    s.add_line(3.28, 3.4, 0.8, 0.022, GOLD)
    s.add_text(3.42, 3.08, 0.62, 0.25, "WS", 8, BLUE, True, align="ctr")
    s.add_line(7.22, 3.01, 0.85, 0.022, GOLD)
    s.add_line(7.22, 3.02, 0.022, 0.77, GOLD)
    s.add_line(7.22, 3.79, 0.85, 0.022, GOLD)
    s.add_text(7.35, 2.72, 0.61, 0.2, "I²C", 8, BLUE, True)
    s.add_text(7.35, 4.02, 0.61, 0.2, "I²C", 8, BLUE, True)

    s.add_card(0.72, 5.05, 11.83, 1.24, fill=PALE_GOLD, outline="F0D4A6")
    s.add_label(1.0, 5.26, "Limite de retour")
    s.add_text(1.0, 5.58, 11.15, 0.48, "Le canal applicatif est actuellement orienté commande : aucune mesure/actionneur ni confirmation I²C ne remonte aux écrans.", 12, "795016", True)
    return s


def joystick_slide() -> Slide:
    s = Slide("Pupitre propulsion & direction", "Interface opérateur", 4)
    s.add_text(0.68, 1.55, 11.9, 0.33, "Joystick combiné au centre · poussée sur le curseur bas · gouvernail sur le curseur latéral droit.", 11, MUTED)

    s.add_card(0.68, 2.02, 8.65, 4.48)
    s.add_label(0.97, 2.28, "COMMANDE COMBINÉE")
    s.add_text(0.97, 2.55, 7.7, 0.37, "Maintenez le joystick dans la direction souhaitée", 16, NAVY, True)
    s.add_text(4.63, 3.01, 0.72, 0.19, "AVANT", 8, MUTED, True, align="ctr")
    s.add_text(4.59, 4.84, 0.82, 0.19, "ARRIÈRE", 8, MUTED, True, align="ctr")

    s.add_shape(2.15, 3.55, 5.69, 1.48, "E8EEF3", radius=True)
    s.add_shape(2.43, 3.43, 5.12, 1.32, "FAFCFE", radius=True, line="C9D6E0")
    for radius in (0.45, 0.83, 1.2):
        s.add_shape(5.0 - radius, 4.06 - radius * 0.24, radius * 2, radius * 0.48, None, line="C7D8E5", ellipse=True)
    s.add_line(5.0, 3.66, 0.015, 0.92, "D2DCE5", 0.015)
    s.add_line(3.6, 4.12, 2.8, 0.015, "D2DCE5", 0.015)

    s.add_shape(4.5, 4.23, 1.0, 0.28, "CCD6DE", radius=True, rotation=-20)
    s.add_shape(4.66, 3.73, 0.62, 0.9, "95A8B6", radius=True, rotation=-20)
    s.add_shape(4.67, 3.67, 0.62, 0.9, "EAF0F5", radius=True, rotation=-20)
    s.add_shape(4.65, 3.12, 0.72, 0.72, "2474A6", radius=True, line="1B5C87", ellipse=True)
    s.add_shape(4.79, 3.25, 0.18, 0.1, "FFFFFF", ellipse=True)
    s.add_text(2.87, 5.12, 4.55, 0.25, "Glisser · relâcher pour arrêter et recentrer", 9, MUTED, align="ctr")

    s.add_card(0.93, 5.51, 8.14, 0.72)
    s.add_text(1.14, 5.7, 1.45, 0.2, "Puissance moteur", 11, NAVY, True)
    s.add_text(1.14, 5.94, 0.65, 0.14, "−100 %", 7, MUTED, True)
    s.add_text(8.12, 5.94, 0.65, 0.14, "+100 %", 7, MUTED, True, align="r")
    s.add_line(2.0, 5.98, 6.47, 0.02, LINE)
    s.add_line(5.0, 5.98, 0.78, 0.025, BLUE)
    s.add_shape(5.67, 5.87, 0.22, 0.22, BLUE, ellipse=True)

    s.add_card(9.58, 2.02, 2.97, 4.48)
    s.add_label(9.88, 2.29, "GOUVERNAIL")
    s.add_text(9.88, 2.55, 2.3, 0.31, "Direction", 17, NAVY, True)
    s.add_text(9.88, 2.94, 2.12, 0.22, "0° · axe central", 10, BLUE, True)
    s.add_text(9.91, 3.43, 0.74, 0.17, "TRIBORD", 8, MUTED, True)
    s.add_text(11.62, 3.43, 0.52, 0.17, "+90°", 8, MUTED, True, align="r")
    s.add_shape(11.01, 3.71, 0.08, 1.8, "E7EDF3", radius=True)
    s.add_shape(11.01, 4.45, 0.08, 0.48, BLUE, radius=True)
    s.add_shape(10.88, 4.35, 0.35, 0.35, PAPER, line=BLUE, ellipse=True)
    s.add_text(11.62, 4.42, 0.52, 0.17, "0°", 8, MUTED, True, align="r")
    s.add_text(11.62, 5.35, 0.52, 0.17, "−90°", 8, MUTED, True, align="r")
    s.add_text(9.91, 5.81, 2.2, 0.35, "Synchronisé avec\nl'axe horizontal du joystick", 8, MUTED)

    s.add_shape(0.95, 6.3, 0.11, 0.11, GOLD, ellipse=True)
    s.add_text(1.17, 6.27, 2.8, 0.18, "Réactivité moteur : niveau 0–5", 8, INK, True)
    s.add_text(6.1, 6.27, 5.9, 0.18, "Utilisation souris, tactile ou flèches clavier · valeurs synchronisées", 8, MUTED, align="r")
    return s


def propulsion_slide() -> Slide:
    s = Slide("Commandes de propulsion", "Propulsion", 5)
    cards = [
        ("AVANCER", "12", "2,12,puissance,255", "Puissance : 0–100"),
        ("RECULER", "13", "2,13,puissance,255", "Puissance : 0–100"),
        ("ARRÊTER", "14", "2,14,255", "Arrêt des moteurs"),
        ("TRIBORD", "15", "2,15,angle,255", "Angle : 0–90°"),
        ("BÂBORD", "16", "2,16,angle,255", "Angle : 0–90°"),
        ("RECENTRER", "17", "2,17,255", "Gouvernail à l'axe"),
        ("RÉACTIVITÉ", "18", "2,18,niveau,255", "Niveau proportionnel : 0–5"),
    ]
    for i, (title, code, frame, info) in enumerate(cards):
        col, row = i % 2, i // 2
        x, y = 0.68 + col * 4.12, 1.72 + row * 1.06
        s.add_card(x, y, 3.88, 0.88)
        s.add_shape(x + 0.2, y + 0.2, 0.49, 0.49, PALE_BLUE, radius=True)
        s.add_text(x + 0.2, y + 0.2, 0.49, 0.45, code, 12, BLUE, True, align="ctr", valign="ctr")
        s.add_text(x + 0.82, y + 0.14, 2.75, 0.22, title, 10, NAVY, True)
        s.add_text(x + 0.82, y + 0.41, 2.75, 0.18, frame, 9, BLUE, True)
        s.add_text(x + 0.82, y + 0.65, 2.75, 0.16, info, 8, MUTED)

    s.add_card(9.07, 1.72, 3.47, 3.99, fill=NAVY, outline=NAVY)
    s.add_label(9.37, 2.02, "MÉLANGE PROPORTIONNEL", color=GOLD)
    s.add_text(9.37, 2.49, 2.8, 0.87, "Direction + puissance\n→ répartition moteurs", 17, PAPER, True)
    s.add_text(9.37, 3.7, 2.65, 0.7, "Le module applique une correction aux moteurs gauche et droit, selon l'angle et le niveau choisis.", 10, "D7E1ED")
    s.add_shape(9.39, 4.73, 2.72, 0.012, "3A5674")
    s.add_text(9.37, 4.94, 2.8, 0.38, "TRAMES TERMINÉES PAR 255 + CRLF", 8, GOLD, True)

    s.add_card(0.68, 5.08, 7.99, 1.09, fill=PALE_GREEN, outline="D2E8D9")
    s.add_label(0.94, 5.3, "Comportement au relâchement")
    s.add_text(0.94, 5.58, 7.4, 0.36, "Joystick au neutre → arrêt moteur (14) + recentrage direction (17).", 11, GREEN, True)
    s.add_text(0.72, 6.46, 11.4, 0.28, "Les curseurs indépendants déclenchent les mêmes commandes; l'écran regroupe brièvement les changements simultanés.", 9, MUTED)
    return s


def turret_slide() -> Slide:
    s = Slide("Tourelle & instruments", "Supervision", 6)
    s.add_card(0.68, 1.8, 5.72, 4.54)
    s.add_label(1.0, 2.08, "MODULE 1 · COMMANDE DE POSITION")
    s.add_text(1.0, 2.45, 4.8, 0.4, "Deux axes, une consigne cohérente", 18, NAVY, True)
    s.add_text(1.0, 2.98, 4.78, 0.58, "Azimut : 0–180°\nÉlévation : 0–60°", 12, INK)
    s.add_text(1.0, 3.89, 4.74, 0.75, "À chaque déplacement, la page envoie azimut et élévation dans la même trame, pour conserver la valeur de l'axe immobile.", 10, MUTED)
    s.add_shape(1.0, 4.97, 2.14, 0.74, PALE_BLUE, radius=True)
    s.add_text(1.18, 5.07, 1.83, 0.17, "AZIMUT", 8, BLUE, True)
    s.add_text(1.18, 5.31, 1.77, 0.22, "0–180°", 13, NAVY, True)
    s.add_shape(3.42, 4.97, 2.13, 0.74, PALE_BLUE, radius=True)
    s.add_text(3.6, 5.07, 1.82, 0.17, "ÉLÉVATION", 8, BLUE, True)
    s.add_text(3.6, 5.31, 1.81, 0.22, "0–60°", 13, NAVY, True)

    s.add_card(6.66, 1.8, 5.88, 4.54)
    s.add_label(6.96, 2.08, "INSTRUMENTS · 5 INDICATEURS")
    names = [("MOTEUR G.", "3 240", "tr/min"), ("ARBRE G.", "1 180", "tr/min"), ("PUISSANCE", "62", "%"), ("ARBRE D.", "1 165", "tr/min"), ("MOTEUR D.", "3 210", "tr/min")]
    for index, (name, value, unit) in enumerate(names):
        x = 6.96 + (index % 3) * 1.72
        y = 2.54 + (index // 3) * 1.11
        s.add_card(x, y, 1.56, 0.88, fill=CANVAS, outline=LINE)
        s.add_text(x + 0.09, y + 0.11, 1.4, 0.16, name, 7, MUTED, True)
        s.add_text(x + 0.09, y + 0.33, 1.4, 0.26, value, 17, NAVY, True)
        s.add_text(x + 0.09, y + 0.66, 1.4, 0.13, unit, 7, BLUE, True)
    s.add_card(6.97, 4.96, 5.22, 0.91, fill=PALE_GOLD, outline="F0D4A6")
    s.add_text(7.15, 5.11, 4.82, 0.62, "Valeurs indicatives uniquement : les modules ne renvoient actuellement aucune télémétrie vers l'interface.", 10, "795016", True)
    s.add_text(6.96, 6.02, 5.18, 0.18, "Fumée et son de tourelle : sorties réservées, non implémentées.", 8, MUTED)
    return s


def hardware_slide() -> Slide:
    s = Slide("Modules & raccordements", "Électronique", 7)
    s.add_text(0.68, 1.54, 11.7, 0.28, "Broches relevées dans les firmwares actuels · confirmer le matériel et les limites mécaniques avant essai.", 10, MUTED)
    module_data = [
        ("CENTRAL", "Arduino Uno + PHPoC", "Serveur WebSocket\nMaître I²C"),
        ("PROPULSION · I²C 2", "Arduino Nano", "Moteurs continus ×2\nServo de direction"),
        ("TOURELLE · I²C 1", "Arduino Nano", "Servo azimut\nServo élévation"),
    ]
    for index, (label, board, roles) in enumerate(module_data):
        x = 0.68 + index * 4.12
        s.add_card(x, 1.95, 3.86, 1.0, fill=NAVY if index == 0 else PAPER, outline=NAVY if index == 0 else LINE)
        txt = PAPER if index == 0 else NAVY
        s.add_label(x + 0.21, 2.12, label, color=GOLD if index == 0 else BLUE)
        s.add_text(x + 0.21, 2.4, 3.37, 0.26, board + "  ·  " + roles.replace("\n", " / "), 9, txt, True)

    s.add_card(0.68, 3.25, 5.83, 2.7)
    s.add_label(0.96, 3.52, "PROPULSION")
    propulsion_rows = [
        ("Moteur gauche", "DIR 9  ·  PWM 3"),
        ("Moteur droit", "DIR 12  ·  PWM 5"),
        ("Servo direction", "14"),
        ("LED état OK / erreur", "16 / 15"),
    ]
    for i, (name, pin) in enumerate(propulsion_rows):
        y = 3.99 + i * 0.43
        s.add_text(0.98, y, 2.7, 0.24, name, 10, INK)
        s.add_text(4.05, y, 1.99, 0.24, pin, 10, BLUE, True, align="r")
        if i != 3:
            s.add_line(0.97, y + 0.34, 5.27, 0.01, LINE, 0.01)

    s.add_card(6.81, 3.25, 5.74, 2.7)
    s.add_label(7.09, 3.52, "TOURELLE")
    turret_rows = [
        ("Servo azimut", "14"),
        ("Servo élévation", "13"),
        ("LED état OK / erreur", "6 / 5"),
        ("Son / fumée", "réservés, non implémentés"),
    ]
    for i, (name, pin) in enumerate(turret_rows):
        y = 3.99 + i * 0.43
        s.add_text(7.11, y, 2.8, 0.24, name, 10, INK)
        s.add_text(10.02, y, 2.18, 0.24, pin, 9, BLUE, True, align="r")
        if i != 3:
            s.add_line(7.1, y + 0.34, 5.16, 0.01, LINE, 0.01)

    s.add_card(0.68, 6.16, 11.87, 0.54, fill=PALE_GOLD, outline="F0D4A6")
    s.add_text(0.91, 6.31, 11.4, 0.2, "Sécurité électrique : masse commune, niveaux logiques compatibles, tirages I²C adaptés et alimentation moteurs dédiée.", 9, "795016", True)
    return s


def protocol_slide() -> Slide:
    s = Slide("Protocole de commande", "Échanges", 8)
    s.add_text(0.68, 1.52, 11.5, 0.27, "Format : module,commande,paramètres…,255  ·  trame WebSocket terminée par CRLF", 11, MUTED)
    rows = [
        ("1", "4", "Tourelle · azimut + élévation", "0–180° / 0–60°"),
        ("2", "12", "Propulsion · marche avant", "0–100 %"),
        ("2", "13", "Propulsion · marche arrière", "0–100 %"),
        ("2", "14", "Propulsion · arrêt moteur", "aucun"),
        ("2", "15", "Direction · tribord", "0–90°"),
        ("2", "16", "Direction · bâbord", "0–90°"),
        ("2", "17", "Direction · recentrage", "aucun"),
        ("2", "18", "Propulsion · réactivité", "niveau 0–5"),
    ]
    x0, y0, widths = 0.7, 1.98, [0.77, 0.88, 5.02, 3.14]
    headers = ["MODULE", "CODE", "FONCTION", "PARAMÈTRES VALIDÉS"]
    offsets = [x0]
    for width in widths[:-1]:
        offsets.append(offsets[-1] + width)
    s.add_shape(x0, y0, sum(widths), 0.48, NAVY, radius=True)
    for x, label, width in zip(offsets, headers, widths):
        s.add_text(x + 0.12, y0 + 0.13, width - 0.2, 0.17, label, 7, PAPER, True)
    for index, row in enumerate(rows):
        y = y0 + 0.5 + index * 0.43
        fill = PAPER if index % 2 == 0 else "F7F9FB"
        s.add_shape(x0, y, sum(widths), 0.42, fill, line=LINE)
        for column, (x, value, width) in enumerate(zip(offsets, row, widths)):
            s.add_text(x + 0.12, y + 0.11, width - 0.22, 0.17, value, 9, BLUE if column < 2 else INK, column < 2 or column == 3)

    s.add_card(0.7, 5.99, 5.67, 0.7, fill=PALE_BLUE, outline="D5E5EF")
    s.add_text(0.94, 6.14, 5.15, 0.39, "Exemple propulsion  ·  2,12,60,255\\r\\nExemple tourelle  ·  1,4,120,25,255", 9, NAVY, True)
    s.add_card(6.65, 5.99, 5.91, 0.7, fill=PALE_GREEN, outline="D2E8D9")
    s.add_text(6.9, 6.14, 5.34, 0.39, "Le central rejette commandes inconnues, champs mal formés et valeurs hors plage avant l'envoi I²C.", 9, GREEN, True)
    return s


def screens_slide() -> Slide:
    s = Slide("Écrans de l’application", "Interface logicielle", 9)
    data = [
        ("01 · ACCÈS", "Connexion", "Formulaire visuel; pas d'authentification effective."),
        ("02 · PILOTAGE", "Tableau de bord", "Navigation par modules; mode SETUP signalé."),
        ("03 · COMMANDE", "Propulsion & direction", "Joystick graphique 3D et curseurs synchronisés."),
        ("04 · COMMANDE", "Tourelle 100 mm", "Curseurs d'azimut et d'élévation."),
        ("05 · MESURES", "Instruments", "Cinq indicateurs de démonstration, sans télémétrie."),
        ("06 · ÉQUIPEMENTS", "Sons & lumières", "Écran présent; commandes non disponibles."),
        ("07 · TECHNIQUE", "Diagnostic", "Envoi de trames brutes et historique local."),
    ]
    for index, (eyebrow, title, description) in enumerate(data):
        col = index % 2
        row = index // 2
        x = 0.7 + col * 6.15
        y = 1.73 + row * 1.1
        s.add_card(x, y, 5.88, 0.92)
        s.add_label(x + 0.19, y + 0.12, eyebrow, w=2.5)
        s.add_text(x + 0.19, y + 0.37, 2.4, 0.25, title, 11, NAVY, True)
        s.add_text(x + 2.67, y + 0.31, 2.95, 0.45, description, 8, MUTED)
    s.add_card(0.7, 5.83, 11.93, 0.77, fill=PALE_GOLD, outline="F0D4A6")
    s.add_text(0.95, 6.03, 11.43, 0.32, "Le témoin WebSocket confirme la connexion, pas la bonne exécution mécanique. Sons/lumières et authentification restent à implémenter.", 10, "795016", True)
    return s


def deployment_slide() -> Slide:
    s = Slide("Déploiement & mise en service", "Exploitation", 10)
    steps = [
        ("01", "Compiler", "Module propulsion et tourelle avec l'AVR et CassardControl."),
        ("02", "Installer le central", "Compiler pour la carte PHPoC installée; fournir la bibliothèque fabricant."),
        ("03", "Déployer l’interface", "Copier ModuleCentral/web sur le serveur web du module."),
        ("04", "Vérifier le bus", "Contrôler adresses I²C 1/2, masses, SDA/SCL et alimentation."),
        ("05", "Tester prudemment", "Connexion WebSocket, axes à faible consigne, sans charge mécanique."),
    ]
    for index, (number, title, detail) in enumerate(steps):
        y = 1.88 + index * 0.87
        s.add_shape(0.88, y + 0.05, 0.48, 0.48, BLUE if index < 4 else GREEN, radius=True)
        s.add_text(0.88, y + 0.05, 0.48, 0.43, number, 9, PAPER, True, align="ctr", valign="ctr")
        if index != len(steps) - 1:
            s.add_line(1.11, y + 0.55, 0.025, 0.33, "B9CAD8", 0.025)
        s.add_text(1.62, y + 0.02, 2.43, 0.28, title, 13, NAVY, True)
        s.add_text(4.15, y + 0.01, 6.73, 0.44, detail, 10, MUTED)

    s.add_card(0.7, 6.25, 11.92, 0.54, fill=PALE_GOLD, outline="F0D4A6")
    s.add_text(0.93, 6.4, 11.4, 0.18, "Confirmer les butées, le sens des servos et l'absence d'interférence avec le mécanisme avant toute commande en charge.", 9, "795016", True)
    return s


def limitations_slide() -> Slide:
    s = Slide("État actuel & limites connues", "Synthèse", 11)
    left_items = [
        ("INTERFACE", "Joystick graphique 3D central; curseur de poussée en bas; direction à droite; affichage responsive."),
        ("COMMANDE", "Moteurs, gouvernail et deux axes de tourelle pilotés par WebSocket et I²C."),
        ("SÉCURITÉ LOGICIELLE", "À l'arrêt du joystick, trame arrêt moteur et recentrage direction."),
    ]
    for i, (name, detail) in enumerate(left_items):
        y = 1.85 + i * 1.1
        s.add_card(0.72, y, 5.62, 0.87)
        s.add_label(0.96, y + 0.13, name)
        s.add_text(0.96, y + 0.39, 5.0, 0.37, detail, 9, INK)

    s.add_card(6.69, 1.85, 5.88, 4.12, fill=NAVY, outline=NAVY)
    s.add_label(7.0, 2.14, "POINTS À TRAITER", color=GOLD)
    limitations = [
        "Le formulaire de connexion n’authentifie pas encore l’utilisateur.",
        "Aucun retour télémétrique ni accusé I²C n’atteint les écrans.",
        "Les instruments montrent des valeurs de démonstration.",
        "Les commandes de son et de fumée ne sont pas implémentées.",
        "Les butées logicielles et mécaniques doivent être confirmées.",
    ]
    for i, item in enumerate(limitations):
        y = 2.57 + i * 0.59
        s.add_shape(7.03, y + 0.07, 0.07, 0.07, GOLD, ellipse=True)
        s.add_text(7.25, y, 4.93, 0.47, item, 10, PAPER)

    s.add_card(0.72, 5.58, 5.62, 0.78, fill=PALE_BLUE, outline="D5E5EF")
    s.add_text(0.97, 5.78, 5.0, 0.38, "Documentation détaillée : Pilotage/docs/interface-pilotage.md", 9, BLUE, True)
    s.add_text(0.72, 6.49, 11.1, 0.21, "Cette présentation décrit le comportement logiciel; valider l'ensemble sur le matériel réel avant exploitation.", 9, MUTED, True)
    return s


def slide_xml_relationships() -> bytes:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>'
        '</Relationships>'
    ).encode("utf-8")


def build_package(slides: list[Slide]) -> None:
    import xml.etree.ElementTree as ET

    ET.register_namespace("", REL)
    ET.register_namespace("a", A)
    ET.register_namespace("p", P)
    ET.register_namespace("r", R)

    content_types = ET.Element(tag(CT, "Types"))
    ET.SubElement(content_types, tag(CT, "Default"), {"Extension": "rels", "ContentType": "application/vnd.openxmlformats-package.relationships+xml"})
    ET.SubElement(content_types, tag(CT, "Default"), {"Extension": "xml", "ContentType": "application/xml"})
    overrides = {
        "/ppt/presentation.xml": "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml",
        "/ppt/slideMasters/slideMaster1.xml": "application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml",
        "/ppt/slideLayouts/slideLayout1.xml": "application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml",
        "/ppt/theme/theme1.xml": "application/vnd.openxmlformats-officedocument.theme+xml",
        "/docProps/core.xml": "application/vnd.openxmlformats-package.core-properties+xml",
        "/docProps/app.xml": "application/vnd.openxmlformats-officedocument.extended-properties+xml",
    }
    for index in range(1, len(slides) + 1):
        overrides[f"/ppt/slides/slide{index}.xml"] = "application/vnd.openxmlformats-officedocument.presentationml.slide+xml"
    for part, content_type in overrides.items():
        ET.SubElement(content_types, tag(CT, "Override"), {"PartName": part, "ContentType": content_type})

    root_rels = ET.Element(tag(REL, "Relationships"))
    ET.SubElement(root_rels, tag(REL, "Relationship"), {
        "Id": "rId1",
        "Type": "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
        "Target": "ppt/presentation.xml",
    })
    ET.SubElement(root_rels, tag(REL, "Relationship"), {
        "Id": "rId2",
        "Type": "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties",
        "Target": "docProps/core.xml",
    })
    ET.SubElement(root_rels, tag(REL, "Relationship"), {
        "Id": "rId3",
        "Type": "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties",
        "Target": "docProps/app.xml",
    })

    presentation = ET.Element(tag(P, "presentation"))
    master_list = ET.SubElement(presentation, tag(P, "sldMasterIdLst"))
    ET.SubElement(master_list, tag(P, "sldMasterId"), {"id": "2147483648", tag(R, "id"): "rId1"})
    slide_list = ET.SubElement(presentation, tag(P, "sldIdLst"))
    for index in range(1, len(slides) + 1):
        ET.SubElement(slide_list, tag(P, "sldId"), {"id": str(255 + index), tag(R, "id"): f"rId{index + 1}"})
    ET.SubElement(presentation, tag(P, "sldSz"), {"cx": str(SLIDE_W), "cy": str(SLIDE_H), "type": "screen16x9"})
    ET.SubElement(presentation, tag(P, "notesSz"), {"cx": "6858000", "cy": "9144000"})
    presentation_rels = ET.Element(tag(REL, "Relationships"))
    ET.SubElement(presentation_rels, tag(REL, "Relationship"), {
        "Id": "rId1",
        "Type": "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster",
        "Target": "slideMasters/slideMaster1.xml",
    })
    for index in range(1, len(slides) + 1):
        ET.SubElement(presentation_rels, tag(REL, "Relationship"), {
            "Id": f"rId{index + 1}",
            "Type": "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide",
            "Target": f"slides/slide{index}.xml",
        })

    master = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<p:sldMaster xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
        'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
        'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
        '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>'
        '<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/>'
        f'<a:chExt cx="{SLIDE_W}" cy="{SLIDE_H}"/></a:xfrm></p:grpSpPr></p:spTree></p:cSld>'
        '<p:clrMap accent1="2474A6" accent2="E6A744" accent3="19375C" accent4="23835B" accent5="718096" accent6="6AA4C7" '
        'bg1="FFFFFF" bg2="F3F6F9" dk1="10233F" dk2="18283B" folHlink="7030A0" hlink="2474A6" lt1="FFFFFF" lt2="F3F6F9"/>'
        '<p:sldLayoutIdLst><p:sldLayoutId id="1" r:id="rId1"/></p:sldLayoutIdLst>'
        '<p:txStyles><p:titleStyle/><p:bodyStyle/><p:otherStyle/></p:txStyles></p:sldMaster>'
    ).encode("utf-8")
    master_rels = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>'
        '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>'
        '</Relationships>'
    ).encode("utf-8")
    layout = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<p:sldLayout xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
        'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
        'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" type="blank" preserve="1">'
        '<p:cSld name="Cassard blank"><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/>'
        '<p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr><a:xfrm>'
        '<a:off x="0" y="0"/><a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/>'
        f'<a:chExt cx="{SLIDE_W}" cy="{SLIDE_H}"/></a:xfrm></p:grpSpPr></p:spTree></p:cSld>'
        '<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>'
    ).encode("utf-8")
    layout_rels = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>'
        '</Relationships>'
    ).encode("utf-8")
    theme = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Cassard">'
        '<a:themeElements><a:clrScheme name="Cassard">'
        '<a:dk1><a:srgbClr val="10233F"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>'
        '<a:dk2><a:srgbClr val="18283B"/></a:dk2><a:lt2><a:srgbClr val="F3F6F9"/></a:lt2>'
        '<a:accent1><a:srgbClr val="2474A6"/></a:accent1><a:accent2><a:srgbClr val="E6A744"/></a:accent2>'
        '<a:accent3><a:srgbClr val="19375C"/></a:accent3><a:accent4><a:srgbClr val="23835B"/></a:accent4>'
        '<a:accent5><a:srgbClr val="718096"/></a:accent5><a:accent6><a:srgbClr val="6AA4C7"/></a:accent6>'
        '<a:hlink><a:srgbClr val="2474A6"/></a:hlink><a:folHlink><a:srgbClr val="7030A0"/></a:folHlink>'
        '</a:clrScheme><a:fontScheme name="Cassard">'
        '<a:majorFont><a:latin typeface="Aptos Display"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>'
        '<a:minorFont><a:latin typeface="Aptos"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>'
        '</a:fontScheme><a:fmtScheme name="Cassard"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>'
        '<a:lnStyleLst><a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>'
        '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
        '<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>'
        '</a:fmtScheme></a:themeElements></a:theme>'
    ).encode("utf-8")
    core = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" '
        'xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" '
        'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
        '<dc:title>Dossier de commande — Cassard</dc:title><dc:subject>Système de pilotage embarqué</dc:subject>'
        '<dc:creator>Documentation Cassard</dc:creator><cp:keywords>pilotage; propulsion; tourelle; Arduino; PHPoC</cp:keywords>'
        '<dcterms:created xsi:type="dcterms:W3CDTF">2026-10-24T00:00:00Z</dcterms:created>'
        '</cp:coreProperties>'
    ).encode("utf-8")
    app = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" '
        'xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">'
        '<Application>Microsoft PowerPoint</Application><PresentationFormat>On-screen Show (16:9)</PresentationFormat>'
        f'<Slides>{len(slides)}</Slides><Company>Cassard</Company><AppVersion>16.0000</AppVersion></Properties>'
    ).encode("utf-8")

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(OUTPUT, "w", ZIP_DEFLATED) as archive:
        archive.writestr("[Content_Types].xml", xml_bytes(content_types))
        archive.writestr("_rels/.rels", xml_bytes(root_rels))
        archive.writestr("ppt/presentation.xml", xml_bytes(presentation))
        archive.writestr("ppt/_rels/presentation.xml.rels", xml_bytes(presentation_rels))
        archive.writestr("ppt/slideMasters/slideMaster1.xml", master)
        archive.writestr("ppt/slideMasters/_rels/slideMaster1.xml.rels", master_rels)
        archive.writestr("ppt/slideLayouts/slideLayout1.xml", layout)
        archive.writestr("ppt/slideLayouts/_rels/slideLayout1.xml.rels", layout_rels)
        archive.writestr("ppt/theme/theme1.xml", theme)
        archive.writestr("docProps/core.xml", core)
        archive.writestr("docProps/app.xml", app)
        for index, slide in enumerate(slides, start=1):
            archive.writestr(f"ppt/slides/slide{index}.xml", slide.to_xml())
            archive.writestr(f"ppt/slides/_rels/slide{index}.xml.rels", slide_xml_relationships())


def main() -> None:
    slides = [
        cover_slide(),
        overview_slide(),
        architecture_slide(),
        joystick_slide(),
        propulsion_slide(),
        turret_slide(),
        hardware_slide(),
        protocol_slide(),
        screens_slide(),
        deployment_slide(),
        limitations_slide(),
    ]
    build_package(slides)
    print(f"Présentation 16:9 créée : {OUTPUT} ({len(slides)} diapositives)")


if __name__ == "__main__":
    main()
