/**
 * Cross-platform browser launcher.
 * Usage: node open-browser.js <url>
 * 
 * Uses the 'open' npm package to launch the default browser
 * on macOS, Windows, and Linux without OS-specific shell commands.
 */

const url = process.argv[2];

if (!url) {
  console.error('Usage: node open-browser.js <url>');
  process.exit(1);
}

// Small delay to let the server start before opening the browser
setTimeout(async () => {
  try {
    const open = (await import('open')).default;
    await open(url);
    console.log(`[BROWSER] Opened: ${url}`);
  } catch (err) {
    console.warn(`[BROWSER] Could not open browser automatically. Please visit: ${url}`);
  }
}, 1500);
