/**************** GLOBALS ****************/
let device = null;
let rxChar = null;
let txChar = null;

let listening = false;
let audioCtx = null;
let lastAlertTime = 0;
const ALERT_COOLDOWN = 3000;

/******** UUIDs ********/
const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_UUID      = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const TX_UUID      = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

/**************** BLE ****************/
async function connectBLE() {
  try {
    device = await navigator.bluetooth.requestDevice({
      acceptAllDevices: true,
      optionalServices: [SERVICE_UUID]
    });

    device.addEventListener("gattserverdisconnected", onDisconnected);

    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);

    rxChar = await service.getCharacteristic(RX_UUID);
    txChar = await service.getCharacteristic(TX_UUID);

    await txChar.startNotifications();
    txChar.addEventListener("characteristicvaluechanged", handleData);

    setStatus("Connected", true);
    console.log("✅ BLE Connected");
  } catch (e) {
    console.error("❌ BLE Error:", e);
  }
}

function disconnectBLE() {
  if (device && device.gatt.connected) {
    device.gatt.disconnect();
  }
  onDisconnected();
}

function onDisconnected() {
  setStatus("Not Connected", false);
  resetUI();
  console.log("🔌 BLE Disconnected");
}

function sendCommand(cmd) {
  if (!rxChar) return;
  rxChar.writeValue(new TextEncoder().encode(cmd));
}

/**************** UI RESET ****************/
function resetUI() {
  document.getElementById("distance").innerText = "-- cm";
  document.getElementById("battery").innerText = "-- %";
  document.getElementById("flame").innerText   = "--";
  document.getElementById("ir").innerText      = "--";
}

/**************** DATA ****************/
function handleData(e) {
  const raw = new TextDecoder().decode(e.target.value).trim();
  const p = Object.fromEntries(raw.split(",").map(v => v.split(":")));

  document.getElementById("distance").innerText =
    Number(p.DIST).toFixed(1) + " cm";

  document.getElementById("battery").innerText =
    Number(p.BAT) + " %";

  const flameVal = Number(p.FLAME);
  document.getElementById("flame").innerText =
    flameVal === 0 ? "🔥 DANGER (Fire)" : "✅ Safe";

  const irL = Number(p.IRL);
  const irR = Number(p.IRR);

  let irText = "Clear";
  if (irL === 0 && irR === 0) irText = "⬅➡ Objects Both";
  else if (irL === 0) irText = "⬅ Object Left";
  else if (irR === 0) irText = "➡ Object Right";

  document.getElementById("ir").innerText = irText;
}

/**************** SOUND ****************/
async function startListening() {
  if (listening) return;
  listening = true;

  const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
  audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  await audioCtx.resume();

  const analyser = audioCtx.createAnalyser();
  analyser.fftSize = 2048;

  const mic = audioCtx.createMediaStreamSource(stream);
  mic.connect(analyser);

  const buffer = new Uint8Array(analyser.fftSize);

  setInterval(() => {
    analyser.getByteTimeDomainData(buffer);

    let sum = 0, peak = 0;
    for (let i = 0; i < buffer.length; i++) {
      const v = buffer[i] - 128;
      sum += v * v;
      peak = Math.max(peak, Math.abs(v));
    }

    const rms = Math.sqrt(sum / buffer.length);
    const now = Date.now();

    if (rms > 22 && peak > 70 && now - lastAlertTime > ALERT_COOLDOWN) {
      sendCommand("ALERT");
      lastAlertTime = now;
    }
  }, 200);
}

/**************** STATUS ****************/
function setStatus(text, connected) {
  const el = document.getElementById("status");
  el.innerText = text;
  el.className = "status " + (connected ? "connected" : "disconnected");
}
