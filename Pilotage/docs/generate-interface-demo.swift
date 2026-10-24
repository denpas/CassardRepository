import AVFoundation
import CoreGraphics
import CoreText
import CoreVideo
import Foundation

let width = 1280
let height = 720
let fps: Int32 = 12
let framesPerScene = 22
let navy = CGColor(red: 16 / 255, green: 35 / 255, blue: 63 / 255, alpha: 1)
let ink = CGColor(red: 24 / 255, green: 40 / 255, blue: 59 / 255, alpha: 1)
let muted = CGColor(red: 113 / 255, green: 128 / 255, blue: 150 / 255, alpha: 1)
let blue = CGColor(red: 36 / 255, green: 116 / 255, blue: 166 / 255, alpha: 1)
let paleBlue = CGColor(red: 234 / 255, green: 243 / 255, blue: 248 / 255, alpha: 1)
let gold = CGColor(red: 230 / 255, green: 167 / 255, blue: 68 / 255, alpha: 1)
let white = CGColor(gray: 1, alpha: 1)
let canvas = CGColor(red: 243 / 255, green: 246 / 255, blue: 249 / 255, alpha: 1)
let border = CGColor(red: 225 / 255, green: 232 / 255, blue: 239 / 255, alpha: 1)
let green = CGColor(red: 93 / 255, green: 212 / 255, blue: 155 / 255, alpha: 1)

struct Scene {
    let eyebrow: String
    let title: String
    let kind: String
}

let scenes = [
    Scene(eyebrow: "ACCÈS", title: "Bienvenue à bord", kind: "login"),
    Scene(eyebrow: "CENTRE DE COMMANDE", title: "Tableau de bord", kind: "dashboard"),
    Scene(eyebrow: "NAVIGATION", title: "Propulsion & direction", kind: "propulsion"),
    Scene(eyebrow: "TOURELLE", title: "Tourelle 100 mm", kind: "turret"),
    Scene(eyebrow: "SUPERVISION", title: "Instruments", kind: "instruments"),
    Scene(eyebrow: "ÉQUIPEMENTS", title: "Sons & lumières", kind: "sounds"),
    Scene(eyebrow: "OUTILS TECHNIQUES", title: "Console de diagnostic", kind: "debug"),
]

func rect(_ context: CGContext, _ x: CGFloat, _ top: CGFloat, _ w: CGFloat, _ h: CGFloat, _ color: CGColor, radius: CGFloat = 0) {
    let box = CGRect(x: x, y: CGFloat(height) - top - h, width: w, height: h)
    context.setFillColor(color)
    context.addPath(CGPath(roundedRect: box, cornerWidth: radius, cornerHeight: radius, transform: nil))
    context.fillPath()
}

func strokeRect(_ context: CGContext, _ x: CGFloat, _ top: CGFloat, _ w: CGFloat, _ h: CGFloat, _ color: CGColor, radius: CGFloat = 0, lineWidth: CGFloat = 1) {
    let box = CGRect(x: x, y: CGFloat(height) - top - h, width: w, height: h)
    context.setStrokeColor(color)
    context.setLineWidth(lineWidth)
    context.addPath(CGPath(roundedRect: box, cornerWidth: radius, cornerHeight: radius, transform: nil))
    context.strokePath()
}

func text(_ context: CGContext, _ value: String, _ x: CGFloat, _ top: CGFloat, _ size: CGFloat, _ color: CGColor, bold: Bool = false) {
    let fontName = bold ? "HelveticaNeue-Bold" : "HelveticaNeue"
    let font = CTFontCreateWithName(fontName as CFString, size, nil)
    let attributes = [kCTFontAttributeName: font, kCTForegroundColorAttributeName: color] as CFDictionary
    let line = CTLineCreateWithAttributedString(CFAttributedStringCreate(nil, value as CFString, attributes)!)
    context.textPosition = CGPoint(x: x, y: CGFloat(height) - top - CTFontGetAscent(font))
    CTLineDraw(line, context)
}

