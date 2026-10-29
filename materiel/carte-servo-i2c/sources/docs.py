"""Documentation PDF SERVO-I2C NANO"""
import os, csv
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, Image,
                                Table, TableStyle, PageBreak, KeepTogether)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
import board as B

ROOT = os.path.join(os.path.dirname(__file__), "..")
# Documentation de la carte : docs/carte-servo-i2c à la racine du dépôt
DOCS = os.path.join(ROOT, "..", "..", "docs", "carte-servo-i2c")
IMG = os.path.join(DOCS, "img")
def police(nom):
    """Police DejaVu : celle du système (Linux) ou, à défaut, celle fournie par matplotlib (macOS, Windows)."""
    systeme = os.path.join("/usr/share/fonts/truetype/dejavu", nom)
    if os.path.exists(systeme):
        return systeme
    import matplotlib
    return os.path.join(matplotlib.get_data_path(), "fonts", "ttf", nom)


pdfmetrics.registerFont(TTFont("DV", police("DejaVuSans.ttf")))
pdfmetrics.registerFont(TTFont("DVB", police("DejaVuSans-Bold.ttf")))
pdfmetrics.registerFont(TTFont("DVM", police("DejaVuSansMono.ttf")))
from reportlab.pdfbase.pdfmetrics import registerFontFamily
registerFontFamily("DV", normal="DV", bold="DVB", italic="DV", boldItalic="DVB")

ACC = colors.HexColor("#0b5d2a")
st = dict(
    h1=ParagraphStyle("h1", fontName="DVB", fontSize=15, textColor=ACC, spaceBefore=8, spaceAfter=6),
    h2=ParagraphStyle("h2", fontName="DVB", fontSize=11.5, textColor=colors.HexColor("#222"), spaceBefore=8, spaceAfter=4),
    p=ParagraphStyle("p", fontName="DV", fontSize=9.3, leading=13.2, spaceAfter=4),
    small=ParagraphStyle("s", fontName="DV", fontSize=8, leading=10.5, textColor=colors.HexColor("#444")),
    cell=ParagraphStyle("c", fontName="DV", fontSize=8, leading=10),
    cellb=ParagraphStyle("cb", fontName="DVB", fontSize=8, leading=10, textColor=colors.white),
    code=ParagraphStyle("code", fontName="DVM", fontSize=7.8, leading=10, backColor=colors.HexColor("#f2f2f2"),
                        borderPadding=4, spaceAfter=6, leftIndent=4),
    warn=ParagraphStyle("w", fontName="DV", fontSize=9, leading=12.5, backColor=colors.HexColor("#fff4e5"),
                        borderColor=colors.HexColor("#e8a33d"), borderWidth=0.6, borderPadding=6, spaceBefore=4, spaceAfter=8),
    title=ParagraphStyle("t", fontName="DVB", fontSize=26, textColor=ACC, leading=32),
    sub=ParagraphStyle("st", fontName="DV", fontSize=13, textColor=colors.HexColor("#333"), leading=18),
)
P = lambda t, s="p": Paragraph(t, st[s])

def table(rows, widths, header=True):
    data = [[Paragraph(str(c), st["cellb"] if (header and i == 0) else st["cell"]) for c in r] for i, r in enumerate(rows)]
    t = Table(data, colWidths=[w * mm for w in widths], repeatRows=1 if header else 0)
    sty = [("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#bbb")),
           ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
           ("TOPPADDING", (0, 0), (-1, -1), 3), ("BOTTOMPADDING", (0, 0), (-1, -1), 3)]
    if header:
        sty += [("BACKGROUND", (0, 0), (-1, 0), ACC)]
        sty += [("BACKGROUND", (0, i), (-1, i), colors.HexColor("#f4f8f5")) for i in range(2, len(rows), 2)]
    t.setStyle(TableStyle(sty))
    return t

def img(name, width_mm):
    from reportlab.lib.utils import ImageReader
    path = os.path.join(IMG, name)
    w, h = ImageReader(path).getSize()
    return Image(path, width=width_mm * mm, height=width_mm * mm * h / w)

def footer(c, doc):
    c.saveState()
    c.setFont("DV", 7.5); c.setFillColor(colors.HexColor("#666"))
    c.drawString(18 * mm, 10 * mm, "SERVO-I2C NANO - Dossier technique - Rév. 1.3 - 29/10/2026")
    c.drawRightString(192 * mm, 10 * mm, f"Page {doc.page}")
    c.setStrokeColor(ACC); c.setLineWidth(0.6); c.line(18 * mm, 13 * mm, 192 * mm, 13 * mm)
    c.restoreState()

