#!/usr/bin/env python3
"""Simulateur du pilotage Cassard : carte maître UNO R4 WiFi + cartes Nano.

HTTP : l'application web (web/, servie telle quelle), le tableau de
bord /simulation.html et le flux d'événements /evenements (Server-Sent Events).
WebSocket : comportement de firmware/ModuleCentral/ModuleCentral.ino (session signée, reprise
par un autre poste, arrêt de sécurité sur perte de liaison).
Les cartes Nano reproduisent firmware/ModulePropulsion/PropulsionCONTControler.h
et firmware/ModuleTourelle/TourelleControler.h.

    python3 outils/simulateur/simulateur.py
    puis ouvrir http://localhost:8080/simulation.html
"""
import argparse
import base64
import hashlib
import hmac
import http.server
import json
import pathlib
import queue
import secrets
import socket
import socketserver
import struct
import sys
import threading
import time

ICI = pathlib.Path(__file__).resolve().parent
WEB = ICI.parent.parent / "web"
DASHBOARD = ICI / "simulation.html"

arguments = argparse.ArgumentParser(description="Simulateur du pilotage Cassard")
arguments.add_argument("--port", type=int, default=8080, help="port HTTP (défaut 8080 ; 80 sur la carte)")
arguments.add_argument("--port-websocket", type=int, default=8181,
                       help="port WebSocket (défaut 8181 ; 81 sur la carte, réservé à l'administrateur sur macOS)")
arguments.add_argument("--mot-de-passe", default="cassard demo 2026", help="mot de passe de pilotage simulé")
arguments.add_argument("--hote", default="127.0.0.1",
                       help="adresse d'écoute ; 0.0.0.0 pour tester depuis l'iPhone sur le même Wi-Fi")
options = arguments.parse_args()
HTTP_PORT, WS_PORT, PASSWORD = options.port, options.port_websocket, options.mot_de_passe
KEY = hashlib.pbkdf2_hmac("sha256", PASSWORD.encode(), b"CASSARD-D614-pilotage", 10000, 32)
HEARTBEAT_TIMEOUT = 3.0   # ping 1 s, 2 échecs : ~3 s (enableHeartbeat(1000, 1000, 2))

lock = threading.RLock()
subscribers = []


def emit(**event):
    event["ms"] = int(time.time() * 1000)
    data = json.dumps(event)
    for q in list(subscribers):
        q.put(data)


def serial(text):
    emit(t="serial", text=text)


# ------------------------------------------------------------------ Nano
class PropulsionNano:
    """Reproduit PropulsionCONTControler.h (module I2C 2)."""
    AMPLITUDE = 254 - 64

    def __init__(self):
        self.power = 0
        self.angle = 0
        self.mode = 0
        self.rudder = 90
        self.left = self.right = (0, 0)   # (DIR, PWM)
        self.left_power = self.right_power = 0
        self.error = False

    @staticmethod
    def c_div(a, b):
        q = abs(a) // abs(b)
        return q if (a >= 0) == (b > 0) else -q

    def mix(self, power, angle):
        if power == 0:
            return 0, 0
        # SimulationModuleProportionnel.xlsx : puissance +/- angle x (proportion x 2 / puissance)
        adjust = self.c_div(angle * self.mode * 2, power)
        clamp = lambda v: max(-100, min(100, v))
        return clamp(power + adjust), clamp(power - adjust)

    def apply(self, left, right):
        # Moteurs montés en miroir : avant = DIR LOW à gauche, DIR HIGH à droite.
        pwm = lambda p: abs(p) * self.AMPLITUDE // 100
        self.left = (0 if left > 0 else 1, pwm(left)) if left else (self.left[0], 0)
        self.right = (1 if right > 0 else 0, pwm(right)) if right else (self.right[0], 0)
        self.left_power, self.right_power = left, right

    def process(self, frame):
        self.error = False
        n, cmd = len(frame), frame[1] if len(frame) > 1 else None
        ok = n >= 3 and frame[-1] == 255 and frame[0] == 2
        if ok and cmd in (12, 13) and n == 4 and frame[2] <= 100:
            power = frame[2] if cmd == 12 else -frame[2]
            self.apply(*self.mix(power, self.angle))
            self.power = power
        elif ok and cmd == 14 and n == 3:
            self.apply(0, 0)
            self.power = 0
        elif ok and cmd in (15, 16) and n == 4 and frame[2] <= 90:
            self.rudder = 90 + frame[2] if cmd == 15 else 90 - frame[2]
            self.angle = frame[2] if cmd == 15 else -frame[2]
            self.apply(*self.mix(self.power, self.angle))
        elif ok and cmd == 17 and n == 3:
            self.rudder, self.angle = 90, 0
            self.apply(*self.mix(self.power, self.angle))
        elif ok and cmd == 18 and n == 4 and frame[2] <= 5:
            self.mode = frame[2]
        else:
            self.error = True
        self.publish()

    def publish(self):
        emit(t="nano", addr=2, state=dict(
            power=self.power, angle=self.angle, mode=self.mode, rudder=self.rudder,
            left=dict(dir=self.left[0], pwm=self.left[1], power=self.left_power),
            right=dict(dir=self.right[0], pwm=self.right[1], power=self.right_power),
            error=self.error))


