/**
 * HD2D-Engine Content Pipeline Server
 * 
 * Serves HTML tools and provides API endpoints for reading/writing
 * game data files directly to the engine's assets/ folder.
 * 
 * Features:
 *   - GET /api/config           → master config
 *   - GET /api/collections/:tool → merged collections from manifest
 *   - GET /api/load/:tool       → saved tool data (creatures / layouts)
 *   - POST /api/save/:tool      → save + export to assets/
 *   - GET /api/heartbeat        → SSE heartbeat for auto-shutdown
 */

const express = require('express');
const fs = require('fs');
const path = require('path');

// --- Configuration ---
const PORT = process.env.PORT || 3001;
const TOOLS_ROOT = __dirname;
const CONFIG_PATH = path.join(TOOLS_ROOT, 'tools.config.json');
const IDLE_TIMEOUT_MS = 30_000; // 30s with no clients → auto-shutdown

// Load master config
let toolsConfig;
try {
  const raw = fs.readFileSync(CONFIG_PATH, 'utf-8');
  toolsConfig = JSON.parse(raw);
  console.log(`[CONFIG] Loaded ${Object.keys(toolsConfig).length} tool(s): ${Object.keys(toolsConfig).join(', ')}`);
} catch (err) {
  console.error(`[FATAL] Failed to load config from ${CONFIG_PATH}:`, err.message);
  process.exit(1);
}

const app = express();

// --- Middleware ---
app.use(express.json({ limit: '50mb' }));
app.use(express.static(path.join(__dirname, 'public')));
app.use('/assets', express.static(path.join(TOOLS_ROOT, '../../assets')));

// ============================================================
// API Routes
// ============================================================

/** GET /api/config — Returns the full tool configuration. */
app.get('/api/config', (_req, res) => {
  res.json(toolsConfig);
});

/**
 * GET /api/collections/:toolName
 * Reads the tool's manifest.json, then loads + merges every referenced
 * JSON file into a single response object.
 */


/**
 * GET /api/load/:toolName
 * Returns the saved tool data (e.g. creature library, UI layouts).
 * Auto-creates default schema if file doesn't exist.
 */
app.get('/api/load/:toolName', (req, res) => {
  const toolName = req.params.toolName;
  const config = toolsConfig[toolName];
  if (!config) return res.status(404).json({ error: `Unknown tool: "${toolName}"` });

  const sourceDir = path.resolve(TOOLS_ROOT, config.sourceDir);
  const filePath = path.join(sourceDir, config.entryFile);

  try {
    if (!fs.existsSync(filePath)) {
      console.log(`[INIT] Source file missing for "${toolName}". Creating default at: ${filePath}`);
      fs.mkdirSync(sourceDir, { recursive: true });
      fs.writeFileSync(filePath, JSON.stringify(config.defaultSchema, null, 2), 'utf-8');
    }
    const data = fs.readFileSync(filePath, 'utf-8');
    res.json(JSON.parse(data));
  } catch (err) {
    console.error(`[ERROR] Failed to load "${toolName}":`, err.message);
    res.status(500).json({ error: `Failed to load data for "${toolName}"`, details: err.message });
  }
});

/**
 * POST /api/save/:toolName
 * Saves JSON to source_data AND runs tool-specific export logic.
 */
app.post('/api/save/:toolName', (req, res) => {
  const toolName = req.params.toolName;
  const config = toolsConfig[toolName];
  if (!config) return res.status(404).json({ error: `Unknown tool: "${toolName}"` });

  const sourceDir = path.resolve(TOOLS_ROOT, config.sourceDir);
  const filePath = path.join(sourceDir, config.entryFile);
  const data = req.body;

  try {
    // 1. Save to source_data
    fs.mkdirSync(sourceDir, { recursive: true });
    fs.writeFileSync(filePath, JSON.stringify(data, null, 2), 'utf-8');
    console.log(`[SAVE] Saved source data for "${toolName}" → ${filePath}`);

    // 2. Run export logic
    const exportResults = runExport(toolName, config, data);

    res.json({
      success: true,
      message: `Saved "${toolName}" data successfully.`,
      exports: exportResults
    });
  } catch (err) {
    console.error(`[ERROR] Failed to save "${toolName}":`, err.message);
    res.status(500).json({ error: `Failed to save data for "${toolName}"`, details: err.message });
  }
});

