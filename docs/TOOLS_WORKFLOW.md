# Tools Workflow

Project Epoch relies on external HTML/JS tools to generate data and UI layouts. We **do not hard-code** extensive data or UI positions in C++.

## 1. UI Workflow

**Goal**: Create or modify a screen layout.

1.  **Open Tool**: Launch `tools/UiArchitect.html` in your web browser.
2.  **Design**:
    *   Drag and drop widgets (Buttons, Labels, Containers).
    *   Set properties (Position, Size, Style, Action IDs).
    *   Create "Prefabs" for reusable components.
3.  **Export**: Click "Export JSON" to download `ui_layouts.json`.
4.  **Integrate**: Place the file in `assets/ui/ui_layouts.json`.
5.  **Runtime**: The Engine's `UIService` loads this file on startup.

> **Note**: Verify that texture paths in the tool match the actual paths in `assets/ui/`.

## 2. Data Workflow (Bestiary)

**Goal**: Create or balance a creature.

1.  **Open Tool**: Launch `tools/BestiaryArchitect.html` in your web browser.
2.  **Design**:
    *   Define Base Stats for a species.
    *   Define Role Modifiers (e.g., "Tank", "Scout").
3.  **Generate**: The tool runs the inference logic to generate thousands of variations.
4.  **Export**: Export the result as `epoch_database.sql`.
5.  **Integrate**:
    *   Run the SQL against your local SQLite database: `sqlite3 assets/database/epoch.db < epoch_database.sql`.