func line(_ context: CGContext, _ x1: CGFloat, _ top1: CGFloat, _ x2: CGFloat, _ top2: CGFloat, _ color: CGColor, _ lineWidth: CGFloat = 1) {
    context.setStrokeColor(color)
    context.setLineWidth(lineWidth)
    context.move(to: CGPoint(x: x1, y: CGFloat(height) - top1))
    context.addLine(to: CGPoint(x: x2, y: CGFloat(height) - top2))
    context.strokePath()
}

func ellipse(_ context: CGContext, x: CGFloat, top: CGFloat, width: CGFloat, ellipseHeight: CGFloat, color: CGColor) {
    let box = CGRect(x: x, y: CGFloat(height) - top - ellipseHeight, width: width, height: ellipseHeight)
    context.setFillColor(color)
    context.fillEllipse(in: box)
}

func card(_ context: CGContext, x: CGFloat, top: CGFloat, width: CGFloat, height: CGFloat) {
    rect(context, x, top, width, height, white, radius: 14)
    strokeRect(context, x, top, width, height, border, radius: 14)
}

func label(_ context: CGContext, _ value: String, _ x: CGFloat, _ top: CGFloat) {
    text(context, value.uppercased(), x, top, 11, blue, bold: true)
}

func slider(_ context: CGContext, x: CGFloat, top: CGFloat, width: CGFloat, progress: CGFloat) {
    rect(context, x, top, width, 8, border, radius: 4)
    rect(context, x, top, max(8, width * min(1, max(0, progress))), 8, blue, radius: 4)
    rect(context, x + width * min(1, max(0, progress)) - 11, top - 8, 24, 24, white, radius: 12)
    strokeRect(context, x + width * min(1, max(0, progress)) - 11, top - 8, 24, 24, border, radius: 12, lineWidth: 2)
}

func header(_ context: CGContext, scene: Scene, page: Int) {
    rect(context, 0, 0, CGFloat(width), 76, navy)
    rect(context, 50, 20, 36, 36, gold, radius: 9)
    text(context, "C", 62, 27, 17, navy, bold: true)
    text(context, "CASSARD", 99, 25, 14, white, bold: true)
    text(context, "PILOTAGE", 99, 44, 9, CGColor(gray: 0.76, alpha: 1), bold: true)
    text(context, scene.title, 390, 28, 18, white, bold: true)
    rect(context, 1004, 21, 132, 32, CGColor(gray: 1, alpha: 0.1), radius: 16)
    rect(context, 1020, 34, 7, 7, green, radius: 4)
    text(context, "Simulation", 1035, 29, 12, white)
    rect(context, 1148, 20, 82, 34, gold, radius: 8)
    text(context, "Aperçu", 1167, 29, 12, navy, bold: true)
    label(context, scene.eyebrow, 72, 116)
    text(context, scene.title, 72, 140, 32, navy, bold: true)
    line(context, 72, 672, 1208, 672, border)
    text(context, "CASSARD  ·  SIMULATION VISUELLE — AUCUNE COMMANDE MATÉRIELLE", 72, 685, 10, muted, bold: true)
    text(context, String(format: "%02d / %02d", page + 1, scenes.count), 1155, 685, 10, muted, bold: true)
    rect(context, 72, 660, CGFloat(page + 1) / CGFloat(scenes.count) * 1136, 3, blue, radius: 2)
}

func drawCardLink(_ context: CGContext, x: CGFloat, top: CGFloat, number: String, category: String, title: String, detail: String, action: String) {
    card(context, x: x, top: top, width: 548, height: 152)
    label(context, number + " / " + category, x + 24, top + 22)
    text(context, title, x + 24, top + 48, 20, navy, bold: true)
    text(context, detail, x + 24, top + 79, 13, muted)
    text(context, action + "    →", x + 24, top + 117, 12, blue, bold: true)
}

