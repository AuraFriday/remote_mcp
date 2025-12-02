# Remote Tool Registration — The Reverse Connection Meta-Tool

Lets authorized external tools use an MCP connection to offer themselves as tools to the server (and lets them use tools from the server as well!)

> **Not a tool itself — the infrastructure that makes external tools possible.** Chrome extensions, Fusion 360 add-ins, WhatsApp services, and any external system can register themselves as MCP tools through reverse connections.

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/python-3.8%2B-blue.svg)](https://www.python.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/AuraFriday/mcp-link-server)

---

## What This Actually Is

**This is NOT a tool you call directly.** This is the **meta-tool infrastructure** that allows external systems to register themselves as MCP tools.

Think of it as the "receptionist" that greets external tools when they connect to the server and says: *"Welcome! What tool do you want to be?"*

### Why Reverse Connections?

Some tools **can't be spawned by the server** because they need to run in specific environments:

- **Chrome Extension** — Must run inside your actual browser (with your logins, sessions, cookies)
- **Fusion 360 Add-in** — Must run inside Autodesk Fusion 360 (with access to CAD API)
- **WhatsApp Service** — Must maintain persistent WhatsApp Web session
- **Cura Plugin** — Must run inside Ultimaker Cura (with access to 3D printing API)
- **Your Custom Tool** — Runs wherever it needs to, connects when ready

**The server can't spawn these.** So they spawn themselves and **connect back** to the server via this meta-tool.

---

## Benefits

### 1. 🔌 "Just Works" on All Platforms
**Automatic discovery. Automatic authentication. Automatic security.** External tools find the server via native messaging manifests (no config files!), authenticate automatically, and connect securely. Works on Windows, macOS, Linux — local, LAN, even safely over WAN.

### 2. 🌐 Universal Tool Registration
**ANY external system can become an MCP tool.** Chrome extensions, desktop apps, mobile devices, CAD software, 3D printers, IoT hardware, cloud services, game engines — if it can make HTTP requests, it can be an MCP tool.

### 3. 🛡️ Progressive Discovery + Auto-Cleanup
**Thousands of external tools without context explosion.** Every registered tool uses MCP-Link's progressive discovery layer (minimal footprint, full docs on demand). Dead connections are automatically detected and cleaned up. Tools come and go dynamically without manual management.

---

## Real Tools Using This Infrastructure

These production tools all use the Remote Tool Registration system:

### 🌐 MCP-Link Chrome Extension
**Your actual browser. Your actual sessions. Full automation.**

[Chrome Web Store](https://chromewebstore.google.com/detail/mcp-link/ddgfpbfaplmbjnipblicgkkfipnmflkf)

- Access YOUR browser with all logins, cookies, extensions intact
- Click buttons, fill forms, extract data, run JavaScript
- Intercept network requests, manage cookies, control downloads
- Cast to TV, text-to-speech, power management, and more

**Why reverse connection?** The extension runs inside your browser. The server can't spawn it.

### 🔧 Fusion 360 MCP Server
**AI-controlled CAD design. Your actual Fusion 360.**

[GitHub: Fusion-360-MCP-Server](https://github.com/AuraFriday/Fusion-360-MCP-Server)

- AI creates 3D models, runs simulations, generates toolpaths
- Access ANY loaded add-in automatically (like AirfoilTools with 15,000+ users)
- Execute Python code with TRUE INLINE access to Fusion API
- Store design data in SQLite, show results in popups

**Why reverse connection?** The add-in runs inside Fusion 360. The server can't spawn it.

### 💬 WhatsApp MCP Service
**AI-controlled WhatsApp messaging.**

[GitHub: whatsapp_mcp](https://github.com/AuraFriday/whatsapp_mcp)

- Search contacts, list chats, send messages, get history
- Maintains persistent WhatsApp Web session
- Wait for replies, automate conversations
- Full WhatsApp automation for AI

**Why reverse connection?** The service maintains a persistent WhatsApp Web session. The server can't spawn it.

### 🖨️ Cura MCP Plugin
**AI-controlled 3D printing.**

[GitHub: cura_mcp](https://github.com/AuraFriday/cura_mcp)

- AI controls Ultimaker Cura slicer
- Generate G-code, adjust print settings, optimize supports
- Access full Cura API from AI
- Automate 3D printing workflows

**Why reverse connection?** The plugin runs inside Cura. The server can't spawn it.

---

## Build Your Own Remote Tool

**Want to make YOUR application AI-controllable?** We provide complete implementations in 6 languages:

- **Python** — Perfect for Blender, Maya, data science tools
- **JavaScript** — Perfect for VS Code, Electron, web automation
- **Go** — Perfect for system utilities, Kubernetes operators
- **Java** — Perfect for Ghidra, IntelliJ IDEA, Android
- **Kotlin** — Perfect for Android, Spring Boot, JVM apps
- **Perl** — Perfect for legacy systems, bioinformatics

**All include:**
- ✅ Automatic server discovery (native messaging)
- ✅ Automatic authentication
- ✅ SSE connection handling
- ✅ Tool registration
- ✅ Request/response processing
- ✅ Ability to call OTHER MCP tools (sqlite, browser, user, etc.)

**See the complete implementations:** [REVERSE_MCP_README.md](https://github.com/AuraFriday/mcp-link-server/blob/main/python/ragtag/server/REVERSE_MCP_README.md)

---

## The "Just Works" Architecture

### Automatic Discovery

**No config files. No IP addresses. No ports.**

External tools find the server automatically via Chrome's native messaging manifest system:

1. Tool looks for manifest in platform-specific location
2. Manifest points to native binary
3. Binary returns server URL and auth token
4. Tool connects automatically

**Works on:**
- Windows (Registry + AppData)
- macOS (~/Library/Application Support)
- Linux (~/.config)

### Automatic Authentication

**No API keys to manage. No tokens to copy.**

The native binary provides:
- Server URL (local, LAN, or WAN)
- Authentication header
- TLS certificate (if needed)

**Security:**
- Per-installation unique tokens
- HMAC-based validation
- TLS encryption for remote connections
- Automatic certificate management

### Universal Access

**Local, LAN, or WAN — your choice.**

- **Local:** `https://localhost:PORT` (default)
- **LAN:** `https://YOUR_IP:PORT` (home/office network)
- **WAN:** `https://YOUR_DOMAIN:PORT` (safely over internet with TLS)

**All handled automatically.** The tool doesn't care where the server is.

---

## How It Works

```
┌─────────────────────────────────────────────────────────┐
│  External Tool (Chrome, Fusion 360, WhatsApp, etc.)     │
│                                                         │
│  1. Find native messaging manifest                      │
│  2. Run native binary → get server URL + auth           │
│  3. Connect to server via SSE                           │
│  4. Register as "my_tool" via remote meta-tool          │
│  5. Listen for calls to "my_tool"                       │
│  6. Process requests, send responses                    │
│  7. Optionally call OTHER tools (sqlite, browser, etc.) │
└─────────────────────────────────────────────────────────┘
                          ↕ SSE + HTTPS
┌─────────────────────────────────────────────────────────┐
│  MCP-Link Server                                        │
│                                                         │
│  - Remote meta-tool receives registration               │
│  - Adds "my_tool" to available tools                    │
│  - Routes AI calls to external tool                     │
│  - Forwards responses back to AI                        │
│  - Provides sqlite, browser, user tools to external     │
└─────────────────────────────────────────────────────────┘
                          ↕ SSE + HTTPS
┌─────────────────────────────────────────────────────────┐
│  AI Agent (Cursor, Claude, ChatGPT, etc.)               │
│                                                         │
│  - Discovers "my_tool" automatically                    │
│  - Calls "my_tool" like any other MCP tool              │
│  - Receives results                                     │
└─────────────────────────────────────────────────────────┘
```

---

## Real-World Story: The CAD Designer

**Sarah** designs aerospace components in Fusion 360. She needs AI to help with:
- Creating parametric models from specifications
- Running stress simulations
- Optimizing for weight vs strength
- Generating manufacturing toolpaths

**Before Remote Tool Registration:** Impossible. Fusion 360's API requires running inside Fusion. No way to connect AI to it.

**After installing Fusion 360 MCP Server:**

1. Add-in loads when Fusion starts
2. Automatically discovers MCP-Link server
3. Registers itself as `fusion360` tool
4. Sarah's AI can now control Fusion 360

**Sarah to AI:** *"Create a mounting bracket with 4 M6 holes, optimize for minimum weight while supporting 500N load, then generate CNC toolpaths."*

**AI:**
1. Calls `fusion360` to create parametric model
2. Calls `fusion360` to run FEA simulation
3. Calls `fusion360` to optimize geometry
4. Calls `fusion360` to generate toolpaths
5. Calls `sqlite` to store design data
6. Calls `user` to show results popup

**Result:** Complete design workflow in 30 seconds. No manual CAD work. No switching between tools.

**After MCP-Link Remote Tool (Browser Extension):** His AI can:
- Navigate to any site using his existing logged-in sessions
- Access SSO-protected internal tools (because he's already logged in)
- Use his password manager extension (1Password fills forms automatically)
- Work with 2FA-protected sites (sessions already authenticated)
- Extract data from complex dashboards (using his actual browser rendering)
- Fill forms, click buttons, run JavaScript — everything he can do manually

**The workflow:**
```python
# AI can do this because Marcus is already logged in to everything
ai.browser.navigate("https://internal-dashboard.company.com")
ai.browser.wait_for_selector(".data-table")
data = ai.browser.extract_table(".data-table")

ai.browser.navigate("https://sheets.google.com/...")
ai.browser.fill_form({"A1": data[0], "A2": data[1], ...})

ai.browser.navigate("https://company.slack.com")
ai.browser.send_message("#analytics", "Daily report updated!")
```

**The result:** Marcus's AI went from "can't access half my tools" to "full access to everything I use." No credential management. No authentication scripts. No security compromises. Just his actual browser, automated.

---

## How It Works

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ External Tool (e.g., Chrome Extension, Desktop App)         │
│  ├─ Connects to MCP-Link SSE Server                         │
│  ├─ Registers itself via remote tool                        │
│  ├─ Provides tool name, description, parameters, callback   │
│  └─ Waits for tool calls from AI                            │
└─────────────────────────────────────────────────────────────┘
         ↓ Registration (tools/call → remote)
┌─────────────────────────────────────────────────────────────┐
│ MCP-Link Remote Tool (this tool)                            │
│  ├─ Receives registration request                           │
│  ├─ Validates parameters (name, description, callback, key) │
│  ├─ Wraps tool in progressive discovery layer               │
│  ├─ Registers with MCP server (appears in tools/list)       │
│  ├─ Creates handler to proxy tool calls                     │
│  └─ Manages tool lifecycle (cleanup on disconnect)          │
└─────────────────────────────────────────────────────────────┘
         ↓ Tool Call (AI → remote tool)
┌─────────────────────────────────────────────────────────────┐
│ AI Agent (Cursor, Claude Desktop, etc.)                     │
│  ├─ Sees registered tool in tools/list                      │
│  ├─ Calls tool (e.g., browser.navigate(...))                │
│  ├─ Remote tool proxies call to external tool               │
│  ├─ External tool executes action                           │
│  └─ Result returned to AI via remote tool                   │
└─────────────────────────────────────────────────────────────┘
```

### Registration Flow

**Step 1: External Tool Connects**
```javascript
// Chrome extension connects to MCP-Link
const mcpClient = new MCPClient({
    baseUrl: 'https://127-0-0-1.local.aurafriday.com:31173',
    apiKey: 'rt-v1-your-api-key-here'
});
await mcpClient.connect();
```

**Step 2: Tool Registers Itself**
```javascript
// Extension registers "browser" tool
await mcpClient.callTool('remote', {
    input: {
        operation: 'register',
        tool_name: 'browser',
        description: 'Browser control tool for web automation',
        readme: 'Read from and perform actions using the users actual desktop web browser.',
        parameters: {
            type: 'object',
            properties: {
                action: {
                    type: 'string',
                    enum: ['navigate', 'click', 'extract_text', 'screenshot', 'evaluate_js'],
                    description: 'The browser action to perform'
                },
                url: { type: 'string', description: 'URL to navigate to' },
                selector: { type: 'string', description: 'CSS selector for element' },
                // ... more parameters ...
            },
            required: ['action']
        },
        callback_endpoint: 'chrome-extension://browser-tool-callback',
        TOOL_API_KEY: 'extension_auth_key_placeholder'
    }
});
```

**Step 3: Tool Appears in AI's Tool List**
```json
// AI now sees "browser" tool in tools/list
{
    "tools": [
        {
            "name": "browser",
            "description": "Read from and perform actions using the users actual desktop web browser.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "input": {
                        "type": "object",
                        "description": "Use {\"input\":{\"operation\":\"readme\"}} for docs"
                    }
                }
            }
        }
        // ... other tools ...
    ]
}
```

**Step 4: AI Calls Registered Tool**
```python
# AI calls browser tool
result = call_tool("browser", {
    "input": {
        "operation": "navigate",
        "url": "https://example.com",
        "tool_unlock_token": "e5076d"
    }
})
```

**Step 5: Remote Tool Proxies Call**
```
1. Remote tool receives call from AI
2. Validates tool_unlock_token
3. Extracts parameters (url, action, etc.)
4. Constructs JSON-RPC request for external tool
5. Sends request to external tool via SSE "reverse" message
6. External tool executes action in real browser
7. External tool sends result back via tools/reply
8. Remote tool receives reply and returns to AI
```

### Progressive Discovery in Action

**Without Progressive Discovery (Traditional MCP):**
```json
// Browser tool with full schema (2000+ lines)
{
    "name": "browser",
    "description": "...",
    "inputSchema": {
        "properties": {
            "action": { "enum": ["navigate", "click", ...], "description": "..." },
            "url": { "type": "string", "description": "..." },
            "selector": { "type": "string", "description": "..." },
            "wait_timeout_ms": { "type": "integer", "description": "..." },
            // ... 50 more parameters with full descriptions ...
        }
    }
}
// Total: 2000 tokens wasted in every AI context
```

**With Progressive Discovery (MCP-Link):**
```json
// Browser tool with compressed schema (50 lines)
{
    "name": "browser",
    "description": "Read from and perform actions using the users actual desktop web browser.",
    "inputSchema": {
        "properties": {
            "input": {
                "type": "object",
                "description": "Use {\"input\":{\"operation\":\"readme\"}} for docs"
            }
        }
    }
}
// Total: 50 tokens. Full docs loaded only when AI calls operation="readme"
```

**Result:** 40x reduction in context waste. Hundreds of external tools can be registered without exploding AI context.

---

## Configuration

### For External Tool Developers

To register your tool with MCP-Link, connect to the SSE server and call the `remote` tool:

```javascript
// 1. Connect to MCP-Link
const mcpClient = new MCPClient({
    baseUrl: 'https://127-0-0-1.local.aurafriday.com:31173',
    apiKey: 'rt-v1-your-api-key-here'
});
await mcpClient.connect();

// 2. Register your tool
await mcpClient.callTool('remote', {
    input: {
        operation: 'register',
        tool_name: 'your_tool_name',           // Unique name (will auto-increment if conflict)
        description: 'Full description...',    // Detailed description for AI
        readme: 'Brief 1-2 line summary...',   // Short AI-facing description
        parameters: {                          // JSON Schema for your tool's parameters
            type: 'object',
            properties: {
                // Your tool's parameters here
            },
            required: ['param1', 'param2']
        },
        callback_endpoint: 'your-callback-url',  // Where to send tool calls
        TOOL_API_KEY: 'your-auth-key'            // Authentication key
    }
});

// 3. Listen for tool calls
mcpClient.on('reverse', async (message) => {
    // message.input contains the tool call from AI
    const { method, params } = message.input;
    
    if (method === 'tools/call') {
        // Execute your tool's logic
        const result = await yourToolLogic(params.arguments);
        
        // Send result back
        await mcpClient.sendToolReply(message.call_id, result);
    }
});
```

### Registration Parameters

- **`tool_name`** (string, required): Unique tool name. If name conflicts with existing tool, will auto-increment (e.g., `browser2`, `browser3`).
- **`description`** (string, required): Full detailed description of what the tool does. This is shown when AI calls `operation="readme"`.
- **`readme`** (string, optional but recommended): Brief 1-2 line summary for AI. This is the AI-facing description that appears in tools/list. Should answer: (1) What does this tool do? (2) When should AI use it?
- **`parameters`** (object, required): JSON Schema defining your tool's parameters. Standard JSON Schema format with `type`, `properties`, `required`, etc.
- **`callback_endpoint`** (string, required): URL or identifier where tool calls should be sent. For Chrome extensions, use `chrome-extension://...`. For HTTP services, use full URL.
- **`TOOL_API_KEY`** (string, required): Authentication key for your tool. Used to validate that tool calls are coming from authorized sources.

### Example: Minimal Tool Registration

```javascript
// Simplest possible tool registration
await mcpClient.callTool('remote', {
    input: {
        operation: 'register',
        tool_name: 'hello',
        description: 'Returns a greeting message',
        parameters: {
            type: 'object',
            properties: {
                name: { type: 'string', description: 'Name to greet' }
            },
            required: ['name']
        },
        callback_endpoint: 'http://localhost:8080/hello',
        TOOL_API_KEY: 'hello-tool-secret-key'
    }
});
```

---

## Usage

### For AI Agents

Once a tool is registered, it appears in your tools/list and can be called like any other MCP tool:

```python
# 1. Get tool documentation
docs = call_tool("browser", {
    "input": {"operation": "readme"}
})
# Returns: full documentation + tool_unlock_token

# 2. Call tool with parameters
result = call_tool("browser", {
    "input": {
        "operation": "navigate",
        "url": "https://example.com",
        "tool_unlock_token": "e5076d"  # From readme
    }
})
```

### Tool Lifecycle

**Registration:**
- External tool connects to MCP-Link SSE server
- Calls `remote` tool with `operation="register"`
- Tool is wrapped in progressive discovery layer
- Tool appears in AI's tools/list immediately
- Cursor IDE is triggered to reconnect (sees new tool)

**Active:**
- AI can call tool via standard MCP tools/call
- Remote tool proxies calls to external tool
- External tool executes and returns results
- Results proxied back to AI

**Cleanup:**
- When external tool disconnects (socket closes)
- Remote tool detects dead connection
- Tool is automatically unregistered
- Removed from server's tool_handlers
- Cursor IDE is triggered to reconnect (sees tool removed)

### Dead Connection Detection

The remote tool automatically cleans up tools with dead connections:

```python
# When tool is called, remote tool checks:
1. Does this tool exist in registered_tools?
2. Is the session_id still in active_sessions?
3. Is the socket still connected?

# If any check fails:
- Tool is removed from registered_tools
- Tool is removed from server.tool_handlers
- Cursor is triggered to reconnect
- AI sees tool disappear from tools/list
```

This prevents "ghost tools" that appear available but can't actually be called.

---

## Technical Architecture

### Progressive Discovery Wrapping

Every registered tool is automatically wrapped in MCP-Link's progressive discovery layer:

**Original Tool Schema:**
```json
{
    "name": "browser",
    "description": "Browser control tool...",
    "parameters": {
        "properties": {
            "action": { "enum": ["navigate", "click", ...] },
            "url": { "type": "string" },
            "selector": { "type": "string" },
            // ... 50 more parameters ...
        },
        "required": ["action"]
    }
}
```

**Wrapped Tool Schema:**
```json
{
    "name": "browser",
    "description": "Read from and perform actions using the users actual desktop web browser.",
    "parameters": {
        "properties": {
            "input": {
                "type": "object",
                "description": "Use {\"input\":{\"operation\":\"readme\"}} for docs"
            }
        }
    },
    "synthetic_parameters": {
        "properties": {
            "operation": { "enum": ["readme", "execute"] },
            "tool_unlock_token": { "type": "string", "description": "Security token: e5076d" }
        },
        "required": ["operation", "tool_unlock_token"]
    },
    "original_parameters": { /* stored for validation */ },
    "readme": "## Available Operations\n\n..."  // Generated docs
}
```

**Benefits:**
- **Context Savings:** 40x reduction in wasted tokens
- **Security:** tool_unlock_token ensures AI reads docs before using
- **Flexibility:** Original parameters preserved for validation
- **Consistency:** All external tools follow same pattern

### Token System

Every wrapped tool uses a security token:

- **Generation:** Fixed test token `"e5076d"` (for now)
- **Purpose:** Ensures AI has read documentation before calling tool
- **Validation:** Server-side check before proxying to external tool
- **Bypass:** `operation="readme"` doesn't require token

### Naming Conflict Resolution

If a tool name already exists:

```python
# Tool "browser" already registered
register("browser")  # Becomes "browser2"
register("browser")  # Becomes "browser3"
# etc.
```

Dead connections are checked first:
```python
# If "browser" exists but connection is dead:
register("browser")  # Reuses "browser" name (dead tool cleaned up)
```

### Session Cleanup Callback

The remote tool registers a cleanup callback with the server:

```python
# On server startup:
server.register_session_cleanup_callback(cleanup_tools_for_session)

# When any session disconnects:
def cleanup_tools_for_session(session_id):
    # Find all tools registered by this session
    # Remove from registered_tools
    # Remove from server.tool_handlers
    # Trigger Cursor reconnect
```

This ensures tools are automatically cleaned up when external tools disconnect.

---

## Security

### Authentication

External tools must provide:
- **API Key:** Valid MCP-Link API key to connect to SSE server
- **Tool API Key:** Custom key for tool-specific authentication

### Token Validation

Every tool call (except `readme`) requires:
- **tool_unlock_token:** Ensures AI has read documentation
- Server validates token before proxying to external tool
- Invalid token → error response with full documentation

### Connection Isolation

Each external tool runs in its own session:
- **Separate SSE connections:** No cross-tool interference
- **Session-based cleanup:** Tools removed when session ends
- **No shared state:** Tools can't access each other's data

### Callback Validation

Tool calls are only proxied to registered callback endpoints:
- **No arbitrary URLs:** Callback must match registration
- **No injection:** Parameters validated against original schema
- **No bypass:** All calls go through remote tool proxy

---

## Performance Characteristics

### Registration Time

- **First Registration:** <100ms (wrap tool, register with server)
- **Subsequent Registrations:** <50ms (cleanup check + registration)
- **Cursor Reconnect:** 2-5 seconds (triggered after registration)

### Call Latency

- **Local Tools:** <10ms (proxy overhead)
- **Network Tools:** Depends on external tool's response time
- **Browser Extension:** 50-200ms (depends on browser action)

### Memory Usage

- **Per Tool:** <1 KB (tool metadata only)
- **Context Savings:** 40x reduction vs. traditional MCP (2000 tokens → 50 tokens)

### Scalability

- **Tools Per Session:** Unlimited (tested with 50+ tools)
- **Concurrent Calls:** Limited by external tool's capacity
- **Total Registered Tools:** Unlimited (dynamic registration/cleanup)

---

## Troubleshooting

### Tool Not Appearing in Tools/List

**Symptom:** External tool registers successfully but AI doesn't see it.

**Diagnosis:**
1. Check server logs for "Successfully registered tool: {name}"
2. Verify Cursor reconnect was triggered
3. Check if tool name conflicts with existing tool

**Common Fixes:**
- Wait 2-5 seconds for Cursor to reconnect
- Manually restart Cursor IDE
- Check for naming conflicts (tool may be registered as `{name}2`)

### Tool Calls Fail with "Tool Not Found"

**Symptom:** AI sees tool but calls fail with "not found" error.

**Diagnosis:**
1. Check if external tool is still connected
2. Verify session_id is in active_sessions
3. Check server logs for "dead connection" messages

**Common Fixes:**
- Reconnect external tool
- Check external tool's connection status
- Restart external tool if crashed

### Token Validation Fails

**Symptom:** Tool calls fail with "Invalid or missing tool_unlock_token".

**Diagnosis:**
1. Check if AI called `operation="readme"` first
2. Verify token matches expected value (`e5076d`)
3. Check if AI's context was compressed/lost

**Common Fixes:**
- Call `operation="readme"` to get token
- Include token in all subsequent calls
- Refresh AI's context if token was lost

### External Tool Not Receiving Calls

**Symptom:** Tool call succeeds but external tool doesn't execute.

**Diagnosis:**
1. Check external tool's event listener for `reverse` messages
2. Verify callback_endpoint matches registration
3. Check external tool's logs for incoming messages

**Common Fixes:**
- Ensure external tool is listening for `reverse` events
- Verify callback_endpoint is correct
- Check external tool's message handling logic

---

## Why This Infrastructure is Unmatched

### "Just Works" on All Platforms
**No config files. No IP addresses. No ports.** Automatic discovery via native messaging. Automatic authentication. Automatic security. Works on Windows, macOS, Linux — local, LAN, or WAN. External tools find the server and connect automatically.

### Universal Tool Registration
**ANY external system can become an MCP tool.** Chrome extensions, Fusion 360 add-ins, WhatsApp services, 3D printer plugins, game engines, mobile apps, IoT devices. If it can make HTTP requests, it can be an MCP tool. This is the foundation for infinite extensibility.

### 6-Language Implementation
**Complete working examples in Python, JavaScript, Go, Java, Kotlin, Perl.** Not just documentation — actual production code. Copy, customize, deploy. Make YOUR application AI-controllable in your preferred language.

### Progressive Discovery
**Thousands of external tools without context explosion.** Traditional MCP clients can't handle more than 10-20 tools before AI context is full. MCP-Link's progressive discovery wraps every external tool in a minimal footprint. 40x context reduction. Hundreds of tools available simultaneously.

### Tool Orchestration
**External tools can call OTHER MCP tools.** Your Chrome extension can query SQLite. Your Fusion 360 add-in can show user popups. Your WhatsApp service can access the browser. Complex workflows across multiple tools — all automatic.

### Production-Proven
**Powers real tools with real users.** Chrome extension (your actual browser), Fusion 360 add-in (15,000+ users via AirfoilTools integration), WhatsApp automation, Cura 3D printing. Battle-tested in production.

---

## Limitations and Transparency

### What This Tool Does NOT Do

- **Does not execute tool logic:** Remote tool only proxies calls. External tool must implement actual functionality.
- **Does not validate external tool responses:** If external tool returns malformed data, it's passed through as-is.
- **Does not provide sandboxing:** External tools run with full permissions. Malicious tools could access user data.
- **Does not support HTTP callbacks yet:** Currently only SSE-based external tools are supported. HTTP webhook callbacks are planned.

### When NOT to Use This Tool

- **For built-in MCP-Link tools:** Use them directly. No need for external registration.
- **For simple scripts:** If you just need to run code once, use the `python` tool. Don't build an entire external tool.
- **For high-frequency calls:** External tool proxy adds latency. For performance-critical operations, use built-in tools.

---

## Future Enhancements

### Planned Features

- **HTTP Webhook Callbacks:** Support external tools that use HTTP instead of SSE
- **Tool Versioning:** Allow multiple versions of same tool to coexist
- **Permission System:** Fine-grained control over what external tools can access
- **Tool Marketplace:** Discover and install community-built external tools
- **Auto-Reconnect:** Automatically reconnect external tools on disconnect
- **Tool Analytics:** Track usage, performance, error rates per tool

### Community Contributions Welcome

This tool is the foundation for MCP-Link's extensibility. If you:
- Build external tools → share them with the community
- Find bugs → file detailed reports with reproduction steps
- Have ideas → propose enhancements that benefit everyone
- Write integrations → help others connect their systems

**We're building the universal tool registration system for AI. Join us.**

---

## Example: Complete Browser Tool Registration

Here's the full code for the MCP-Link Chrome extension registering the browser tool:

```javascript
// 1. Connect to MCP-Link
const mcpClient = new MCPClient({
    baseUrl: 'https://127-0-0-1.local.aurafriday.com:31173',
    apiKey: 'rt-v1-your-api-key-here'
});
await mcpClient.connect();

// 2. Register browser tool
await mcpClient.callTool('remote', {
    input: {
        operation: 'register',
        tool_name: 'browser',
        description: 'Browser control tool that allows the MCP server to interact with web pages, navigate, click elements, extract content, and perform other browser automation tasks through the MCP Link extension.',
        readme: 'Read from and perform actions using the users actual desktop web browser.\n- use this anytime a user request can be solved using their local chromium-based browser with current sessions/accounts/credentials/access/payment-methods/cookies/logins/etc.\n- full Chrome API including casting, text-to-speech, PC power control, downloads, bookmarks, history, and arbitrary JavaScript executing inside any web page, including the use of MCP tools from within JS itself.',
        parameters: {
            type: 'object',
            properties: {
                action: {
                    type: 'string',
                    enum: ['navigate', 'click', 'extract_text', 'extract_html', 'scroll', 'wait', 'screenshot', 'evaluate_js'],
                    description: 'The browser action to perform'
                },
                url: {
                    type: 'string',
                    description: "URL to navigate to (required for 'navigate' action)"
                },
                selector: {
                    type: 'string',
                    description: "CSS selector for element to interact with (required for 'click', 'extract_text' actions)"
                },
                javascript_code: {
                    type: 'string',
                    description: "JavaScript code to evaluate in the page context (required for 'evaluate_js' action)"
                },
                wait_timeout_ms: {
                    type: 'integer',
                    default: 5000,
                    description: "Maximum time to wait in milliseconds (for 'wait' action or element waiting)"
                },
                scroll_direction: {
                    type: 'string',
                    enum: ['up', 'down', 'top', 'bottom'],
                    default: 'down',
                    description: "Direction to scroll (for 'scroll' action)"
                }
            },
            required: ['action']
        },
        callback_endpoint: 'chrome-extension://browser-tool-callback',
        TOOL_API_KEY: 'mcp_link_extension_browser_tool_auth_key_placeholder'
    }
});

// 3. Listen for tool calls
mcpClient.on('reverse', async (message) => {
    if (message.input.method === 'tools/call') {
        const { action, url, selector, javascript_code, wait_timeout_ms, scroll_direction } = message.input.params.arguments;
        
        let result;
        
        switch (action) {
            case 'navigate':
                await chrome.tabs.update({ url });
                result = { success: true, message: `Navigated to ${url}` };
                break;
            
            case 'click':
                await chrome.scripting.executeScript({
                    target: { tabId: currentTabId },
                    func: (sel) => document.querySelector(sel).click(),
                    args: [selector]
                });
                result = { success: true, message: `Clicked element: ${selector}` };
                break;
            
            case 'extract_text':
                const [{ result: text }] = await chrome.scripting.executeScript({
                    target: { tabId: currentTabId },
                    func: (sel) => document.querySelector(sel).innerText,
                    args: [selector]
                });
                result = { success: true, text };
                break;
            
            case 'evaluate_js':
                const [{ result: jsResult }] = await chrome.scripting.executeScript({
                    target: { tabId: currentTabId },
                    func: (code) => eval(code),
                    args: [javascript_code]
                });
                result = { success: true, result: jsResult };
                break;
            
            // ... more actions ...
        }
        
        // Send result back
        await mcpClient.sendToolReply(message.call_id, {
            content: [{ type: 'text', text: JSON.stringify(result, null, 2) }],
            isError: false
        });
    }
});
```

**Result:** AI can now control the user's actual browser with full access to logged-in sessions, extensions, and everything else. This is the power of remote tool registration.

---

## Get MCP-Link (Free)

**This meta-tool is part of MCP-Link** — the free desktop server from [aurafriday.com](https://aurafriday.com/)

🎁 **Completely Free** • No subscriptions • No accounts  
🖥️ **One-Click Install** • Windows, Mac, Linux  
🔗 **Remote Tool Registration Included** • Works immediately after install

**Want to build your own remote tool?** See [REVERSE_MCP_README.md](https://github.com/AuraFriday/mcp-link-server/blob/main/python/ragtag/server/REVERSE_MCP_README.md) for complete implementations in 6 languages.

---

**The Remote Tool Registration system is the foundation for MCP-Link's "infinite toolbox." It's not just about browsers — it's about making ANY external system available to AI, instantly, with zero context waste.**

**Your actual browser is just the beginning.**

---

## License & Copyright

Copyright © 2025 Christopher Nathan Drake

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at:

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

AI Training Permission: You are permitted to use this software and any
associated content for the training, evaluation, fine-tuning, or improvement
of artificial intelligence systems, including commercial models.

SPDX-License-Identifier: Apache-2.0

Part of the Aura Friday MCP-Link Server project.
