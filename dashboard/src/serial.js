// Reads the ESP32's serial output in Chrome (Web Serial API).
// The firmware prints one `JSON:{...}` line per beat; every other line
// (RR:, OUTLIER:, REJECTED:, HRV RESULTS ...) is for humans.

export const BAUD_RATE = 115200;

export function isSerialSupported() {
  return typeof navigator !== 'undefined' && 'serial' in navigator;
}

// Returns the parsed beat object, or null if the line is not a valid JSON line.
export function parseLine(line) {
  if (!line.startsWith('JSON:')) return null;

  try {
    return JSON.parse(line.slice(5));
  } catch {
    return null;
  }
}

// Serial data arrives in random-sized chunks, not whole lines.
// This keeps the unfinished tail and only emits complete lines.
export function createLineSplitter(onLine) {
  let buffer = '';

  return (chunk) => {
    buffer += chunk;
    const lines = buffer.split('\n');
    buffer = lines.pop();

    for (const line of lines) {
      const trimmed = line.trim();
      if (trimmed) onLine(trimmed);
    }
  };
}

// Asks the user to pick a port, then calls onLine for every line.
// Returns an async function that closes the connection.
export async function openSerial(onLine, onClose) {
  const port = await navigator.serial.requestPort();
  await port.open({ baudRate: BAUD_RATE });

  const decoder = new TextDecoderStream();
  const pipeDone = port.readable.pipeTo(decoder.writable).catch(() => {});
  const reader = decoder.readable.getReader();
  const push = createLineSplitter(onLine);

  const readLoop = (async () => {
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        push(value);
      }
    } catch {
      // Device unplugged or reader cancelled
    } finally {
      reader.releaseLock();
      onClose?.();
    }
  })();

  return async () => {
    await reader.cancel().catch(() => {});
    await readLoop;
    await pipeDone;
    await port.close().catch(() => {});
  };
}