func render(_ scene: Scene, page: Int, phase: CGFloat) -> CGImage {
    let context = CGContext(data: nil, width: width, height: height, bitsPerComponent: 8, bytesPerRow: width * 4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue)!
    context.setAllowsAntialiasing(true)
    context.setShouldAntialias(true)
    rect(context, 0, 0, CGFloat(width), CGFloat(height), canvas)
    header(context, scene: scene, page: page)

    switch scene.kind {
    case "login":
        card(context, x: 394, top: 231, width: 492, height: 382)
        rect(context, 434, 268, 36, 36, gold, radius: 9)
        text(context, "C", 446, 275, 17, navy, bold: true)
        text(context, "Espace de pilotage", 485, 278, 14, navy, bold: true)
        text(context, "Identifiant", 434, 341, 12, ink, bold: true)
        strokeRect(context, 434, 364, 412, 46, border, radius: 8)
        text(context, "Votre identifiant", 450, 379, 13, muted)
        text(context, "Mot de passe", 434, 429, 12, ink, bold: true)
        strokeRect(context, 434, 452, 412, 46, border, radius: 8)
        text(context, "••••••••••", 450, 466, 14, muted)
        rect(context, 434, 522, 412, 48, gold, radius: 8)
        text(context, "Accéder au tableau de bord", 520, 538, 14, navy, bold: true)
    case "dashboard":
        drawCardLink(context, x: 72, top: 214, number: "01", category: "NAVIGATION", title: "Propulsion & direction", detail: "Moteurs, marche arrière et gouvernail", action: "Ouvrir le module")
        drawCardLink(context, x: 660, top: 214, number: "02", category: "TOURELLE", title: "Tourelle 100 mm", detail: "Réglage des axes d'azimut et d'élévation", action: "Ouvrir le module")
        drawCardLink(context, x: 72, top: 386, number: "03", category: "SUPERVISION", title: "Instruments", detail: "Indicateurs de vitesse et de puissance", action: "Consulter les instruments")
        drawCardLink(context, x: 660, top: 386, number: "04", category: "ÉQUIPEMENTS", title: "Sons & lumières", detail: "Accès au module des équipements", action: "Ouvrir le module")
        card(context, x: 72, top: 558, width: 1136, height: 64)
        label(context, "OUTILS TECHNIQUES", 96, 578)
        text(context, "Console de diagnostic", 300, 576, 16, navy, bold: true)
        text(context, "Échange de trames de protocole", 552, 578, 13, muted)
        text(context, "Accéder  →", 1020, 578, 12, blue, bold: true)
    case "propulsion":
        card(context, x: 72, top: 204, width: 858, height: 407)
        label(context, "Commande combinée", 98, 226)
        text(context, "Maintenez le joystick dans la direction souhaitée", 98, 246, 15, navy, bold: true)
        text(context, "AVANT", 487, 280, 9, muted, bold: true)
        text(context, "ARRIÈRE", 483, 485, 9, muted, bold: true)
        let joystickX = 500 + phase * 58
        let joystickY = 397 - phase * 34
        ellipse(context, x: 264, top: 364, width: 472, ellipseHeight: 136, color: CGColor(red: 0.9, green: 0.93, blue: 0.96, alpha: 1))
        ellipse(context, x: 272, top: 348, width: 456, ellipseHeight: 130, color: CGColor(red: 0.98, green: 0.99, blue: 1, alpha: 1))
        for ring in 1...3 {
            let radius = CGFloat(ring) * 55
            context.setStrokeColor(CGColor(red: 0.78, green: 0.85, blue: 0.9, alpha: 1))
            context.setLineWidth(1)
            context.strokeEllipse(in: CGRect(x: 500 - radius, y: CGFloat(height) - 415 - radius * 0.34, width: radius * 2, height: radius * 0.68))
        }
        line(context, 500, 350, 500, 473, CGColor(red: 0.83, green: 0.88, blue: 0.92, alpha: 1), 1)
        line(context, 346, 411, 654, 411, CGColor(red: 0.83, green: 0.88, blue: 0.92, alpha: 1), 1)
        ellipse(context, x: joystickX - 40, top: joystickY - 31, width: 80, ellipseHeight: 28, color: CGColor(red: 0.72, green: 0.79, blue: 0.85, alpha: 1))
        line(context, 500, 419, joystickX, joystickY, CGColor(red: 0.52, green: 0.6, blue: 0.68, alpha: 1), 23)
        line(context, 500, 419, joystickX, joystickY, CGColor(red: 0.91, green: 0.94, blue: 0.97, alpha: 1), 15)
        let knobRect = CGRect(x: joystickX - 42, y: CGFloat(height) - joystickY - 42, width: 84, height: 84)
        let knobColors = [CGColor(red: 0.99, green: 1, blue: 1, alpha: 1), CGColor(red: 0.44, green: 0.66, blue: 0.79, alpha: 1), CGColor(red: 0.14, green: 0.45, blue: 0.65, alpha: 1)] as CFArray
        let knobGradient = CGGradient(colorsSpace: CGColorSpaceCreateDeviceRGB(), colors: knobColors, locations: [0, 0.4, 1])!
        context.drawRadialGradient(knobGradient, startCenter: CGPoint(x: knobRect.midX - 16, y: knobRect.midY + 17), startRadius: 0, endCenter: CGPoint(x: knobRect.midX, y: knobRect.midY), endRadius: 46, options: [])
        context.setStrokeColor(CGColor(red: 0.1, green: 0.35, blue: 0.5, alpha: 1))
        context.setLineWidth(2)
        context.strokeEllipse(in: knobRect)
        ellipse(context, x: joystickX - 20, top: joystickY - 23, width: 23, ellipseHeight: 11, color: CGColor(gray: 1, alpha: 0.7))
        text(context, "Glisser · relâcher pour arrêter et recentrer", 365, 510, 10, muted)

        text(context, "Puissance moteur", 98, 551, 13, navy, bold: true)
        text(context, "+62 % · AVANT", 760, 551, 11, blue, bold: true)
        let throttleStart: CGFloat = 140
        let throttleEnd: CGFloat = 862
        let throttleCenter = (throttleStart + throttleEnd) / 2
        let throttleKnob = throttleCenter + 0.31 * (throttleEnd - throttleStart) * phase
        line(context, throttleStart, 584, throttleEnd, 584, border, 8)
        line(context, throttleCenter, 584, throttleKnob, 584, blue, 8)
        ellipse(context, x: throttleKnob - 12, top: 572, width: 24, ellipseHeight: 24, color: blue)
        text(context, "−100 %", 110, 603, 9, muted)
        text(context, "NEUTRE", 477, 603, 9, muted, bold: true)
        text(context, "+100 %", 825, 603, 9, muted)

        card(context, x: 950, top: 204, width: 258, height: 407)
        label(context, "Gouvernail", 975, 228)
        text(context, "Direction", 975, 249, 17, navy, bold: true)
        text(context, "+24° · TRIBORD", 975, 281, 11, blue, bold: true)
        let steeringCenter: CGFloat = 412
        let steeringKnob = steeringCenter - 64 * phase
        line(context, 1079, 327, 1079, 520, border, 8)
        line(context, 1079, steeringCenter, 1079, steeringKnob, blue, 8)
        ellipse(context, x: 1067, top: steeringKnob - 12, width: 24, ellipseHeight: 24, color: blue)
        text(context, "TRIBORD", 1036, 310, 9, muted, bold: true)
        text(context, "+90°", 1098, 328, 9, muted)
        text(context, "0°", 1098, 407, 9, muted)
        text(context, "−90°", 1098, 510, 9, muted)
        text(context, "BÂBORD", 1034, 539, 9, muted, bold: true)
        text(context, "Axe horizontal du joystick", 975, 573, 10, muted)

        text(context, "Réactivité moteur", 98, 632, 10, ink, bold: true)
        text(context, "NIVEAU 3", 768, 632, 9, blue, bold: true)
        line(context, 220, 638, 854, 638, border, 5)
        line(context, 220, 638, 220 + 380 * phase, 638, blue, 5)
        ellipse(context, x: 220 + 380 * phase - 8, top: 630, width: 16, ellipseHeight: 16, color: blue)
    case "turret":
        card(context, x: 72, top: 220, width: 548, height: 390)
        text(context, "Élévation", 102, 250, 19, navy, bold: true)
        text(context, "Inclinaison · de 0° à 60°", 102, 284, 13, muted)
        text(context, "24°", 102, 328, 28, blue, bold: true)
        rect(context, 324, 331, 8, 220, border, radius: 4)
        rect(context, 324, 425 - 18 * phase, 8, 126 + 18 * phase, blue, radius: 4)
        rect(context, 315, 413 - 18 * phase, 26, 26, blue, radius: 13)
        card(context, x: 660, top: 220, width: 548, height: 390)
        text(context, "Azimut", 690, 250, 19, navy, bold: true)
        text(context, "Orientation · de 0° à 180°", 690, 284, 13, muted)
        text(context, "118°", 690, 328, 28, blue, bold: true)
        line(context, 720, 464, 1148, 464, border, 8)
        line(context, 720, 464, 720 + 428 * (0.5 + 0.12 * phase), 464, blue, 8)
        rect(context, 720 + 428 * (0.5 + 0.12 * phase) - 12, 452, 24, 24, blue, radius: 12)
        text(context, "0°", 716, 495, 11, muted)
        text(context, "180°", 1110, 495, 11, muted)
    case "instruments":
        let metrics = [("MOTEUR GAUCHE", "3 240", "tr/min"), ("ARBRE GAUCHE", "1 180", "tr/min"), ("PUISSANCE", "62", "%"), ("ARBRE DROIT", "1 165", "tr/min"), ("MOTEUR DROIT", "3 210", "tr/min")]
        for (index, metric) in metrics.enumerated() {
            let x = 72 + CGFloat(index) * 229
            card(context, x: x, top: 248, width: 211, height: 235)
            label(context, metric.0, x + 18, 272)
            text(context, metric.1, x + 18, 325, 27, navy, bold: true)
            text(context, metric.2, x + 18, 363, 12, muted)
            rect(context, x + 18, 420, 175, 8, border, radius: 4)
            rect(context, x + 18, 420, 175 * (0.35 + 0.11 * CGFloat(index)), 8, blue, radius: 4)
        }
        rect(context, 72, 516, 1136, 60, paleBlue, radius: 10)
        text(context, "Aperçu illustratif — les mesures réelles nécessitent une source de télémétrie.", 96, 538, 13, blue, bold: true)
    case "sounds":
        card(context, x: 72, top: 231, width: 1136, height: 234)
        label(context, "MODULE ÉQUIPEMENTS", 104, 264)
        text(context, "Sons & lumières", 104, 302, 22, navy, bold: true)
        text(context, "Les commandes de ce module ne sont pas encore disponibles.", 104, 350, 15, muted)
        rect(context, 104, 399, 253, 36, paleBlue, radius: 8)
        text(context, "ÉCRAN EN PRÉPARATION", 122, 410, 11, blue, bold: true)
    case "debug":
        card(context, x: 72, top: 220, width: 1136, height: 384)
        label(context, "OUTILS TECHNIQUES", 104, 250)
        text(context, "Échange de trames", 104, 278, 20, navy, bold: true)
        text(context, "Envoyez une commande brute au contrôleur.", 104, 313, 13, muted)
        rect(context, 104, 353, 1072, 74, CGColor(red: 0.98, green: 0.99, blue: 1, alpha: 1), radius: 8)
        strokeRect(context, 104, 353, 1072, 74, border, radius: 8)
        text(context, "2,12,60,255", 126, 378, 14, ink)
        rect(context, 104, 450, 174, 40, gold, radius: 8)
        text(context, "Envoyer la trame", 124, 463, 12, navy, bold: true)
        rect(context, 104, 512, 1072, 1, border)
        text(context, "ENVOYÉ", 104, 538, 10, muted, bold: true)
        text(context, "2,12,60,255", 190, 535, 13, ink)
        rect(context, 1080, 534, 8, 8, green, radius: 4)
        text(context, "Aperçu local", 1098, 529, 10, muted)
    default:
        break
    }

    return context.makeImage()!
}

