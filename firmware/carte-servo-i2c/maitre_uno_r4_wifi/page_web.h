#pragma once
// Page de commande servie sur http://<ip>/ (stockée en flash)
static const char PAGE_WEB[] = R"HTML(<!doctype html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SERVO-I2C</title>
<style>
body{font-family:system-ui,sans-serif;margin:0;background:#f3f6f4;color:#17221b}
main{max-width:520px;margin:0 auto;padding:16px;display:grid;gap:14px}
h1{font-size:20px;margin:4px 0}
.c{background:#fff;border:1px solid #d6dfd8;border-radius:8px;padding:14px;display:grid;gap:8px}
.r{display:flex;justify-content:space-between;align-items:center;gap:8px}
input[type=range]{width:100%}
.v{font:600 22px ui-monospace,monospace;min-width:4ch;text-align:right}
button{font:600 14px system-ui;border:0;border-radius:6px;padding:8px 12px;background:#0b5d2a;color:#fff}
button.s{background:#e3ebe5;color:#17221b}
#st{font-size:13px}.ok{color:#0b5d2a}.ko{color:#b3261e}
#log{font:12px ui-monospace,monospace;color:#5a6b60;white-space:pre-wrap;max-height:120px;overflow:auto}
</style></head><body><main>
<h1>SERVO-I2C &middot; UNO R4 WiFi</h1>
<div id="st" class="ko">Connexion...</div>
<div class="c"><div class="r"><b>Servo 1</b><span class="v" id="v1">90</span></div>
<input type="range" id="s1" min="0" max="180" value="90">
<div class="r"><button class="s" onclick="env('R1')">Rel&acirc;cher</button><button onclick="pos(1,90)">Centre</button></div></div>
<div class="c"><div class="r"><b>Servo 2</b><span class="v" id="v2">90</span></div>
<input type="range" id="s2" min="0" max="180" value="90">
<div class="r"><button class="s" onclick="env('R2')">Rel&acirc;cher</button><button onclick="pos(2,90)">Centre</button></div></div>
<div class="r"><button class="s" onclick="env('?')">Lire l'&eacute;tat</button><span id="et"></span></div>
<div id="log"></div>
</main><script>
var ws,att={},log=document.getElementById('log');
function trace(t){log.textContent=(t+'\n'+log.textContent).slice(0,1500)}
function co(){ws=new WebSocket('ws://'+location.hostname+':81/','text.phpoc');
ws.onopen=function(){st('Connecté',1);env('?')};
ws.onclose=function(){st('Déconnecté, nouvelle tentative...',0);setTimeout(co,2000)};
ws.onmessage=function(e){trace(e.data);if(e.data.indexOf('ETAT')==0)document.getElementById('et').textContent=e.data}}
function st(t,ok){var s=document.getElementById('st');s.textContent=t;s.className=ok?'ok':'ko'}
function env(c){if(ws&&ws.readyState==1){ws.send(c);trace('> '+c)}}
function pos(n,a){document.getElementById('s'+n).value=a;document.getElementById('v'+n).textContent=a;env('S'+n+'='+a)}
[1,2].forEach(function(n){var s=document.getElementById('s'+n);
s.oninput=function(){document.getElementById('v'+n).textContent=s.value;
if(!att[n]){att[n]=setTimeout(function(){att[n]=0;env('S'+n+'='+s.value)},60)}}});
co();
</script></body></html>)HTML";
