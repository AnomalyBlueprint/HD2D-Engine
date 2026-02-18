#!/usr/bin/env node
/**
 * extract-collections.js
 * 
 * One-shot script: reads bestiary/index.html and extracts the remaining
 * hardcoded JS data objects into individual JSON files under source_data/bestiary/.
 * 
 * Run:  node tools/tool-server/extract-collections.js
 */

const fs = require('fs');
const path = require('path');
const vm = require('vm');

const ROOT = path.resolve(__dirname, '..');
const HTML_PATH = path.join(ROOT, 'tool-server', 'public', 'bestiary', 'index.html');
const OUT_DIR = path.join(ROOT, 'source_data', 'bestiary');

const html = fs.readFileSync(HTML_PATH, 'utf8');

// Extract the <script type="text/babel"> block
const scriptStart = html.indexOf('<script type="text/babel">');
const scriptEnd = html.indexOf('</script>', scriptStart);
let jsCode = html.substring(
  html.indexOf('>', scriptStart) + 1,
  scriptEnd
);

// We need to evaluate just the constant declarations. 
// Strip out JSX, React, and function definitions that would fail in plain Node.
// Strategy: extract lines from "// --- CONSTANTS" to "const REAL_WORLD_DATA = {" end,
// plus the modifier blocks that follow.

function extractBlock(source, startMarker, endMarker) {
  const startIdx = source.indexOf(startMarker);
  if (startIdx === -1) return null;
  
  // Find the balanced closing brace
  let braceDepth = 0;
  let inString = false;
  let stringChar = '';
  let i = source.indexOf('{', startIdx);
  if (i === -1) return null;
  
  const blockStart = i;
  for (; i < source.length; i++) {
    const ch = source[i];
    
    if (inString) {
      if (ch === '\\') { i++; continue; }
      if (ch === stringChar) inString = false;
      continue;
    }
    
    if (ch === '"' || ch === "'" || ch === '`') {
      inString = true;
      stringChar = ch;
      continue;
    }
    
    if (ch === '{' || ch === '[') braceDepth++;
    if (ch === '}' || ch === ']') {
      braceDepth--;
      if (braceDepth === 0) {
        return source.substring(blockStart, i + 1);
      }
    }
  }
  return null;
}

// Helper: evaluate a JS object literal in a sandboxed context
function evalObject(objStr) {
  // Wrap in parens so it's an expression
  const code = `(${objStr})`;
  return vm.runInNewContext(code, {});
}

function writeJSON(filename, data) {
  const filePath = path.join(OUT_DIR, filename);
  fs.writeFileSync(filePath, JSON.stringify(data, null, 2) + '\n');
  console.log(`  ✓ ${filename} (${Object.keys(data).length} entries)`);
}

console.log('Extracting bestiary collections from HTML...\n');

// --- BASE_ATTACKS ---
const baseAttacksBlock = extractBlock(jsCode, 'const BASE_ATTACKS =');
if (baseAttacksBlock) {
  const data = evalObject(baseAttacksBlock);
  writeJSON('base_attacks.json', data);
}

// --- REAL_WORLD_DATA ---
const rwdBlock = extractBlock(jsCode, 'const REAL_WORLD_DATA =');
if (rwdBlock) {
  const data = evalObject(rwdBlock);
  writeJSON('real_world_data.json', data);
  // Count creatures
  let total = 0;
  for (const cat of Object.values(data)) total += Object.keys(cat).length;
  console.log(`    → ${total} creatures across ${Object.keys(data).length} categories`);
}

// --- SIZE_AND_AGE_MODIFIERS ---
const samBlock = extractBlock(jsCode, 'const SIZE_AND_AGE_MODIFIERS =');
if (samBlock) {
  writeJSON('size_and_age_modifiers.json', evalObject(samBlock));
}

// --- CONDITION_MODIFIERS ---
const cmBlock = extractBlock(jsCode, 'const CONDITION_MODIFIERS =');
if (cmBlock) {
  writeJSON('condition_modifiers.json', evalObject(cmBlock));
}

// --- BIOLOGICAL_UPGRADES ---
const buBlock = extractBlock(jsCode, 'const BIOLOGICAL_UPGRADES =');
if (buBlock) {
  writeJSON('biological_upgrades.json', evalObject(buBlock));
}

// --- PIGMENTATION ---
const pigBlock = extractBlock(jsCode, 'const PIGMENTATION =');
if (pigBlock) {
  writeJSON('pigmentation.json', evalObject(pigBlock));
}

// --- PATTERNS ---
const patBlock = extractBlock(jsCode, 'const PATTERNS =');
if (patBlock) {
  writeJSON('patterns.json', evalObject(patBlock));
}

// --- PACK_MULTIPLIERS ---
const pmBlock = extractBlock(jsCode, 'const PACK_MULTIPLIERS =');
if (pmBlock) {
  writeJSON('pack_multipliers.json', evalObject(pmBlock));
}

// --- MAX_PACK_LIMITS ---
const mplBlock = extractBlock(jsCode, 'const MAX_PACK_LIMITS =');
if (mplBlock) {
  writeJSON('max_pack_limits.json', evalObject(mplBlock));
}

console.log('\nDone! All collections extracted to:', OUT_DIR);