let scriptURL = URL(fileURLWithPath: #filePath)
let defaultOutput = scriptURL.deletingLastPathComponent().appendingPathComponent("interface-pilotage-demo.mp4")
let outputURL = CommandLine.arguments.dropFirst().first.map { URL(fileURLWithPath: $0) } ?? defaultOutput
try? FileManager.default.removeItem(at: outputURL)

let writer = try AVAssetWriter(outputURL: outputURL, fileType: .mp4)
let input = AVAssetWriterInput(mediaType: .video, outputSettings: [
    AVVideoCodecKey: AVVideoCodecType.h264,
    AVVideoWidthKey: width,
    AVVideoHeightKey: height,
])
input.expectsMediaDataInRealTime = false
let adaptor = AVAssetWriterInputPixelBufferAdaptor(assetWriterInput: input, sourcePixelBufferAttributes: [
    kCVPixelBufferPixelFormatTypeKey as String: Int(kCVPixelFormatType_32BGRA),
    kCVPixelBufferWidthKey as String: width,
    kCVPixelBufferHeightKey as String: height,
    kCVPixelBufferCGImageCompatibilityKey as String: true,
    kCVPixelBufferCGBitmapContextCompatibilityKey as String: true,
])
guard writer.canAdd(input) else {
    fatalError("Le conteneur vidéo ne peut pas ajouter de piste vidéo.")
}
writer.add(input)
guard writer.startWriting() else {
    throw writer.error ?? NSError(domain: "CassardDemo", code: 1, userInfo: [NSLocalizedDescriptionKey: "Impossible de démarrer l'encodage vidéo."])
}
writer.startSession(atSourceTime: .zero)

var frameIndex: Int64 = 0
for (page, scene) in scenes.enumerated() {
    for frame in 0..<framesPerScene {
        while !input.isReadyForMoreMediaData {
            Thread.sleep(forTimeInterval: 0.005)
        }
        var pixelBuffer: CVPixelBuffer?
        guard let pool = adaptor.pixelBufferPool,
              CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, pool, &pixelBuffer) == kCVReturnSuccess,
              let buffer = pixelBuffer else {
            writer.cancelWriting()
            fatalError("Impossible de réserver un tampon d'image vidéo.")
        }
        CVPixelBufferLockBaseAddress(buffer, [])
        if let baseAddress = CVPixelBufferGetBaseAddress(buffer),
           let context = CGContext(
                data: baseAddress,
                width: width,
                height: height,
                bitsPerComponent: 8,
                bytesPerRow: CVPixelBufferGetBytesPerRow(buffer),
                space: CGColorSpaceCreateDeviceRGB(),
                bitmapInfo: CGBitmapInfo.byteOrder32Little.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue)).rawValue
           ) {
            let image = render(scene, page: page, phase: CGFloat(frame) / CGFloat(framesPerScene - 1))
            context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        }
        CVPixelBufferUnlockBaseAddress(buffer, [])
        let time = CMTime(value: frameIndex, timescale: fps)
        guard adaptor.append(buffer, withPresentationTime: time) else {
            writer.cancelWriting()
            throw writer.error ?? NSError(domain: "CassardDemo", code: 2, userInfo: [NSLocalizedDescriptionKey: "Échec d'ajout d'une image à la vidéo."])
        }
        frameIndex += 1
    }
}

input.markAsFinished()
let semaphore = DispatchSemaphore(value: 0)
writer.finishWriting { semaphore.signal() }
semaphore.wait()
guard writer.status == .completed else {
    throw writer.error ?? NSError(domain: "CassardDemo", code: 3, userInfo: [NSLocalizedDescriptionKey: "Échec de finalisation de la vidéo."])
}
print("Vidéo créée : \(outputURL.path)")