class TourelleNano:
    """Reproduit TourelleControler::GoToPos (module I2C 1)."""

    def __init__(self):
        self.azimut, self.elevation, self.error = 90, 0, False

    def process(self, frame):
        self.error = not (len(frame) == 5 and frame[0] == 1 and frame[1] == 4 and frame[4] == 255
                          and frame[2] <= 180 and frame[3] <= 80)
        if not self.error:
            self.azimut, self.elevation = frame[2], frame[3]
        self.publish()

    def publish(self):
        emit(t="nano", addr=1, state=dict(azimut=self.azimut, elevation=self.elevation, error=self.error))


nanos = {2: PropulsionNano(), 1: TourelleNano()}
NAMES = {12: "marche avant", 13: "marche arrière", 14: "arrêt moteur", 15: "barre à tribord",
         16: "barre à bâbord", 17: "barre au centre", 18: "réactivité", 4: "position tourelle"}


def i2c_send(frame, reason=""):
    emit(t="i2c", addr=frame[0], bytes=frame, label=NAMES.get(frame[1], "?"), reason=reason)
    nanos[frame[0]].process(frame)


# ------------------------------------------------- carte maître (R4 WiFi)
master = {"client": None, "nonce": "", "last": 0, "ready": False}
# Appareils du Wi-Fi, comme le firmware : adresses avec une liaison WebSocket ouverte,
# ou ayant chargé une page depuis moins de 30 s.
WIFI_DEVICE_TIMEOUT = 30
wifi_devices = {}


def note_wifi_device(ip):
    with lock:
        known = ip in wifi_devices
        wifi_devices[ip] = time.time()
        if not known:
            publish_wifi()


def publish_wifi():
    now = time.time()
    devices = {ip for ip, seen in wifi_devices.items() if now - seen < WIFI_DEVICE_TIMEOUT}
    devices |= {handler.client_address[0] for handler in list(clients.values())}
    emit(t="wifi", devices=len(devices))
clients = {}
next_id = [0]
LIMITS = {(1, 4): (5, (180, 80)), (2, 12): (4, (100,)), (2, 13): (4, (100,)), (2, 14): (3, ()),
          (2, 15): (4, (90,)), (2, 16): (4, (90,)), (2, 17): (3, ()), (2, 18): (4, (5,))}


def publish_master():
    emit(t="master", clients=sorted(clients), session=master["client"], seq=master["last"], ready=master["ready"])


def received(valid):
    """Trame comptée par la matrice de LED : valide, ou refusée (« ! »)."""
    emit(t="rx", ok=valid)


def parse_command(payload):
    try:
        fields = [int(f) for f in payload.split(",")]
    except ValueError:
        return None
    rule = LIMITS.get(tuple(fields[:2]))
    if len(payload) > 19 or len(fields) < 3 or fields[-1] != 255 or not rule or len(fields) != rule[0] \
            or any(v > m for v, m in zip(fields[2:-1], rule[1])):
        return None
    return fields


def safe_stop(reason):
    i2c_send([2, 14, 255], reason)
    i2c_send([2, 17, 255], reason)


def end_session(reason):
    safe_stop(reason)
    master.update(client=None, ready=False)
    publish_master()


def handle_frame(client, text):
    text = text.strip()
    if not text:
        return
    emit(t="ws", dir="in", client=client, text=text)
    if len(text) > 64:
        received(False)
        return clients[client].send("AUTH,ERR")
    if text == "AUTH":
        if master["client"] is not None:
            safe_stop("nouvelle session")
            if master["client"] != client and master["client"] in clients:
                clients[master["client"]].send("AUTH,REPRISE")
        master.update(client=client, nonce=secrets.token_hex(16), last=0, ready=False)
        publish_master()
        return clients[client].send("NONCE," + master["nonce"])
    parts = text.split("|")
    ok = client == master["client"] and len(parts) == 3 and parts[1].isdigit() and len(parts[1]) <= 10
    if ok:
        payload, sequence, mac = parts
        expected = hmac.new(KEY, f"{master['nonce']}|{sequence}|{payload}".encode(), hashlib.sha256).hexdigest()[:16]
        ok = hmac.compare_digest(expected, mac) and int(sequence) > master["last"]
    if not ok:
        received(False)
        return clients[client].send("AUTH,ERR")
    master["last"] = int(sequence)
    if payload == "PING":
        master["ready"] = True
        publish_master()
        received(True)
        return clients[client].send("AUTH,OK")
    publish_master()
    fields = parse_command(payload)
    if fields:
        i2c_send(fields)
    received(bool(fields))   # signée mais invalide : erreur, comme le firmware


