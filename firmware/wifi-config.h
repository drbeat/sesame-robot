// wifi-config.h — WiFi provisioning UI served at GET /wifi
#pragma once

const char wifi_config_html[] PROGMEM = R"=====(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Sesame · WiFi Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{--bg:#08080f;--surface:#111118;--card:#1a1a24;--ch:#20202e;--ac:#8b5cf6;--ag:rgba(139,92,246,.28);--teal:#2dd4bf;--tx:#e2e8f0;--mu:#64748b;--bd:#2a2a38;--err:#f87171;--ok:#4ade80}
body{background:var(--bg);color:var(--tx);font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;min-height:100vh;display:flex;align-items:flex-start;justify-content:center;padding:24px 16px}
.wrap{width:100%;max-width:440px}
.hd{text-align:center;margin-bottom:28px}
.logo{display:inline-flex;align-items:center;gap:10px;margin-bottom:6px}
.li{width:42px;height:42px;background:linear-gradient(135deg,var(--ac),var(--teal));border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:22px}
.lt{font-size:22px;font-weight:700;letter-spacing:-.5px}
.sub{color:var(--mu);font-size:14px}
.card{background:var(--card);border:1px solid var(--bd);border-radius:16px;padding:20px;margin-bottom:14px}
.ct{font-size:12px;font-weight:600;color:var(--mu);text-transform:uppercase;letter-spacing:.08em;margin-bottom:14px;display:flex;align-items:center;justify-content:space-between}
.nl{display:flex;flex-direction:column;gap:8px}
.ni{display:flex;align-items:center;gap:12px;padding:12px;background:var(--surface);border:1px solid var(--bd);border-radius:10px;cursor:pointer;transition:all .15s}
.ni:hover{background:var(--ch);border-color:var(--ac)}
.ni.sel{background:rgba(139,92,246,.1);border-color:var(--ac)}
.sb{display:flex;align-items:flex-end;gap:2px;height:16px;flex-shrink:0}
.sb div{width:4px;background:var(--bd);border-radius:2px;transition:background .2s}
.sb div:nth-child(1){height:4px}.sb div:nth-child(2){height:8px}.sb div:nth-child(3){height:12px}.sb div:nth-child(4){height:16px}
.sb div.on{background:var(--teal)}
.ni-info{flex:1;min-width:0}
.ni-name{font-size:14px;font-weight:500;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.ni-meta{font-size:12px;color:var(--mu);margin-top:2px}
.fl{font-size:12px;color:var(--mu);flex-shrink:0}
.fg{margin-bottom:14px}
label{font-size:13px;color:var(--mu);margin-bottom:6px;display:block}
.iw{position:relative}
input[type=text],input[type=password]{width:100%;background:var(--surface);border:1px solid var(--bd);border-radius:10px;color:var(--tx);font-size:14px;padding:11px 14px;outline:none;transition:border-color .15s}
input:focus{border-color:var(--ac)}
.hp{padding-right:44px}
.eye{position:absolute;right:12px;top:50%;transform:translateY(-50%);background:none;border:none;color:var(--mu);cursor:pointer;font-size:16px;padding:4px;line-height:1;transition:color .15s}
.eye:hover{color:var(--tx)}
.btn{width:100%;padding:13px;border-radius:10px;border:none;font-size:15px;font-weight:600;cursor:pointer;transition:all .2s;display:flex;align-items:center;justify-content:center;gap:8px}
.bp{background:linear-gradient(135deg,var(--ac),#7c3aed);color:#fff;box-shadow:0 4px 20px var(--ag)}
.bp:hover{transform:translateY(-1px);box-shadow:0 6px 28px var(--ag)}
.bp:active{transform:none}
.bp:disabled{opacity:.5;cursor:not-allowed;transform:none}
.bg{background:transparent;color:var(--mu);border:1px solid var(--bd);font-size:13px;padding:7px 14px;width:auto}
.bg:hover{color:var(--tx);border-color:var(--tx)}
.sbox{display:flex;align-items:center;gap:12px;padding:14px;border-radius:10px;font-size:14px}
.sc{background:rgba(139,92,246,.1);border:1px solid var(--ac)}
.sd{background:rgba(74,222,128,.1);border:1px solid var(--ok)}
.sf{background:rgba(248,113,113,.1);border:1px solid var(--err)}
.sp{width:18px;height:18px;border:2px solid rgba(139,92,246,.3);border-top-color:var(--ac);border-radius:50%;animation:spin .8s linear infinite;flex-shrink:0}
@keyframes spin{to{transform:rotate(360deg)}}
.sk{background:linear-gradient(90deg,var(--surface) 25%,var(--ch) 50%,var(--surface) 75%);background-size:200%;animation:sh 1.4s infinite;border-radius:8px}
@keyframes sh{to{background-position:-200% 0}}
.es{text-align:center;padding:24px;color:var(--mu);font-size:14px}
.ip{display:inline-block;background:rgba(45,212,191,.15);border:1px solid var(--teal);color:var(--teal);padding:2px 8px;border-radius:6px;font-size:12px;font-family:monospace;margin-top:4px}
.mt{font-size:13px;color:var(--ac);cursor:pointer;text-align:center;margin-top:10px}
.mt:hover{text-decoration:underline}
.hide{display:none!important}
</style></head><body><div class="wrap">
<div class="hd">
  <div class="logo"><div class="li">&#x1F916;</div><span class="lt">Sesame Robot</span></div>
  <div class="sub">Connect to your WiFi network</div>
</div>
<div class="card">
  <div class="ct"><span>Available Networks</span>
    <button class="btn bg" id="scanBtn" onclick="scan()">&#x27F3; Scan</button>
  </div>
  <div class="nl" id="nl">
    <div class="sk" style="height:52px"></div>
    <div class="sk" style="height:52px;opacity:.7"></div>
    <div class="sk" style="height:52px;opacity:.4"></div>
  </div>
  <div class="mt" onclick="manualEntry()">&#x270E; Enter network name manually</div>
</div>
<div class="card">
  <div class="ct">Credentials</div>
  <div class="fg">
    <label for="si">Network Name (SSID)</label>
    <input id="si" type="text" placeholder="Select above or type here" autocomplete="off" spellcheck="false">
  </div>
  <div class="fg">
    <label for="pi">Password</label>
    <div class="iw">
      <input id="pi" type="password" class="hp" placeholder="WiFi password" autocomplete="new-password">
      <button class="eye" onclick="togglePw()">&#x1F441;</button>
    </div>
  </div>
  <button class="btn bp" id="cb" onclick="connect()"><span id="cbt">Connect</span></button>
</div>
<div class="card hide" id="sc">
  <div class="ct">Status</div>
  <div class="sbox sc" id="sb">
    <div class="sp" id="sp"></div>
    <div><div id="stxt">Connecting&hellip;</div><div class="ip hide" id="ip"></div></div>
  </div>
</div>
</div>
<script>
let poll;
async function scan(){
  const btn=document.getElementById('scanBtn'),nl=document.getElementById('nl');
  btn.disabled=true;btn.textContent='Scanning\u2026';
  nl.innerHTML='<div class="sk" style="height:52px"></div><div class="sk" style="height:52px;opacity:.7"></div><div class="sk" style="height:52px;opacity:.4"></div>';
  try{
    const nets=await(await fetch('/wifi/scan')).json();
    if(!nets.length){nl.innerHTML='<div class="es">&#x1F4E1; No networks found</div>';return;}
    nets.sort((a,b)=>b.rssi-a.rssi);
    nl.innerHTML=nets.map(n=>`<div class="ni" onclick="pick(this,'${esc(n.ssid)}')">
      <div class="sb">${bars(n.rssi)}</div>
      <div class="ni-info"><div class="ni-name">${esc(n.ssid)}</div><div class="ni-meta">${n.rssi} dBm &middot; ${n.secure?'&#x1F512; Secured':'&#x1F513; Open'}</div></div>
    </div>`).join('');
  }catch{nl.innerHTML='<div class="es">&#x26A0;&#xFE0F; Scan failed &mdash; try again</div>';}
  finally{btn.disabled=false;btn.textContent='\u27F3 Scan';}
}
function bars(r){const l=r>-55?4:r>-65?3:r>-75?2:1;return[1,2,3,4].map(i=>`<div class="${i<=l?'on':''}"></div>`).join('');}
function pick(el,ssid){document.querySelectorAll('.ni').forEach(e=>e.classList.remove('sel'));el.classList.add('sel');document.getElementById('si').value=ssid;document.getElementById('pi').focus();}
function manualEntry(){const si=document.getElementById('si');si.value='';si.placeholder='Type your network name\u2026';si.focus();}
function togglePw(){const p=document.getElementById('pi');p.type=p.type==='password'?'text':'password';}
async function connect(){
  const ssid=document.getElementById('si').value.trim(),pass=document.getElementById('pi').value;
  if(!ssid){document.getElementById('si').focus();return;}
  const cb=document.getElementById('cb');cb.disabled=true;document.getElementById('cbt').textContent='Connecting\u2026';
  status('c','Saving credentials\u2026');
  try{
    await fetch('/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,password:pass})});
    status('c',`Connecting to "${ssid}"\u2026`);
    startPoll(ssid,cb);
  }catch{status('f','Could not reach robot. Try again.');cb.disabled=false;document.getElementById('cbt').textContent='Connect';}
}
function startPoll(ssid,cb){
  clearInterval(poll);let tries=0;
  poll=setInterval(async()=>{
    tries++;
    try{
      const s=await(await fetch('/wifi/status')).json();
      if(s.connected){clearInterval(poll);status('d',`Connected to "${ssid}"!`,s.ip);cb.disabled=false;document.getElementById('cbt').textContent='Reconnect';}
      else if(s.failed||tries>20){clearInterval(poll);status('f',s.failed?'Wrong password or network unreachable.':'Connection timed out.');cb.disabled=false;document.getElementById('cbt').textContent='Try Again';}
    }catch{}
  },1500);
}
function status(t,txt,ip){
  const sc=document.getElementById('sc'),sb=document.getElementById('sb'),sp=document.getElementById('sp'),stxt=document.getElementById('stxt'),ipdiv=document.getElementById('ip');
  sc.classList.remove('hide');sb.className='sbox '+(t==='c'?'sc':t==='d'?'sd':'sf');
  sp.style.display=t==='c'?'block':'none';stxt.textContent=txt;
  if(ip){ipdiv.textContent=ip;ipdiv.classList.remove('hide');}else ipdiv.classList.add('hide');
}
function esc(s){return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');}
scan();
</script></body></html>
)=====";
