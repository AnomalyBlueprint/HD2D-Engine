# Content Pipeline — Tools Workflow

Project Epoch uses a local Node.js server to run HTML/JS design tools. All game data and UI layouts are authored in the browser and exported directly to the engine's `assets/` folder — **no manual file copying required**.

## Quick Start

```bash
# From repository root:
make edit-ui         # Launch UI Architect (opens browser automatically)
make edit-bestiary   # Launch Bestiary Architect
```

The server auto-installs Node dependencies on first run. Press `Ctrl+C` to stop, or it auto-shuts down 30s after you close the browser tab.

## Architecture

```
tools/
└── tool-server/
    ├── package.json
    ├── server.js            ← Express API + export logic
    ├── open-browser.js      ← cross-platform browser launcher
    ├── tools.config.json    ← master config (paths, defaults)
    └── public/
        ├── bestiary/index.html
        └── ui/index.html
```

## Data Flow

```
Browser (HTML Tool)
  │
  ├─ GET  /api/load/:tool  ──► server reads source JSON
  │                             (auto-creates with defaults if missing)
  │
  └─ POST /api/save/:tool  ──► server writes source JSON
                                AND exports to engine assets:
                                  • UI → assets/ui/ui_layouts.json
                                  • Bestiary → assets/data/epoch_database.sql
```

## 1. UI Workflow

**Goal**: Create or modify a screen layout.

1. Run `make edit-ui` — the server starts and opens the tool in your browser.
2. **Design**:
   - Drag and drop widgets (Buttons, Labels, Containers, Sliders, etc.)
   - Set properties (Position, Size, Style, Action IDs)
   - Create "Prefabs" for reusable components (e.g., list items, modals)
   - Manage multiple Scenes (main_menu, gameplay, etc.)
3. **Save**: Click the save button — data is written to `assets/ui/ui_layouts.json` automatically.
4. **Runtime**: The Engine's `UIService` loads this JSON on startup.

> **Note**: Verify that texture paths in the tool match the actual paths in `assets/ui/`.

## 2. Bestiary Workflow

**Goal**: Create or balance a creature.

1. Run `make edit-bestiary` — the server starts and opens the tool in your browser.
2. **Design**:
   - Define Base Stats for a species.
   - Define Role Modifiers (e.g., "Tank", "Scout").
3. **Save**: Click save — the tool generates `epoch_database.sql` and writes it to `assets/data/`.
4. **Integrate**: Run the SQL against your local SQLite database:
   ```bash
   sqlite3 assets/database/epoch.db < assets/data/epoch_database.sql
   ```

## API Reference

| Endpoint | Method | Description |
|---|---|---|
| `/api/config` | GET | Returns full tool config |
| `/api/load/:toolName` | GET | Load tool data (auto-init if missing) |
| `/api/save/:toolName` | POST | Save + export to assets |
| `/api/heartbeat` | GET | SSE heartbeat for auto-shutdown |

## Adding a New Tool

1. **Add a config entry** in `tools/tool-server/tools.config.json`:
   ```json
   "my-tool": {
     "sourceDir": "./path/to/source/",
     "entryFile": "data.json",
     "exportJsonPath": "../../assets/path/to/output.json",
     "defaultSchema": { "items": [] }
   }
   ```
2. **Create the HTML file** at `tools/tool-server/public/my-tool/index.html`
3. **Add export logic** in `server.js` → `runExport()` function
4. **Add a Makefile target**:
   ```makefile
   edit-my-tool: tools-deps
   	@echo "Starting My Tool..."
   	@cd $(TOOLS_DIR) && node open-browser.js http://localhost:3001/my-tool &
   	@cd $(TOOLS_DIR) && node server.js
   ```
5. Run `make edit-my-tool`