class WebSocketHandler(socketserver.StreamRequestHandler):
    def send(self, text):
        data = text.encode()
        emit(t="ws", dir="out", client=self.client_id, text=text)
        try:
            self.wfile.write(bytes([0x81, len(data)]) + data)
            self.wfile.flush()
        except OSError:
            pass

    def handle(self):
        headers = {}
        self.rfile.readline()
        while True:
            line = self.rfile.readline().decode().strip()
            if not line:
                break
            name, _, value = line.partition(":")
            headers[name.lower()] = value.strip()
        accept = base64.b64encode(hashlib.sha1((headers["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest()).decode()
        response = f"HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: {accept}\r\n"
        if "cassard" in headers.get("sec-websocket-protocol", ""):
            response += "Sec-WebSocket-Protocol: cassard\r\n"
        self.wfile.write((response + "\r\n").encode())
        self.cut = False
        with lock:
            next_id[0] += 1
            self.client_id = next_id[0]
            clients[self.client_id] = self
            publish_master()
            publish_wifi()
        buffer = ""
        try:
            while True:
                header = self.rfile.read(2)
                if len(header) < 2:
                    break
                opcode, length = header[0] & 15, header[1] & 127
                if length == 126:
                    length = struct.unpack(">H", self.rfile.read(2))[0]
                elif length == 127:
                    length = struct.unpack(">Q", self.rfile.read(8))[0]
                mask = self.rfile.read(4) if header[1] & 128 else b"\0\0\0\0"
                data = bytes(b ^ mask[i % 4] for i, b in enumerate(self.rfile.read(length)))
                if opcode == 8:
                    break
                if opcode == 1 and not self.cut:
                    buffer += data.decode("utf-8", "replace")
                    lines = buffer.split("\n")
                    buffer = lines.pop()
                    with lock:
                        for line in lines:
                            handle_frame(self.client_id, line)
        except OSError:
            pass
        with lock:
            clients.pop(self.client_id, None)
            if master["client"] == self.client_id:
                serial("Liaison du pilote perdue : arret de securite")
                end_session("liaison du pilote perdue")
            publish_master()
            publish_wifi()

    def cut_link(self):
        """Simule la perte du Wi-Fi : plus rien ne passe, le ping échoue."""
        self.cut = True
        emit(t="ws", dir="info", client=self.client_id, text=f"liaison coupée — détection par le ping dans ~{HEARTBEAT_TIMEOUT:.0f} s")

        def drop():
            try:
                self.request.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
        threading.Timer(HEARTBEAT_TIMEOUT, drop).start()


# ------------------------------------------------------------ HTTP + SSE
class HttpHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(WEB), **kwargs)

    def log_message(self, *args):
        pass

    def end_headers(self):
        # Pas de cache : une page modifiée dans web/ doit apparaître au rechargement,
        # y compris dans le cadre iPhone du tableau de bord.
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_GET(self):
        if not self.path.startswith(("/simulation.html", "/evenements")):
            note_wifi_device(self.client_address[0])
        if self.path.startswith("/simulation.html"):
            body = DASHBOARD.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if self.path == "/evenements":
            return self.stream()
        if self.path.split("?")[0] == "/_cassard-link.js":
            return self.serve_link()
        return super().do_GET()

    def serve_link(self):
        """Sert _cassard-link.js en remplaçant, en mémoire, le port WebSocket de la carte."""
        source = (WEB / "_cassard-link.js").read_text(encoding="utf-8")
        body = source.replace("var WEBSOCKET_PORT = 81;", f"var WEBSOCKET_PORT = {WS_PORT};").encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/javascript; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path == "/sim/couper":
            with lock:
                target = clients.get(master["client"])
            if target:
                target.cut_link()
        self.send_response(204)
        self.end_headers()

    def stream(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.end_headers()
        q = queue.Queue()
        subscribers.append(q)
        with lock:
            q.put(json.dumps(dict(t="serial", ms=int(time.time() * 1000),
                                  text="Point d'acces Wi-Fi \"cassard\" (WPA2) : interface sur http://192.168.4.1")))
            publish_master()
            publish_wifi()
            for nano in nanos.values():
                nano.publish()
        try:
            while True:
                try:
                    data = q.get(timeout=15)
                    self.wfile.write(f"data: {data}\n\n".encode())
                except queue.Empty:
                    self.wfile.write(b": veille\n\n")
                self.wfile.flush()
        except OSError:
            pass
        finally:
            subscribers.remove(q)


class Server(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True


if __name__ == "__main__":
    if "var WEBSOCKET_PORT = 81;" not in (WEB / "_cassard-link.js").read_text(encoding="utf-8"):
        sys.exit("_cassard-link.js : port WebSocket introuvable, mettre à jour serve_link()")
    threading.Thread(target=Server((options.hote, WS_PORT), WebSocketHandler).serve_forever, daemon=True).start()

    def expire_wifi_devices():
        while True:
            time.sleep(2)
            with lock:
                publish_wifi()
    threading.Thread(target=expire_wifi_devices, daemon=True).start()
    print(f"Simulation : http://localhost:{HTTP_PORT}/simulation.html  (mot de passe de démo {PASSWORD!r})", flush=True)
    try:
        Server((options.hote, HTTP_PORT), HttpHandler).serve_forever()
    except KeyboardInterrupt:
        pass