def build():
    from PIL import Image
    Image.open(os.path.join(IMG, "schematic.png")).rotate(90, expand=True).save(os.path.join(IMG, "schematic_rot.png"))
    out = os.path.join(DOCS, "SERVO_I2C_NANO_Documentation.pdf")
    doc = BaseDocTemplate(out, pagesize=A4, leftMargin=18 * mm, rightMargin=18 * mm, topMargin=16 * mm,
                          bottomMargin=18 * mm, title="SERVO-I2C NANO - Dossier technique",
                          author="Pascal Denicou", subject="Carte de pilotage de 2 servos par I2C")
    doc.addPageTemplates([PageTemplate("p", [Frame(doc.leftMargin, doc.bottomMargin, doc.width, doc.height)], onPage=footer)])
    s = []

    # ---------------- Couverture ----------------
    s += [Spacer(1, 18 * mm), P("SERVO-I2C NANO", "title"),
          P("Carte esclave I2C de pilotage de 2 servomoteurs<br/>à base d'Arduino Nano", "sub"),
          Spacer(1, 6 * mm),
          P("Dossier technique - Révision 1.3 - 29 octobre 2026<br/>Carte rév. 1.1 - maître Arduino UNO R4 WiFi", "p"), Spacer(1, 6 * mm),
          img("pcb_top.png", 95), Spacer(1, 6 * mm),
          table([["Contenu du dossier", ""],
                 ["1. Présentation et caractéristiques", "6. Nomenclature (BOM)"],
                 ["2. Architecture et choix techniques", "7. Faire fabriquer la carte"],
                 ["3. Schéma électrique", "8. Assemblage et mise en service"],
                 ["4. Connecteurs et cavaliers", "9. Firmware et intégration du protocole"],
                 ["5. Circuit imprimé", "10. Maître Arduino UNO R4 WiFi"],
                 ["", "11. Points de vigilance"]], [87, 87]),
          PageBreak()]

    # ---------------- 1 ----------------
    s += [P("1. Présentation et caractéristiques", "h1"),
          P("La carte SERVO-I2C NANO reçoit des ordres sur un bus I2C (en tant qu'esclave) et pilote deux "
            "servomoteurs de modélisme. Elle est construite autour d'un <b>Arduino Nano</b> enfiché sur la carte, "
            "et les servos sont alimentés par une <b>source dédiée</b> (jack DC), totalement indépendante du "
            "régulateur de l'Arduino."),
          table([["Paramètre", "Valeur"],
                 ["Dimensions carte", f"{B.W:.0f} x {B.H:.0f} mm, 2 couches, épaisseur 1,6 mm (USB du Nano dépassant de 2,5 mm)"],
                 ["Microcontrôleur", "Arduino Nano v3 (ATmega328P, 16 MHz, 5 V) sur barrettes femelles - remplaçable"],
                 ["Bus de commande", "I2C esclave 5 V, adresse configurable (0x40 par défaut), 100 / 400 kHz"],
                 ["Connecteur I2C", "JST XH 4 points (verrouillable, détrompé) : GND / +5V / SDA / SCL"],
                 ["Sorties servos", "2 x barrette 1x3 au pas de 2,54 mm, ordre standard S / + / -"],
                 ["Alimentation servos", "Jack DC 5,5 x 2,1 mm, centre +, 4,8 à 8,4 V, 2,5 A max (limite du jack)"],
                 ["Alimentation logique", "+5V du bus I2C (par défaut) ou rail servo via diode (option JP1)"],
                 ["Protections", "Inversion de polarité (P-MOSFET), résistances série 220 Ω sur les signaux, réservoir 470 µF"],
                 ["Signalisation", "LED verte « PWR » : alimentation servos présente ; LED jaune « ACT » (A3) : flash à chaque commande transmise à un servo"],
                 ["Fixation", "2 trous M2,5 (Ø 2,7 mm) en diagonale"],
                 ["Dégagement mécanique", "≈ 4,2 mm entre le bord du Nano et les fiches servo, ≈ 5 mm jusqu'au connecteur I2C"]], [45, 129]),
          Spacer(1, 4 * mm),
          P("2. Architecture et choix techniques", "h1"),
          P("<b>Arduino Nano.</b> C'est le plus petit Arduino officiel disposant d'un port USB intégré et de toutes "
            "ses broches sur deux rangées au pas standard : il se monte sur barrettes femelles, ce qui le rend "
            "remplaçable et reprogrammable en place. Sa logique 5 V évite tout adaptateur de niveau sur un bus I2C 5 V."),
          P("<b>Alimentation séparée des servos.</b> Le courant des servos (jusqu'à 1 à 2 A en pointe par servo) ne "
            "transite jamais par l'Arduino. Le rail VSERVO part du jack J1, traverse le MOSFET de protection Q1 puis "
            "dessert directement les connecteurs J2/J3 par des pistes de 1,5 mm. Les masses sont communes "
            "(indispensable pour la référence du signal PWM) et reliées par deux plans de masse cousus par vias."),
          P("<b>Protection contre l'inversion de polarité.</b> Q1 (AO3401A, P-MOSFET 30 V / 4 A, ~50 mΩ) est monté en "
            "« diode idéale » : en polarité correcte il conduit avec une chute négligeable ; en polarité inverse il "
            "bloque. C1 (470 µF) absorbe les appels de courant des servos au démarrage."),
          P("<b>Alimentation du Nano.</b> Par défaut le Nano est alimenté par la broche +5V du connecteur I2C, "
            "fournie par le maître : la logique est ainsi isolée des parasites des servos. Le pont JP1 permet, en "
            "option, d'alimenter le Nano depuis J1 à travers la diode Schottky D1 (uniquement si J1 ≤ 5,5 V)."),
          P("<b>Broches utilisées.</b> A0 et A1 (utilisables en sorties numériques, compatibles avec la "
            "bibliothèque Servo) pilotent les servos car elles sont du même côté que les connecteurs, ce qui donne "
            "un routage très court. A4/A5 sont les broches I2C matérielles. A3 commande la LED d'activité D3 (jaune, « ACT »)."),
          PageBreak()]

    # ---------------- 3 ----------------
    s += [P("3. Schéma électrique", "h1"),
          P("Le schéma est également fourni en pleine résolution (schematic.pdf / .svg / .png) dans le dossier "
            "<i>docs/carte-servo-i2c/img/</i> du dépôt."),
          img("schematic_rot.png", 150), PageBreak()]

    # ---------------- 4 ----------------
    s += [P("4. Connecteurs et cavaliers", "h1"),
          P("J1 - Alimentation des servos", "h2"),
          table([["Contact", "Signal", "Remarque"],
                 ["Centre (pin 1)", "+ VSERVO (4,8 à 8,4 V)", "Tension selon les servos utilisés (6 V typique, 7,4 V pour servos HV)"],
                 ["Manchon (pin 2)", "GND", "Masse commune avec le Nano et le bus I2C"],
                 ["Contact (pin 3)", "GND", "Contact de commutation relié à la masse"]], [32, 50, 92]),
          P("J2 / J3 - Servos (barrette mâle 1x3, 2,54 mm)", "h2"),
          table([["Broche", "Signal", "Servo 1 (J2)", "Servo 2 (J3)"],
                 ["1 (carrée, côté Nano)", "S - signal PWM", "Nano A0 via R5 220 Ω", "Nano A1 via R6 220 Ω"],
                 ["2 (centre)", "+ VSERVO", "Rail servo", "Rail servo"],
                 ["3 (extérieur)", "- GND", "Masse", "Masse"]], [40, 40, 47, 47]),
          P("Correspond au câblage standard Futaba/JR/Hitec : fil orange/jaune/blanc = S, rouge = +, marron/noir = -.", "small"),
          P("J4 - Bus I2C (JST XH 4 points, pas de 2,5 mm)", "h2"),
          table([["Broche", "Signal", "Nano", "Remarque"],
                 ["1", "GND", "GND", "Masse de référence"],
                 ["2", "+5V", "+5V (pin 27)", "Alimente le Nano (≈ 20 à 40 mA)"],
                 ["3", "SDA", "A4", "Logique 5 V"],
                 ["4", "SCL", "A5", "Logique 5 V"]], [18, 25, 35, 96]),
          P("Cavaliers à souder (ouverts à la livraison)", "h2"),
          table([["Repère", "Fermé (pont de soudure)", "Quand le fermer"],
                 ["JP2", "Pull-ups 4,7 kΩ de SDA et SCL vers +5V", "Si aucun pull-up n'existe déjà sur le bus. Un seul jeu de pull-ups par bus "
                  "(ou résistance équivalente ≥ 1,5 kΩ)."],
                 ["JP1", "Nano alimenté par J1 via D1", "Si le maître ne fournit pas de +5V sur J4 <b>et</b> que J1 ≤ 5,5 V. "
                  "Ne jamais fermer avec un +5V présent sur J4 ni avec J1 > 5,5 V."]], [18, 55, 101]),
          Paragraph("<b>Attention - bus 3,3 V.</b> Le Nano fonctionne en 5 V. Avec un maître 3,3 V (Raspberry Pi, "
                    "ESP32...), laissez JP2 ouvert, ne reliez pas les pull-ups au 5 V et intercalez un adaptateur de "
                    "niveau I2C (type PCA9306 ou BSS138) entre le maître et J4 ; le +5V de J4 doit alors être fourni "
                    "par une alimentation 5 V.", st["warn"]),
          P("Scénarios d'alimentation", "h2"),
          table([["Cas", "J4 pin 2", "J1", "JP1", "Résultat"],
                 ["Standard", "+5V du maître", "Alim servos 4,8-8,4 V", "Ouvert", "Logique et puissance séparées (recommandé)"],
                 ["Autonome", "non connecté", "5,0-5,5 V", "Fermé", "Tout alimenté par J1 (Nano ≈ J1 - 0,3 V)"],
                 ["Programmation", "indifférent", "indifférent", "Ouvert", "USB du Nano ; servos actifs seulement si J1 présent"]],
                [24, 30, 36, 16, 68]),
          PageBreak()]

    # ---------------- 5 ----------------
    s += [P("5. Circuit imprimé", "h1"),
          Table([[img("pcb_top.png", 82), img("pcb_bottom.png", 82)]], colWidths=[87 * mm, 87 * mm]),
          P("Rendus générés depuis les fichiers de fabrication. Le dessous porte un aide-mémoire sérigraphié des "
            "brochages et cavaliers.", "small"),
          P("Spécifications de fabrication", "h2"),
          table([["Paramètre", "Valeur"],
                 ["Couches", "2 (cuivre dessus + dessous, plans de masse sur les deux faces)"],
                 ["Matériau / épaisseur", "FR-4 TG130 ou mieux, 1,6 mm"],
                 ["Cuivre", "1 oz (35 µm) faces externes"],
                 ["Dimensions", f"{B.W:.1f} x {B.H:.1f} mm, contour rectangulaire"],
                 ["Piste / isolement min.", "0,30 mm / 0,30 mm (plans), 0,20 mm (DRC pistes)"],
                 ["Perçage min. / via", "0,40 mm (vias Ø 0,8 mm, couverts de vernis)"],
                 ["Trous oblongs", "3 (jack J1), 1,0 x 3,0 mm, métallisés - définis en G85 dans le fichier PTH"],
                 ["Vernis épargne / sérigraphie", "Vert / blanc (au choix)"],
                 ["Finition", "HASL sans plomb (ENIG possible)"],
                 ["Trous non métallisés", "2 x Ø 2,7 mm (fixation M2,5) - fichier NPTH séparé"]], [50, 124]),
          PageBreak(),
          P("Fichiers fournis (dossier materiel/carte-servo-i2c/fabrication/gerber)", "h2"),
          table([["Fichier", "Contenu"],
                 ["SERVO_I2C_NANO-F_Cu.gtl / B_Cu.gbl", "Cuivre dessus / dessous"],
                 ["SERVO_I2C_NANO-F_Mask.gts / B_Mask.gbs", "Vernis épargne dessus / dessous"],
                 ["SERVO_I2C_NANO-F_Silkscreen.gto / B_Silkscreen.gbo", "Sérigraphie dessus / dessous"],
                 ["SERVO_I2C_NANO-F_Paste.gtp", "Crème à braser (pochoir CMS)"],
                 ["SERVO_I2C_NANO-Edge_Cuts.gm1", "Contour de la carte"],
                 ["SERVO_I2C_NANO-PTH.drl / NPTH.drl", "Perçages métallisés (dont oblongs) / non métallisés (Excellon)"],
                 ["materiel/carte-servo-i2c/fabrication/assembly/ : BOM_JLCPCB.csv, CPL_JLCPCB.csv", "Nomenclature et placement CMS (format JLCPCB)"]], [85, 89]),
          P("Plan d'assemblage", "h2"), img("assembly_top.png", 105), PageBreak()]

    # ---------------- 6 BOM ----------------
    rows = [["Repères", "Qté", "Valeur", "Boîtier", "Référence", "LCSC"]]
    with open(os.path.join(ROOT, "fabrication", "assembly", "BOM_complete.csv"), encoding="utf-8") as f:
        r = csv.reader(f, delimiter=";"); next(r)
        for row in r:
            rows.append([row[0], row[1], row[2], row[3].replace("_", " "), row[5], row[6]])
    s += [P("6. Nomenclature (BOM)", "h1"),
          table(rows, [20, 9, 33, 46, 36, 30]),
          Spacer(1, 3 * mm),
          P("Les codes LCSC des CMS correspondent à des références « basic » courantes de JLCPCB. Ceux marqués « à "
            "vérifier » (connecteurs, électrolytique) doivent être confirmés dans le catalogue au moment de la "
            "commande ; toute référence équivalente de même empreinte convient. Le Nano et ses barrettes ne sont pas "
            "assemblés par le fabricant.", "small"),
          P("Coût indicatif (5 cartes, hors port)", "h2"),
          P("PCB nu : quelques euros les 5 ; assemblage CMS économique : environ 10 à 25 € selon les frais de "
            "préparation ; Arduino Nano : 5 € (compatible) à 25 € (officiel) ; connecteurs et condensateur : 2 à 3 € "
            "par carte. Ordres de grandeur à vérifier au moment de la commande."),
          PageBreak()]

    # ---------------- 7 ----------------
    s += [P("7. Faire fabriquer la carte", "h1"),
          P("Les Gerber sont au format RS-274X (attributs X2), les perçages au format Excellon métrique. Ils sont "
            "acceptés par les fabricants usuels (JLCPCB, PCBWay, Eurocircuits, Aisler...). Exemple avec JLCPCB :"),
          table([["Étape", "Action"],
                 ["1", "Envoyer le fichier <b>SERVO_I2C_NANO_gerber.zip</b> (dossier materiel/carte-servo-i2c/fabrication/) sur la page de devis. "
                       "Les dimensions 37,5 x 49 mm et 2 couches sont détectées automatiquement."],
                 ["2", "Paramètres : FR-4, 1,6 mm, 1 oz, HASL sans plomb, couleur au choix. Laisser « Remove order number » "
                       "selon votre préférence (la face arrière est chargée : choisir l'option payante ou accepter le numéro)."],
                 ["3", "Vérifier dans la visionneuse les 3 trous oblongs du jack (ils doivent apparaître en oblong)."],
                 ["4", "Option assemblage : cocher « PCB Assembly », face Top, puis charger <b>BOM_JLCPCB.csv</b> et "
                       "<b>CPL_JLCPCB.csv</b>. Contrôler dans l'aperçu l'orientation de Q1, D1 (barre = cathode, côté "
                       "JP1), D2 et D3 (cathodes vers le bas)."],
                 ["5", "Les composants traversants (J1, J2, J3, J4, C1, barrettes du Nano) se soudent à la main ; "
                       "ils peuvent aussi être ajoutés en « THT assembly » si le service le propose."]], [14, 160]),
          Paragraph("Pour un autre fabricant : envoyer le même ZIP et indiquer « 2 couches, 1,6 mm, 1 oz, slots "
                    "métallisés présents dans le fichier PTH (G85) ». Les fichiers sont générés en vue de dessus.",
                    st["warn"]),
          P("8. Assemblage et mise en service", "h1"),
          P("Ordre de soudure", "h2"),
          P("1) CMS (si non assemblés) : R1-R7, C2, D1, D2, D3, Q1. 2) Barrettes femelles 1x15 du Nano (les enficher sur "
            "le Nano pendant la soudure pour garantir l'alignement). 3) J2, J3, J4. 4) C1 en respectant la polarité "
            "(+ = pastille carrée, côté droit). 5) J1. 6) Ponts JP1/JP2 selon votre configuration."),
          P("Contrôles avant la première mise sous tension", "h2"),
          table([["#", "Test", "Attendu"],
                 ["1", "Résistance entre centre et manchon de J1 (Nano retiré)", "> 1 kΩ (charge de C1 visible)"],
                 ["2", "Résistance +5V / GND sur J4", "Pas de court-circuit"],
                 ["3", "J1 alimenté en 6 V via alim. de labo limitée à 200 mA", "LED D2 allumée, 6 V sur la broche 2 de J2/J3"],
                 ["4", "J1 alimenté en polarité inverse (test optionnel, courant limité)", "LED éteinte, ≈ 0 V sur VSERVO"],
                 ["5", "Nano en place, programmé, J4 relié au maître", "Le maître détecte l'adresse 0x40 (scan I2C)"],
                 ["6", "Envoi d'un ordre de position", "Le servo se positionne et la LED jaune « ACT » flashe (60 ms)"]],
                [8, 90, 76]),
          PageBreak()]

    # ---------------- 9 ----------------
    s += [P("9. Firmware et intégration de votre protocole", "h1"),
          P("Le dossier <i>firmware/carte-servo-i2c/servo_i2c_nano</i> est un sketch Arduino prêt à compiler (IDE Arduino ou "
            "arduino-cli, carte « Arduino Nano », processeur ATmega328P - ou « Old Bootloader » pour beaucoup de clones). "
            "Il ne dépend que des bibliothèques standard <b>Wire</b> et <b>Servo</b>."),
          table([["Fichier", "Rôle"],
                 ["servo_i2c_nano.ino", "Cœur : I2C esclave, tampons, gestion des servos, sécurité. Rien à modifier en principe."],
                 ["config.h", "Adresse I2C, plage d'impulsions, durée du flash LED, timeout de sécurité, traces série."],
                 ["protocole.h / .cpp", "<b>Votre protocole</b> : décodage des trames reçues et préparation des réponses. "
                                       "Fourni avec un protocole d'exemple à remplacer."],
                 ["test_maitre/test_maitre.ino", "Programme de test pour un Arduino maître utilisant le protocole d'exemple."],
                 ["maitre_uno_r4_wifi/", "Maître WiFi UNO R4 : page web + WebSocket vers I2C (voir section 10)."]], [48, 126]),
          P("Principe", "h2"),
          P("Les callbacks I2C s'exécutent sous interruption : ils se contentent de copier la trame reçue. "
            "Le décodage est fait dans <i>loop()</i> par <i>protocoleTraiterTrame()</i>, ce qui permet un traitement "
            "aussi riche que nécessaire sans bloquer le bus. Les réponses au maître sont préparées à l'avance via "
            "<i>reponseDefinir()</i> et envoyées lorsque le maître lit."),
          P("Ce que vous avez à écrire", "h2"),
          Paragraph("bool protocoleTraiterTrame(const uint8_t* trame, uint8_t len) {<br/>"
                    "&nbsp;&nbsp;// 1. valider la trame (longueur, commande, CRC éventuel...)<br/>"
                    "&nbsp;&nbsp;// 2. appeler servoEcrireAngle(idx, deg) ou servoEcrireMicros(idx, us)<br/>"
                    "&nbsp;&nbsp;// 3. préparer la réponse : reponseDefinir(buffer, n)<br/>"
                    "&nbsp;&nbsp;return true;   // trame valide<br/>}", st["code"]),
          table([["Service fourni", "Effet"],
                 ["servoEcrireAngle(idx, deg)", "Position 0-180° (idx 0 = servo 1 / J2, 1 = servo 2 / J3)"],
                 ["servoEcrireMicros(idx, us)", "Impulsion en µs, bornée par SERVO_US_MIN/MAX (500-2500 par défaut)"],
                 ["servoRelacher(idx)", "Arrête les impulsions : le servo n'est plus asservi"],
                 ["servoLireMicros(idx)", "Dernière consigne (0 si relâché)"],
                 ["reponseDefinir(data, len)", "Données renvoyées à la prochaine lecture du maître (32 octets max)"],
                 ["trameAEtePerdue()", "Vrai si une trame est arrivée avant la fin du traitement de la précédente"]], [55, 119]),
          P("Contraintes à connaître", "h2"),
          P("• Trames limitées à <b>32 octets</b> par transaction (tampon Wire de l'AVR). "
            "• La bibliothèque Servo utilise le Timer1 : les PWM matérielles des broches D9/D10 ne sont plus "
            "disponibles (sans impact ici). • Les servos ne sont pas activés au démarrage : aucune impulsion n'est "
            "émise avant le premier ordre, ce qui évite les à-coups à la mise sous tension. "
            "• <i>TIMEOUT_SECURITE_MS</i> permet de relâcher les servos si le maître se tait (perte de liaison). "
            "• <b>LED d'activité</b> : chaque appel à servoEcrireAngle() ou servoEcrireMicros() fait flasher la LED "
            "« ACT » (et la LED L du Nano) pendant LED_ACT_MS, sans bloquer le programme. Une trame invalide ou "
            "sans ordre servo ne l'allume pas."),
          P("Protocole d'exemple fourni", "h2"),
          table([["Trame écrite par le maître", "Action"],
                 ["01 ss aa", "Servo ss (0/1) à l'angle aa (0-180)"],
                 ["02 ss LL HH", "Servo ss à l'impulsion HHLL µs"],
                 ["03 ss", "Relâcher le servo ss"],
                 ["10, puis lecture 6 octets", "Réponse A5, version, consigne servo 1 (µs, LE), consigne servo 2 (µs, LE)"]], [55, 119]),
          PageBreak(),
          P("10. Maître Arduino UNO R4 WiFi", "h1"),
          P("L'Arduino UNO R4 WiFi remplace l'ensemble <b>Arduino UNO + shield PHPOC</b> côté maître. Le module "
            "ESP32-S3 intégré apporte le WiFi : plus besoin de shield réseau. Le microcontrôleur principal (RA4M1) "
            "fonctionne en <b>logique 5 V</b>, comme le Nano de la carte : la liaison I2C se fait en direct, sans "
            "adaptateur de niveau. <b>La carte SERVO-I2C NANO est inchangée.</b>"),
          img("cablage_uno_r4_wifi.png", 174),
          P("Câblage", "h2"),
          table([["UNO R4 WiFi", "Fil", "Carte J4", "Remarque"],
                 ["GND (POWER)", "noir", "1 GND", "Masse commune obligatoire"],
                 ["5V (POWER)", "rouge", "2 +5V", "Alimente le Nano de la carte (≈ 30 mA)"],
                 ["SDA (en-tête près d'AREF, ou A4)", "bleu", "3 SDA", "Bus Wire, 5 V"],
                 ["SCL (en-tête près d'AREF, ou A5)", "jaune", "4 SCL", "Bus Wire, 5 V"]], [52, 16, 22, 84]),
          Paragraph("<b>Ne pas utiliser le connecteur Qwiic</b> du R4 : c'est un second bus (Wire1) en 3,3 V. "
                    "Fermer <b>JP2</b> sur la carte pour disposer des pull-ups 4,7 kΩ, sauf si d'autres pull-ups sont "
                    "déjà présents sur le bus. JP1 reste ouvert : le Nano est alimenté par le 5V du R4.", st["warn"]),
          P("Alimentation : le R4 est alimenté par USB-C ou par son jack (6 à 24 V). Les servos restent alimentés "
            "par J1 de la carte. Avec une seule alimentation de 7 à 8,4 V, on peut la brancher à la fois sur le jack "
            "du R4 et sur J1."),
          P("Firmware maître (firmware/carte-servo-i2c/maitre_uno_r4_wifi)", "h2"),
          table([["Fichier", "Rôle"],
                 ["maitre_uno_r4_wifi.ino", "Connexion WiFi, page de commande sur le port 80, serveur WebSocket sur le port 81, "
                                            "traduction des commandes en trames I2C, reconnexion automatique, matrice de LED."],
                 ["arduino_secrets.h", "Nom et mot de passe du réseau WiFi (2,4 GHz)."],
                 ["config.h", "IP fixe optionnelle, ports, sous-protocole WebSocket, adresse I2C de la carte (0x40), fréquence I2C."],
                 ["carte_servo.cpp / .h", "<b>Construction des trames I2C</b> : à adapter à votre protocole (même rôle que "
                                          "protocole.cpp côté carte)."],
                 ["page_web.h", "Page de commande embarquée : deux curseurs, boutons Centre et Relâcher, lecture d'état."]], [46, 128]),
          P("Installation : dans l'IDE Arduino, installer le paquet de cartes <b>Arduino UNO R4 Boards</b> (fournit "
            "WiFiS3, Wire et la matrice de LED) et la bibliothèque <b>WebSockets</b> de Markus Sattler (Links2004), "
            "version 2.5.1 ou plus récente, qui prend en charge l'UNO R4 WiFi. Renseigner arduino_secrets.h, "
            "téléverser, puis lire l'adresse IP dans le moniteur série (115 200 bauds)."),
          P("Commandes WebSocket", "h2"),
          table([["Message texte", "Effet", "Réponse"],
                 ["S1=90 / S2=45", "Servo 1 ou 2 à l'angle donné (0 à 180°)", "OK S1=90 ou ERR ..."],
                 ["U1=1500 / U2=1200", "Impulsion en µs (500 à 2500)", "OK U1=1500 ou ERR ..."],
                 ["R1 / R2", "Relâche le servo", "OK R1"],
                 ["?", "Lit l'état de la carte", "ETAT fw=1.0 us1=1500 us2=0"]], [38, 76, 60]),
          P("Chaque commande acceptée fait flasher la LED « ACT » de la carte et un cadre sur la matrice du R4. "
            "En cas d'erreur I2C, la réponse indique la cause (carte absente, donnée refusée, délai dépassé)."),
          P("Migration depuis le shield PHPOC", "h2"),
          table([["UNO + PHPOC", "UNO R4 WiFi"],
                 ["#include &lt;Phpoc.h&gt; / Phpoc.begin()", "#include &lt;WiFiS3.h&gt; / WiFi.begin(ssid, mot de passe)"],
                 ["PhpocServer server(80) + server.beginWebSocket(\"remote_push\")", "WebSocketsServer sur le port 81 (bibliothèque WebSockets)"],
                 ["Page web stockée dans le shield (PHPoC Debugger)", "Page embarquée dans le sketch (page_web.h), servie sur le port 80"],
                 ["ws://ip/remote_push, sous-protocole text.phpoc", "ws://ip:81/ ; le sous-protocole text.phpoc est accepté"],
                 ["Configuration réseau dans le shield", "arduino_secrets.h et config.h"]], [87, 87]),
          P("Pour réutiliser une page web écrite pour le PHPOC, il suffit en général de remplacer l'adresse "
            "<i>ws://ip/remote_push</i> par <i>ws://ip:81/</i> ; le serveur répond avec le sous-protocole "
            "« text.phpoc » quand la page le demande. Les ordres reçus sont traités par traiterCommande() : "
            "adapter cette fonction si votre page envoie un autre format de message."),
          PageBreak(),
          P("11. Points de vigilance", "h1"),
          P("• <b>Courant</b> : le jack PJ-002A est donné pour 2,5 A. Pour deux servos puissants (calage > 1,5 A "
            "chacun), utiliser une alimentation adaptée et envisager un bornier à vis en révision 2. "
            "• <b>Tension</b> : C1 est en 10 V ; ne pas dépasser 8,4 V sur J1. "
            "• <b>Longueur du bus I2C</b> : prévu pour quelques dizaines de centimètres ; au-delà, abaisser la "
            "fréquence à 100 kHz voire utiliser un prolongateur de bus (P82B715...). "
            "• <b>Hauteur</b> : le Nano sur barrettes femelles de 8,5 mm laisse la place aux CMS placés dessous "
            "(R3, R4, JP2) ; ne pas souder le Nano à plat. "
            "• <b>Empreinte du jack</b> : conforme au plan recommandé du PJ-002A (oblongs 1 x 3 mm) ; si vous "
            "choisissez un autre modèle, vérifiez sa compatibilité avec ce plan avant commande. "
            "• <b>Validation</b> : le DRC (isolements, connectivité) a été vérifié par script et les Gerber relus avec "
            "un lecteur indépendant ; il est recommandé de commander d'abord une petite série de prototypes."),
          P("Emplacement dans le dépôt CassardRepository", "h2"),
          Paragraph("docs/carte-servo-i2c/<br/>&nbsp;&nbsp;README.txt, ce PDF<br/>&nbsp;&nbsp;img/ (schéma PDF/SVG/PNG, rendus, câblage)<br/>"
                    "materiel/carte-servo-i2c/<br/>&nbsp;&nbsp;fabrication/SERVO_I2C_NANO_gerber.zip (à envoyer au fabricant)<br/>"
                    "&nbsp;&nbsp;fabrication/gerber/ (Gerber + perçages, détail)<br/>"
                    "&nbsp;&nbsp;fabrication/assembly/ (BOM, CPL)<br/>"
                    "&nbsp;&nbsp;sources/ (scripts Python ayant généré la carte - modifiables et régénérables)<br/>"
                    "firmware/carte-servo-i2c/ (carte, maître UNO R4 WiFi, test maître)", st["code"]),
          P("Historique", "h2"),
          table([["Rév.", "Date", "Modification"], ["1.0", "27/10/2026", "Version initiale (Arduino Nano)"],
                 ["1.1", "27/10/2026", "Connecteurs décalés de 3,5 mm (dégagement Nano), carte 37,5 x 49 mm ; ajout LED d'activité D3/R7 sur A3"],
                 ["1.2", "29/10/2026", "Maître Arduino UNO R4 WiFi (remplace UNO + PHPOC) : firmware WiFi/WebSocket, câblage, migration. Carte inchangée (rév. 1.1)"],
                 ["1.3", "29/10/2026", "Dossier intégré au dépôt CassardRepository : nouveaux emplacements (docs/, materiel/, firmware/). Carte inchangée (rév. 1.1)"]], [15, 25, 134]),
          ]
    doc.build(s)
    return out

if __name__ == "__main__":
    print(build())
