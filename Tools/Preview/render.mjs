// Renders preview screenshots with headless Chromium.
// Usage: node render.mjs <outDir> [view[:night] ...]
import { createRequire } from 'node:module';
import http from 'node:http';
const require = createRequire(import.meta.url);
let chromium;
try { ({ chromium } = require('playwright')); } catch { ({ chromium } = require(require('node:child_process').execSync('npm root -g').toString().trim() + '/playwright')); }
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const outDir = process.argv[2] || path.join(here, 'shots');
const views = process.argv.slice(3).length ? process.argv.slice(3) : ['aerial', 'street', 'beach', 'neon:night', 'marina', 'rig'];
fs.mkdirSync(outDir, { recursive: true });

const types = { '.html': 'text/html', '.js': 'text/javascript', '.mjs': 'text/javascript' };
const server = http.createServer((req, res) => {
  const p = path.join(here, decodeURIComponent(req.url.split('?')[0]));
  if (!p.startsWith(here) || !fs.existsSync(p) || fs.statSync(p).isDirectory()) { res.writeHead(404); res.end(); return; }
  res.writeHead(200, { 'Content-Type': types[path.extname(p)] || 'application/octet-stream' });
  fs.createReadStream(p).pipe(res);
});
await new Promise(r => server.listen(0, r));
const port = server.address().port;

const browser = await chromium.launch({ args: ['--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'] });
const page = await browser.newPage({ viewport: { width: 1600, height: 900 } });
page.on('console', m => { if (m.type() === 'error') console.log('console error:', m.text()); });
page.on('pageerror', e => console.log('page error:', e.message));
for (const spec of views) {
  const [view, mode] = spec.split(':');
  const night = mode === 'night' ? 1 : 0;
  await page.goto(`http://127.0.0.1:${port}/index.html?view=${view}&night=${night}`);
  await page.waitForFunction('window.READY === true', null, { timeout: 240000 });
  const file = path.join(outDir, `${view}${night ? '_night' : ''}.png`);
  await page.screenshot({ path: file });
  console.log('saved', file);
}
await browser.close();
server.close();