// ============================================================
// Heartbeat Auto-Shutdown (SSE)
// ============================================================

const activeClients = new Set();
let idleTimer = null;

function resetIdleTimer() {
  if (idleTimer) clearTimeout(idleTimer);
  idleTimer = null;
}

function startIdleTimer() {
  if (idleTimer) return; // already running
  idleTimer = setTimeout(() => {
    console.log(`\n[AUTO-SHUTDOWN] No clients connected for ${IDLE_TIMEOUT_MS / 1000}s. Shutting down...`);
    server.close(() => {
      console.log('[SERVER] Closed.');
      process.exit(0);
    });
    // Force exit after 5s if close hangs
    setTimeout(() => process.exit(0), 5000);
  }, IDLE_TIMEOUT_MS);
}

/**
 * GET /api/heartbeat
 * Server-Sent Events endpoint. Browser connects on page load,
 * disconnects on tab close. Server tracks active connections and
 * auto-shuts down after IDLE_TIMEOUT_MS with no clients.
 */
app.get('/api/heartbeat', (req, res) => {
  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    'Connection': 'keep-alive'
  });
  res.write('data: connected\n\n');

  const clientId = Symbol();
  activeClients.add(clientId);
  resetIdleTimer();
  console.log(`[HEARTBEAT] Client connected (${activeClients.size} active)`);

  // Ping every 15s to keep connection alive
  const pingInterval = setInterval(() => {
    res.write('data: ping\n\n');
  }, 15_000);

  req.on('close', () => {
    clearInterval(pingInterval);
    activeClients.delete(clientId);
    console.log(`[HEARTBEAT] Client disconnected (${activeClients.size} active)`);
    if (activeClients.size === 0) {
      startIdleTimer();
    }
  });
});

// ============================================================
// Export Logic
// ============================================================

function runExport(toolName, config, data) {
  const results = [];

  if (toolName === 'bestiary' && config.exportSqlPath) {
    const sqlPath = path.resolve(TOOLS_ROOT, config.exportSqlPath);
    const sql = generateBestiarySql(data, config);
    fs.mkdirSync(path.dirname(sqlPath), { recursive: true });
    fs.writeFileSync(sqlPath, sql, 'utf-8');
    results.push({ type: 'sql', path: sqlPath, size: sql.length });
    console.log(`[EXPORT] Wrote SQL (${sql.length} bytes) → ${sqlPath}`);
  }

  if (toolName === 'ui' && config.exportJsonPath) {
    const jsonPath = path.resolve(TOOLS_ROOT, config.exportJsonPath);
    const json = JSON.stringify(data, null, 2);
    fs.mkdirSync(path.dirname(jsonPath), { recursive: true });
    fs.writeFileSync(jsonPath, json, 'utf-8');
    results.push({ type: 'json', path: jsonPath, size: json.length });
    console.log(`[EXPORT] Wrote JSON (${json.length} bytes) → ${jsonPath}`);
  }

  return results;
}

// ============================================================
// Bestiary SQL Generator
// Reads collection JSONs from source_data/ instead of hardcoding.
// ============================================================

function escSql(str) {
  if (str === null || str === undefined) return 'NULL';
  return "'" + String(str).replace(/'/g, "''") + "'";
}


function generateBestiarySql(data) {
  const creatures = data.creatures || [];
  
  // Load reference data from STATIC directory (public/bestiary/data)
  const staticDataDir = path.join(TOOLS_ROOT, 'public/bestiary/data');
  const readJson = (file) => {
    try {
      return JSON.parse(fs.readFileSync(path.join(staticDataDir, file), 'utf-8'));
    } catch (e) {
      console.warn(`[WARN] Missing static data file: ${file}`);
      return {};
    }
  };

  const HARVEST_TYPES = readJson('harvest_types.json');
  const ANATOMY_PARTS = readJson('anatomy_parts.json');
  const hardLoot = readJson('hard_loot_modifiers.json');
  const organicLoot = readJson('organic_loot_modifiers.json');
  const baseAttacks = readJson('base_attacks.json');
  const prefixes = readJson('prefixes.json');

  let sql = `-- Project Epoch Database Export\n`;
  sql += `-- Generated on: ${new Date().toISOString()}\n`;
  sql += `-- Creatures: ${creatures.length} | Base Attacks: ${Object.keys(baseAttacks).length}\n\n`;

  sql += 'PRAGMA foreign_keys = ON;\n\n';

  // [1] Harvest Types
  sql += '-- [1] HARVEST TYPES\n';
  sql += 'CREATE TABLE IF NOT EXISTS Harvest_Types (id TEXT PRIMARY KEY, tool TEXT, attribute TEXT, speed TEXT, time_sec INTEGER);\n';
  for (const [id, v] of Object.entries(HARVEST_TYPES)) {
    sql += `INSERT OR IGNORE INTO Harvest_Types VALUES (${escSql(id)}, ${escSql(v.toolNeeded)}, ${escSql(v.attribute)}, ${escSql(v.speed)}, ${v.baseTimeSeconds});\n`;
  }
  sql += '\n';

  // [2] Anatomy Parts
  sql += '-- [2] ANATOMY PARTS\n';
  sql += 'CREATE TABLE IF NOT EXISTS Anatomy_Parts (id TEXT PRIMARY KEY, default_harvest TEXT, drops TEXT);\n';
  for (const [id, v] of Object.entries(ANATOMY_PARTS)) {
    sql += `INSERT OR IGNORE INTO Anatomy_Parts VALUES (${escSql(id)}, ${escSql(v.defaultHarvest)}, ${escSql(v.baseDrops.join(','))});\n`;
  }
  sql += '\n';

  // [3] Loot Modifiers
  sql += '-- [3] LOOT MODIFIERS\n';
  sql += 'CREATE TABLE IF NOT EXISTS Loot_Modifiers (id TEXT PRIMARY KEY, type TEXT, multiplier REAL, quality TEXT, weight INTEGER);\n';
  for (const [id, v] of Object.entries(hardLoot)) {
    sql += `INSERT OR IGNORE INTO Loot_Modifiers VALUES (${escSql(id)}, 'HARD', ${v.valueMult}, ${escSql(v.craftQuality)}, ${v.dropWeight});\n`;
  }
  for (const [id, v] of Object.entries(organicLoot)) {
    sql += `INSERT OR IGNORE INTO Loot_Modifiers VALUES (${escSql(id)}, 'ORGANIC', ${v.valueMult}, ${escSql(v.craftQuality)}, ${v.dropWeight});\n`;
  }
  sql += '\n';

  // [4] Attack Definitions
  sql += '-- [4] ATTACK DEFINITIONS\n';
  sql += 'CREATE TABLE IF NOT EXISTS Attack_Defs (\n';
  sql += '  id INTEGER PRIMARY KEY AUTOINCREMENT,\n';
  sql += '  name TEXT UNIQUE,\n';
  sql += '  category TEXT,\n';
  sql += '  effect TEXT,\n';
  sql += '  base_damage TEXT,\n';
  sql += '  anatomy_tag TEXT\n';
  sql += ');\n\n';
  sql += 'BEGIN TRANSACTION;\n';

  // Base attacks
  for (const [name, v] of Object.entries(baseAttacks)) {
    sql += `INSERT OR IGNORE INTO Attack_Defs (name, category, effect, base_damage, anatomy_tag) VALUES (${escSql(name)}, ${escSql(v.type)}, ${escSql(v.effect)}, ${escSql(v.baseDamage)}, ${escSql(v.anatomyTag)});\n`;
  }

  // Expanded attacks with prefixes
  for (const [name, v] of Object.entries(baseAttacks)) {
    for (const [prefix, mod] of Object.entries(prefixes)) {
      const parts = v.baseDamage.split('d');
      let newDamage = '0';
      if (parts.length === 2) {
        const newCount = Math.max(1, parseInt(parts[0]) + mod.diceMod);
        newDamage = `${newCount}d${parts[1]}`;
      }
      sql += `INSERT OR IGNORE INTO Attack_Defs (name, category, effect, base_damage, anatomy_tag) VALUES (${escSql(prefix + name)}, ${escSql(v.type)}, ${escSql(v.effect)}, ${escSql(newDamage)}, ${escSql(v.anatomyTag)});\n`;
    }
  }
  sql += 'COMMIT;\n\n';

  // [5] Creature Definitions
  sql += '-- [5] CREATURE GENETICS\n';
  sql += 'CREATE TABLE IF NOT EXISTS Creature_Defs (\n';
  sql += '  id TEXT PRIMARY KEY,\n';
  sql += '  name TEXT,\n';
  sql += '  type TEXT,\n';
  sql += '  size_index INTEGER,\n';
  sql += '  cr REAL,\n';
  sql += '  hp INTEGER,\n';
  sql += '  ac INTEGER,\n';
  sql += '  biomes_json TEXT\n';
  sql += ');\n\n';

  // [6] Creature <-> Attack Link
  sql += '-- [6] CREATURE <-> ATTACK LINK\n';
  sql += 'CREATE TABLE IF NOT EXISTS Creature_Attacks (\n';
  sql += '  creature_id TEXT,\n';
  sql += '  attack_name TEXT,\n';
  sql += '  FOREIGN KEY(creature_id) REFERENCES Creature_Defs(id),\n';
  sql += '  FOREIGN KEY(attack_name) REFERENCES Attack_Defs(name)\n';
  sql += ');\n\n';

  // Insert creatures
  sql += 'BEGIN TRANSACTION;\n';
  for (const c of creatures) {
    const id = c.id || `creature_${Date.now()}_${Math.random().toString(36).slice(2, 8)}`;
    sql += `INSERT INTO Creature_Defs (id, name, type, size_index, cr, hp, ac, biomes_json) VALUES (${escSql(id)}, ${escSql(c.name || 'Unknown')}, ${escSql(c.type || 'Beast')}, ${c.sizeIdx ?? c.size_index ?? 4}, ${c.cr ?? 0}, ${c.hp ?? 1}, ${c.ac ?? 10}, ${escSql(JSON.stringify(c.biomes || []))});\n`;
    for (const atk of (c.attacks || [])) {
      sql += `INSERT INTO Creature_Attacks (creature_id, attack_name) VALUES (${escSql(id)}, ${escSql(atk)});\n`;
    }
  }
  sql += 'COMMIT;\n\n';

  return sql;
}

// ============================================================
// Server Startup
// ============================================================

const server = app.listen(PORT, () => {
  console.log('');
  console.log('╔══════════════════════════════════════════════╗');
  console.log('║   HD2D-Engine Content Pipeline Server        ║');
  console.log('╠══════════════════════════════════════════════╣');
  console.log(`║   Local:  http://localhost:${PORT}              ║`);
  console.log('║                                              ║');
  console.log(`║   Bestiary:  http://localhost:${PORT}/bestiary   ║`);
  console.log(`║   UI:        http://localhost:${PORT}/ui         ║`);
  console.log('║                                              ║');
  console.log('║   Auto-shutdown: 30s after last tab closes   ║');
  console.log('╚══════════════════════════════════════════════╝');
  console.log('');
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\n[SERVER] Shutting down gracefully...');
  server.close(() => {
    console.log('[SERVER] Closed.');
    process.exit(0);
  });
});
