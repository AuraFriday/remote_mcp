#!/usr/bin/env node
/**
 * File: reverse_mcp.js
 * Project: Aura Friday MCP-Link Server - Remote Tool Provider Demo
 * Component: Registers a demo tool with the MCP server and handles reverse calls
 * Author: Christopher Nathan Drake (cnd)
 * Created: 2025-11-03
 * Last Modified: 2025-11-03 by cnd (JavaScript port from Python)
 * SPDX-License-Identifier: Apache-2.0
 * Copyright: (c) 2025 Christopher Nathan Drake. All rights reserved.
 * "signature": "ԝᏂꓜGбƧÐ𝟫1īƴЕ4ꓴꜱҮ𝟩6ɊɌʌꓝᏂʌƋꓴnȜᎻɯ𝟩hZЗ𝟣fꓗǝꓳМ𝟦Υν𝟥৭ᗞµĵԛhƬⲦx𝟫Oŧ𝐴ոʈꓰցXOnꙄL𝟣ǝƘⲔᎬᑕոոꓚLƘМ𝘈ɋ5Ү𝟚ŧꓬhοƶꓴ3AӠɗᎻΜƎȷᏮ9ᎠďАƶᏮΕ6ƨᑕo",
 * "signdate": "2025-12-02T06:27:48.226Z",
 * 
 * VERSION: 2025.11.03.001 - Remote Tool Provider Demo (JavaScript/Node.js)
 * 
 * BUILD/RUN INSTRUCTIONS:
 *   No build required - Node.js is interpreted
 *   
 *   Requirements:
 *     - Node.js 14+ (tested with Node.js 18+)
 *     - Standard library only (no npm install needed)
 *   
 *   Run:
 *     node reverse_mcp.js [--background]
 *     node reverse_mcp.js --help
 * 
 * HOW TO USE THIS CODE:
 * ---------------------
 * This code is a complete, self-contained reference template for integrating MCP (Model Context Protocol)
 * tool support into other applications like Fusion 360, Blender, Ghidra, and similar products.
 * 
 * HOW THIS WORKS:
 * ---------------
 * 1. You create a new add-on or extension or plugin or similar for the application you want to let an AI 
 *    control on your behalf. (hereafter addIn)
 * 2. This template gives your new addIn the facility to discover the correct endpoint where a local 
 *    controller MCP server is running, and then:
 * 3. lets your addIn register itself with that server as a new tool, which any AI using that MCP server 
 *    can then discover and access and use.
 * 4. and finally, this template processes incoming tool requests from the AI, which you implement in your 
 *    addIn, and this template sends the results of those tool-calls back to the AI.
 * 5. BONUS: Your addIn can also CALL OTHER MCP tools on the server (sqlite, browser, user, etc.) - making 
 *    it possible to orchestrate complex workflows!
 * *. The server installer can be found at https://github.com/aurafriday/mcp-link-server/releases
 * 
 * ARCHITECTURE OVERVIEW:
 * ----------------------
 * 1. Native Messaging Discovery: Locates the MCP server by finding the Chrome native messaging manifest
 *    (com.aurafriday.shim.json) which is installed by the Aura Friday MCP-Link server.
 * 
 * 2. Server Configuration: Executes the native messaging binary to get the server's SSE endpoint URL
 *    and authentication token. The binary is a long-running stdio service, so we terminate it after
 *    reading the initial JSON config.
 * 
 * 3. SSE Connection: Establishes a persistent Server-Sent Events (SSE) connection to receive messages
 *    from the server. This uses EventEmitter to route incoming messages to the appropriate handlers.
 * 
 * 4. Dual-Channel Communication:
 *    - POST requests (via HTTP/HTTPS) to send JSON-RPC commands to the server
 *    - SSE stream (long-lived GET connection) to receive JSON-RPC responses and reverse tool calls
 * 
 * 5. Tool Registration: Uses the server's "remote" tool to register your custom tool with these components:
 *    - tool_name: Unique identifier for your tool
 *    - readme: Minimal summary for the AI (when to use this tool)
 *    - description: Comprehensive documentation for the AI (what it does, how to use it, examples)
 *    - parameters: JSON Schema defining the tool's input parameters
 *    - callback_endpoint: Identifier for routing reverse calls back to your client
 *    - TOOL_API_KEY: Authentication key for your tool
 * 
 * 6. Reverse Call Handling: After registration, your tool appears in the server's tool list. When an
 *    AI agent calls your tool, the server sends a "reverse" message via the SSE stream containing:
 *    - tool: Your tool's name
 *    - call_id: Unique ID for this invocation (used to send the reply)
 *    - input: The parameters passed by the AI
 * 
 * 7. Reply Mechanism: Your code processes the request and sends a "tools/reply" message back to the
 *    server with the call_id and result. The server forwards this to the AI.
 * 
 * INTEGRATION STEPS:
 * ------------------
 * 1. Copy this file to your project
 * 2. Modify the tool registration section (search for "demo_tool_nodejs"):
 *    - Change tool_name to your tool's unique identifier
 *    - Update description and readme to explain your tool's purpose
 *    - Define your tool's parameters schema
 *    - Set a unique callback_endpoint and TOOL_API_KEY
 * 
 * 3. Replace the handleEchoRequest() function with your tool's actual logic:
 *    - Extract parameters from the input_data
 *    - Perform your tool's operations (file I/O, API calls, computations, etc.)
 *    - OPTIONALLY: Call other MCP tools using callMcpTool() function
 *    - Return a result object with "content" array and "isError" boolean
 * 
 * 4. (Optional) Use callMcpTool() to orchestrate other MCP tools:
 *    - Your handler receives sseConnection parameter
 *    - Use callMcpTool() to call sqlite, browser, user, or any other MCP tool
 *    - Example: const result = await callMcpTool(sseConnection, "sqlite", 
 *                                                {"input": {"sql": ".tables", "tool_unlock_token": "..."}})
 *    - This enables complex workflows like: read data from app → query database → show results to user
 * 
 * 5. Run your tool provider script:
 *    - It will auto-discover the server, register your tool, and listen for calls
 *    - The tool remains registered as long as the script is running
 *    - Press Ctrl+C to cleanly shut down
 * 
 * RESULT FORMAT:
 * --------------
 * All tool results must follow this structure:
 * {
 *   "content": [
 *     {"type": "text", "text": "Your response text here"},
 *     {"type": "image", "data": "base64...", "mimeType": "image/png"}  // optional
 *   ],
 *   "isError": false  // or true if an error occurred
 * }
 * 
 * ASYNC/EVENT MODEL:
 * ------------------
 * - Main async flow: Handles tool registration and awaits reverse calls via event emitter
 * - SSE stream reader: Continuously reads the SSE stream and emits events for messages
 * - EventEmitter pattern: Uses 'reverse' events for tool calls and pending response maps for RPC
 * 
 * DEPENDENCIES:
 * -------------
 * Node.js 14+ with standard library only (no npm install required):
 * - fs, path, os: File system and OS operations
 * - https, http: Network communication
 * - child_process: Execute native messaging binary
 * - events: EventEmitter for message routing
 * 
 * ERROR HANDLING & RECONNECTION:
 * -------------------------------
 * - SSL certificate verification is disabled (self-signed certs are common in local servers)
 * - Native binary timeout is 5 seconds (increase if needed)
 * - SSE response timeout is 10 seconds per request (configurable)
 * - All errors are logged to stderr for debugging
 * - Automatic reconnection with exponential backoff if SSE connection drops:
 *   * Retry delays: 2s, 4s, 8s, 16s, 32s, 60s (max), 60s, 60s...
 *   * After successful reconnection, retry counter resets
 *   * Tool is automatically re-registered after reconnection
 *   * Retries forever until manually stopped (Ctrl+C)
 */

const DOCO = `
This script demonstrates how to register a tool with the MCP server using the remote tool system.
It acts as a tool provider that:
1. Connects to the MCP server via native messaging discovery
2. Registers a "demo_tool_nodejs" with the server
3. Listens for reverse tool calls from the server
4. Processes "echo" requests and sends back replies
5. Demonstrates calling OTHER MCP tools (sqlite, browser, etc.) from within the handler
6. Runs continuously until stopped with Ctrl+C

The demo tool responds to these message patterns:
- "list databases" or "list db" - Calls sqlite to list all databases (START HERE to discover what's available)
- "list tables" - Calls sqlite to list tables in :memory: database
- "list tables in <database>" - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")
- Any other message - Simple echo response

Usage: node reverse_mcp.js [--background]
`;

const fs = require('fs');
const path = require('path');
const os = require('os');
const https = require('https');
const http = require('http');
const { spawn } = require('child_process');
const { EventEmitter } = require('events');

// Find native messaging manifest for this platform
function findNativeMessagingManifest() {
  const platform = os.platform();
  const home = os.homedir();
  let possiblePaths = [];
  
  if (platform === 'win32') {
    const localAppData = process.env.LOCALAPPDATA || path.join(home, 'AppData', 'Local');
    possiblePaths.push(path.join(localAppData, 'AuraFriday', 'com.aurafriday.shim.json'));
  } else if (platform === 'darwin') {
    possiblePaths.push(
      path.join(home, 'Library/Application Support/Google/Chrome/NativeMessagingHosts/com.aurafriday.shim.json'),
      path.join(home, 'Library/Application Support/Chromium/NativeMessagingHosts/com.aurafriday.shim.json'),
      path.join(home, 'Library/Application Support/Microsoft Edge/NativeMessagingHosts/com.aurafriday.shim.json'),
      path.join(home, 'Library/Application Support/BraveSoftware/Brave-Browser/NativeMessagingHosts/com.aurafriday.shim.json')
    );
  } else {
    possiblePaths.push(
      path.join(home, '.config/google-chrome/NativeMessagingHosts/com.aurafriday.shim.json'),
      path.join(home, '.config/chromium/NativeMessagingHosts/com.aurafriday.shim.json'),
      path.join(home, '.config/microsoft-edge/NativeMessagingHosts/com.aurafriday.shim.json')
    );
  }
  
  for (const p of possiblePaths) {
    if (fs.existsSync(p)) {
      return p;
    }
  }
  
  return null;
}

// Read and parse manifest
function readNativeMessagingManifest(manifestPath) {
  try {
    const content = fs.readFileSync(manifestPath, 'utf8');
    return JSON.parse(content);
  } catch (e) {
    console.error(`Error reading manifest: ${e.message}`);
    return null;
  }
}

// Discover MCP server endpoint by running native binary
async function discoverMcpServerEndpoint(manifest) {
  const binaryPath = manifest.path;
  
  if (!binaryPath) {
    console.error('ERROR: No "path" in manifest');
    return null;
  }
  
  if (!fs.existsSync(binaryPath)) {
    console.error(`ERROR: Native binary not found: ${binaryPath}`);
    return null;
  }
  
  console.error(`Running native binary: ${binaryPath}`);
  console.error('[DEBUG] Native messaging protocol uses 4-byte length prefix (little-endian uint32)');
  
  return new Promise((resolve) => {
    const proc = spawn(binaryPath, [], {
      stdio: ['pipe', 'pipe', 'pipe']
    });
    
    let accumulated = Buffer.alloc(0);
    let messageLength = null;
    let jsonData = null;
    
    const timeout = setTimeout(() => {
      proc.kill();
      console.error('ERROR: Native binary timed out');
      resolve(null);
    }, 5000);
    
    proc.stdout.on('data', (chunk) => {
      accumulated = Buffer.concat([accumulated, chunk]);
      
      // Step 1: Read the 4-byte length prefix (little-endian uint32)
      if (messageLength === null && accumulated.length >= 4) {
        messageLength = accumulated.readUInt32LE(0);
        console.error(`[DEBUG] Message length from native binary: ${messageLength} bytes`);
        
        if (messageLength <= 0 || messageLength > 10000000) {
          console.error(`ERROR: Invalid message length: ${messageLength}`);
          clearTimeout(timeout);
          proc.kill();
          resolve(null);
          return;
        }
      }
      
      // Step 2: Read the JSON payload of the specified length
      if (messageLength !== null && accumulated.length >= 4 + messageLength) {
        const jsonBytes = accumulated.slice(4, 4 + messageLength);
        const text = jsonBytes.toString('utf8');
        console.error(`[DEBUG] Successfully read ${jsonBytes.length} bytes of JSON`);
        console.error(`[DEBUG] JSON preview: ${text.substring(0, 100)}...`);
        
        try {
          jsonData = JSON.parse(text);
          clearTimeout(timeout);
          proc.kill();
          resolve(jsonData);
        } catch (e) {
          console.error(`ERROR: Failed to parse JSON: ${e.message}`);
          console.error(`Output was: ${text.substring(0, 200)}`);
          clearTimeout(timeout);
          proc.kill();
          resolve(null);
        }
      }
    });
    
    proc.on('error', (err) => {
      clearTimeout(timeout);
      console.error(`ERROR: Failed to run native binary: ${err.message}`);
      resolve(null);
    });
  });
}

// Extract server URL from config
function extractServerUrl(configJson) {
  try {
    const mcpServers = configJson.mcpServers || {};
    const firstServer = Object.values(mcpServers)[0];
    return firstServer ? firstServer.url : null;
  } catch (e) {
    console.error(`ERROR: Failed to extract URL from config: ${e.message}`);
    return null;
  }
}

// SSE Connection Manager
class SSEConnection extends EventEmitter {
  constructor(serverUrl, authHeader) {
    super();
    this.serverUrl = serverUrl;
    this.authHeader = authHeader;
    this.sessionId = null;
    this.messageEndpoint = null;
    this.connected = false;
    this.isAlive = true;
    this.pendingResponses = new Map();
  }
  
  async connect() {
    const url = new URL(this.serverUrl);
    const isHttps = url.protocol === 'https:';
    const client = isHttps ? https : http;
    
    return new Promise((resolve, reject) => {
      const options = {
        hostname: url.hostname,
        port: url.port || (isHttps ? 443 : 80),
        path: url.pathname + url.search,
        method: 'GET',
        headers: {
          'Accept': 'text/event-stream',
          'Cache-Control': 'no-cache',
          'Authorization': this.authHeader
        },
        rejectUnauthorized: false  // Accept self-signed certs
      };
      
      const req = client.request(options, (res) => {
        if (res.statusCode !== 200) {
          reject(new Error(`HTTP ${res.statusCode}`));
          return;
        }
        
        this.connected = true;
        
        let buffer = '';
        let eventType = null;
        
        res.on('data', (chunk) => {
          buffer += chunk.toString();
          const lines = buffer.split('\n');
          buffer = lines.pop() || '';
          
          for (const line of lines) {
            const trimmed = line.replace(/\r$/, '');
            
            if (trimmed === '') {
              // Empty line - dispatch event
              eventType = null;
              continue;
            }
            
            if (trimmed.startsWith(':')) {
              // Comment/ping
              continue;
            }
            
            const colonIndex = trimmed.indexOf(':');
            if (colonIndex === -1) continue;
            
            const field = trimmed.substring(0, colonIndex);
            let value = trimmed.substring(colonIndex + 1);
            if (value.startsWith(' ')) value = value.substring(1);
            
            if (field === 'event') {
              eventType = value;
            } else if (field === 'data') {
              if (eventType === 'endpoint') {
                this.messageEndpoint = value;
                const match = value.match(/session_id=([^&]+)/);
                if (match) {
                  this.sessionId = match[1];
                  resolve(true);
                }
              } else {
                // Regular message
                try {
                  const msg = JSON.parse(value);
                  this.handleMessage(msg);
                } catch (e) {
                  // Not JSON, ignore
                }
              }
            }
          }
        });
        
        res.on('end', () => {
          this.connected = false;
          this.isAlive = false;
          this.emit('disconnected');
        });
        
        res.on('error', (err) => {
          this.connected = false;
          this.isAlive = false;
          this.emit('error', err);
        });
      });
      
      req.on('error', reject);
      req.end();
    });
  }
  
  handleMessage(msg) {
    if (msg.reverse) {
      // Reverse tool call
      this.emit('reverse', msg);
    } else if (msg.id && this.pendingResponses.has(msg.id)) {
      // Response to our request
      const resolver = this.pendingResponses.get(msg.id);
      this.pendingResponses.delete(msg.id);
      resolver(msg);
    }
  }
  
  async sendRequest(method, params) {
    const requestId = require('crypto').randomUUID();
    const url = new URL(this.messageEndpoint, this.serverUrl);
    const isHttps = url.protocol === 'https:';
    const client = isHttps ? https : http;
    
    const payload = {
      jsonrpc: '2.0',
      id: requestId,
      method,
      params
    };
    
    const body = JSON.stringify(payload);
    
    return new Promise((resolve, reject) => {
      // Register response handler
      this.pendingResponses.set(requestId, resolve);
      
      const timeout = setTimeout(() => {
        this.pendingResponses.delete(requestId);
        reject(new Error(`Timeout waiting for response to ${method}`));
      }, 10000);
      
      const options = {
        hostname: url.hostname,
        port: url.port || (isHttps ? 443 : 80),
        path: url.pathname + url.search,
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Content-Length': Buffer.byteLength(body),
          'Authorization': this.authHeader
        },
        rejectUnauthorized: false
      };
      
      const req = client.request(options, (res) => {
        if (res.statusCode !== 202) {
          clearTimeout(timeout);
          this.pendingResponses.delete(requestId);
          reject(new Error(`POST failed with status ${res.statusCode}`));
        }
        // Response comes via SSE, not here
      });
      
      req.on('error', (err) => {
        clearTimeout(timeout);
        this.pendingResponses.delete(requestId);
        reject(err);
      });
      
      req.write(body);
      req.end();
      
      // Override timeout clear when response arrives
      this.pendingResponses.set(requestId, (response) => {
        clearTimeout(timeout);
        resolve(response);
      });
    });
  }
  
  async sendToolReply(callId, result) {
    const url = new URL(this.messageEndpoint, this.serverUrl);
    const isHttps = url.protocol === 'https:';
    const client = isHttps ? https : http;
    
    const payload = {
      jsonrpc: '2.0',
      id: callId,
      method: 'tools/reply',
      params: { result }
    };
    
    const body = JSON.stringify(payload);
    
    return new Promise((resolve, reject) => {
      const options = {
        hostname: url.hostname,
        port: url.port || (isHttps ? 443 : 80),
        path: url.pathname + url.search,
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Content-Length': Buffer.byteLength(body),
          'Authorization': this.authHeader
        },
        rejectUnauthorized: false
      };
      
      const req = client.request(options, (res) => {
        if (res.statusCode === 202) {
          console.error(`[OK] Sent tools/reply for call_id ${callId}`);
          resolve(true);
        } else {
          console.error(`ERROR: tools/reply POST failed with status ${res.statusCode}`);
          resolve(false);
        }
      });
      
      req.on('error', (err) => {
        console.error(`ERROR: Failed to send tools/reply: ${err.message}`);
        resolve(false);
      });
      
      req.write(body);
      req.end();
    });
  }
}

/**
 * Call another MCP tool on the server.
 * 
 * This function demonstrates how to call other MCP tools from within your remote tool handler.
 * It uses the existing SSE connection and JSON-RPC infrastructure to make tool calls.
 * 
 * @param {SSEConnection} sseConnection - Active SSE connection
 * @param {string} toolName - Name of the tool to call (e.g., "sqlite", "browser", "user")
 * @param {object} arguments_ - Arguments to pass to the tool
 * @param {number} timeoutSeconds - Timeout in seconds (default: 30)
 * @returns {Promise<object|null>} JSON-RPC response, or null on error
 * 
 * @example
 * // Call sqlite tool to list tables
 * const result = await callMcpTool(
 *   sseConnection,
 *   "sqlite",
 *   {"input": {"sql": ".tables", "tool_unlock_token": "29e63eb5"}},
 *   30
 * );
 * 
 * @example
 * // Call browser tool to list tabs
 * const result = await callMcpTool(
 *   sseConnection,
 *   "browser",
 *   {"input": {"operation": "list_tabs", "tool_unlock_token": "e5076d"}}
 * );
 */
async function callMcpTool(sseConnection, toolName, arguments_, timeoutSeconds = 30) {
  const toolCallParams = {
    name: toolName,
    arguments: arguments_
  };
  
  // Use the existing sendRequest method with custom timeout
  const requestId = require('crypto').randomUUID();
  const url = new URL(sseConnection.messageEndpoint, sseConnection.serverUrl);
  const isHttps = url.protocol === 'https:';
  const client = isHttps ? https : http;
  
  const payload = {
    jsonrpc: '2.0',
    id: requestId,
    method: 'tools/call',
    params: toolCallParams
  };
  
  const body = JSON.stringify(payload);
  
  return new Promise((resolve, reject) => {
    // Register response handler
    sseConnection.pendingResponses.set(requestId, resolve);
    
    const timeout = setTimeout(() => {
      sseConnection.pendingResponses.delete(requestId);
      reject(new Error(`Timeout waiting for response to tools/call (${toolName})`));
    }, timeoutSeconds * 1000);
    
    const options = {
      hostname: url.hostname,
      port: url.port || (isHttps ? 443 : 80),
      path: url.pathname + url.search,
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(body),
        'Authorization': sseConnection.authHeader
      },
      rejectUnauthorized: false
    };
    
    const req = client.request(options, (res) => {
      if (res.statusCode !== 202) {
        clearTimeout(timeout);
        sseConnection.pendingResponses.delete(requestId);
        reject(new Error(`POST failed with status ${res.statusCode}`));
      }
      // Response comes via SSE, not here
    });
    
    req.on('error', (err) => {
      clearTimeout(timeout);
      sseConnection.pendingResponses.delete(requestId);
      reject(err);
    });
    
    req.write(body);
    req.end();
    
    // Override timeout clear when response arrives
    sseConnection.pendingResponses.set(requestId, (response) => {
      clearTimeout(timeout);
      resolve(response);
    });
  });
}

// Register demo_tool_nodejs
async function registerDemoTool(sseConnection) {
  console.error('Registering demo_tool_nodejs with MCP server...');
  
  // Get the source file location
  const sourceFile = __filename;
  
  const registrationParams = {
    name: 'remote',
    arguments: {
      input: {
        operation: 'register',
        tool_name: 'demo_tool_nodejs',
        readme: 'Demo tool that echoes messages back and can call other MCP tools.\n- Use this to test the remote tool system and verify bidirectional communication.\n- Demonstrates how remote tools can call OTHER tools on the server (like sqlite, browser, etc.)', // MINIMAL: Tell the AI ONLY when to use this tool
        description: `Demo tool (Node.js implementation) for testing remote tool registration and end-to-end MCP communication. This tool demonstrates TWO key capabilities: (1) Basic echo functionality - echoes back any message sent to it, and (2) Tool-to-tool communication - shows how remote tools can call OTHER MCP tools on the server. This verifies that: (a) tool registration works correctly, (b) reverse calls from server to client function properly, (c) the client can successfully reply to tool calls, (d) the full bidirectional JSON-RPC communication channel is operational, and (e) remote tools can orchestrate other tools. This tool is implemented in ${sourceFile} and serves as a reference template for integrating MCP tool support into other applications like Fusion 360, Blender, Ghidra, and similar products. Usage workflow: (1) Start by discovering databases: {"message": "list databases"} calls sqlite to show all available databases. (2) Then list tables in a specific database: {"message": "list tables in test.db"} calls sqlite and returns table names. (3) Basic echo: {"message": "test"} returns 'Echo: test'. The tool automatically detects keywords in the message to trigger different demonstrations.`, // COMPREHENSIVE: Tell the AI everything it needs to know to use this tool
        parameters: {
          type: 'object',
          properties: {
            message: {
              type: 'string',
              description: 'The message to echo back'
            }
          },
          required: ['message']
        },
        callback_endpoint: 'nodejs-client://demo-tool-callback',
        TOOL_API_KEY: 'nodejs_demo_tool_auth_key_12345'
      }
    }
  };
  
  const response = await sseConnection.sendRequest('tools/call', registrationParams);
  
  if (response && response.result) {
    const result = response.result;
    if (result.content && result.content[0]) {
      const text = result.content[0].text;
      if (text.includes('Successfully registered tool')) {
        console.error(`[OK] ${text}`);
        return true;
      }
    }
  }
  
  console.error('ERROR: Unexpected registration response');
  return false;
}

/**
 * Handle an echo request from the server.
 * 
 * This demonstrates TWO capabilities:
 * 1. Basic echo functionality - echoes back the message
 * 2. Calling OTHER MCP tools - demonstrates how to call sqlite, browser, etc.
 * 
 * @param {object} callData - The tool call data from the reverse message
 * @param {SSEConnection} sseConnection - Optional SSE connection for making tool calls
 * @returns {Promise<object>} Result object to send back
 * 
 * @example
 * // Basic echo
 * {"message": "Hello World"}
 * 
 * // Step 1: Discover what databases exist
 * {"message": "list databases"}
 * 
 * // Step 2: List tables in a specific database
 * {"message": "list tables in test.db"}
 * 
 * // Or use default :memory: database
 * {"message": "list tables"}
 */
async function handleEchoRequest(callData, sseConnection = null) {
  const arguments_ = callData.params?.arguments || {};
  const message = arguments_.message || '(no message provided)';
  
  console.error(`[ECHO] Received echo request: ${message}`);
  
  // Basic echo response
  let responseText = `Echo: ${message}`;
  
  // DEMONSTRATION: If we have connection info, show how to call other tools
  if (sseConnection) {
    const messageLower = message.toLowerCase();
    
    try {
      // Demo 1: List databases (triggered by keyword "databases" or "db")
      // Check this FIRST because it's more specific and helps users discover what databases exist
      if (messageLower.includes('databases') || messageLower.includes('list db')) {
        console.error('[DEMO] Calling sqlite tool to list databases...');
        
        // Call the sqlite tool to list databases
        const sqliteResult = await callMcpTool(
          sseConnection,
          'sqlite',
          {"input": {"sql": ".databases", "tool_unlock_token": "29e63eb5"}}
        );
        
        // Append the result to our response
        if (sqliteResult && sqliteResult.result) {
          responseText += '\n\n[DEMO] Called sqlite tool successfully!\n';
          responseText += `Result:\n${JSON.stringify(sqliteResult.result, null, 2)}`;
        } else {
          responseText += '\n\n[DEMO] SQLite tool call failed or returned no result:\n';
          responseText += JSON.stringify(sqliteResult, null, 2);
        }
      }
      // Demo 2: List tables (triggered by keywords "tables" - check AFTER databases to avoid conflicts)
      else if (messageLower.includes('tables')) {
        console.error('[DEMO] Calling sqlite tool to list tables...');
        
        // Extract database name if specified (e.g., "list tables in test.db")
        let database = ':memory:';
        if (messageLower.includes(' in ')) {
          const parts = message.split(' in ');
          if (parts.length > 1) {
            database = parts[1].trim();
          }
        }
        
        // Call the sqlite tool to list tables
        const sqliteResult = await callMcpTool(
          sseConnection,
          'sqlite',
          {"input": {"sql": ".tables", "database": database, "tool_unlock_token": "29e63eb5"}}
        );
        
        // Append the result to our response
        if (sqliteResult && sqliteResult.result) {
          responseText += '\n\n[DEMO] Called sqlite tool successfully!\n';
          responseText += `Database: ${database}\n`;
          responseText += `Result:\n${JSON.stringify(sqliteResult.result, null, 2)}`;
        } else {
          responseText += '\n\n[DEMO] SQLite tool call failed or returned no result:\n';
          responseText += JSON.stringify(sqliteResult, null, 2);
        }
      }
    } catch (error) {
      responseText += `\n\n[ERROR] Tool call failed: ${error.message}`;
      console.error(`[ERROR] Tool call failed: ${error.message}`);
    }
  }
  
  return {
    content: [{
      type: 'text',
      text: responseText
    }],
    isError: false
  };
}

// Main worker
async function mainWorker() {
  console.error('=== Aura Friday Remote Tool Provider Demo ===');
  console.error(`PID: ${process.pid}`);
  console.error('Registering demo_tool with MCP server\n');
  
  // Connection state for reconnection logic
  let retryCount = 0;
  const maxRetryDelay = 60; // Max 1 minute between retries
  
  // Outer reconnection loop - keeps trying forever
  while (true) {
    try {
      // Calculate retry delay with exponential backoff
      if (retryCount > 0) {
        const delay = Math.min(Math.pow(2, retryCount), maxRetryDelay);
        console.error(`\n[RECONNECT] Waiting ${delay} seconds before retry (attempt #${retryCount})...`);
        await new Promise(resolve => setTimeout(resolve, delay * 1000));
        console.error('[RECONNECT] Attempting to reconnect...\n');
      }
      
      // Step 1: Find manifest
      console.error('Step 1: Finding native messaging manifest...');
      const manifestPath = findNativeMessagingManifest();
      
      if (!manifestPath) {
        console.error('ERROR: Could not find native messaging manifest');
        retryCount++;
        continue;
      }
      
      console.error(`[OK] Found manifest: ${manifestPath}\n`);
      
      // Step 2: Read manifest
      console.error('Step 2: Reading manifest...');
      const manifest = readNativeMessagingManifest(manifestPath);
      
      if (!manifest) {
        console.error('ERROR: Could not read manifest');
        retryCount++;
        continue;
      }
      
      console.error('[OK] Manifest loaded\n');
      
      // Step 3: Discover server endpoint
      console.error('Step 3: Discovering MCP server endpoint...');
      const configJson = await discoverMcpServerEndpoint(manifest);
      
      if (!configJson) {
        console.error('ERROR: Could not get configuration from native binary');
        console.error('Is the Aura Friday MCP server running?');
        retryCount++;
        continue;
      }
      
      const serverUrl = extractServerUrl(configJson);
      
      if (!serverUrl) {
        console.error('ERROR: Could not extract server URL from configuration');
        retryCount++;
        continue;
      }
      
      console.error(`[OK] Found server at: ${serverUrl}\n`);
      
      // Step 4: Extract auth header
      const mcpServers = configJson.mcpServers || {};
      const firstServer = Object.values(mcpServers)[0];
      const authHeader = firstServer?.headers?.Authorization;
      
      if (!authHeader) {
        console.error('ERROR: No authorization header found in configuration');
        retryCount++;
        continue;
      }
      
      // Step 5: Connect to SSE
      console.error('Step 4: Connecting to SSE endpoint...');
      const sseConnection = new SSEConnection(serverUrl, authHeader);
      
      try {
        await sseConnection.connect();
      } catch (e) {
        console.error(`ERROR: Could not connect to SSE endpoint: ${e.message}`);
        retryCount++;
        continue;
      }
      
      console.error(`[OK] Connected! Session ID: ${sseConnection.sessionId}\n`);
      
      // Step 6: Check for remote tool
      console.error('Step 5: Checking for remote tool...');
      const toolsResponse = await sseConnection.sendRequest('tools/list', {});
      
      if (!toolsResponse || !toolsResponse.result) {
        console.error('ERROR: Could not get tools list');
        retryCount++;
        continue;
      }
      
      const tools = toolsResponse.result.tools || [];
      const hasRemote = tools.some(t => t.name === 'remote');
      
      if (!hasRemote) {
        console.error('ERROR: Server does not have "remote" tool - cannot register demo_tool');
        retryCount++;
        continue;
      }
      
      console.error('[OK] Remote tool found\n');
      
      // Step 7: Register demo_tool_nodejs
      console.error('Step 6: Registering demo_tool_nodejs...');
      if (!await registerDemoTool(sseConnection)) {
        console.error('ERROR: Failed to register demo_tool_nodejs');
        retryCount++;
        continue;
      }
      
      // Reset retry count after successful connection and registration
      retryCount = 0;
      
      console.error('\n' + '='.repeat(60));
      console.error('[OK] demo_tool_nodejs registered successfully!');
      console.error('Listening for reverse tool calls... (Press Ctrl+C to stop)');
      console.error('='.repeat(60) + '\n');
      
      // Step 8: Listen for reverse calls
      sseConnection.on('reverse', async (msg) => {
        const reverseData = msg.reverse;
        const toolName = reverseData.tool;
        const callId = reverseData.call_id;
        const inputData = reverseData.input;
        
        console.error('\n[CALL] Reverse call received:');
        console.error(`       Tool: ${toolName}`);
        console.error(`       Call ID: ${callId}`);
        console.error(`       Input: ${JSON.stringify(inputData, null, 2)}`);
        
        if (toolName === 'demo_tool_nodejs') {
          // Handle the echo request (pass connection info so it can call other tools)
          const result = await handleEchoRequest(inputData, sseConnection);
          await sseConnection.sendToolReply(callId, result);
        } else {
          console.error(`[WARN] Unknown tool: ${toolName}`);
        }
      });
      
      // Monitor connection health
      const healthCheck = setInterval(() => {
        if (!sseConnection.isAlive) {
          console.error('\n[WARN] SSE connection lost - reconnecting...');
          clearInterval(healthCheck);
          retryCount = 1; // Start with first retry delay
          // Break inner loop to trigger reconnection
          process.emit('internal-reconnect');
        }
      }, 1000);
      
      // Keep alive until Ctrl+C or reconnection needed
      await new Promise((resolve) => {
        const sigintHandler = () => {
          clearInterval(healthCheck);
          console.error('\n\n' + '='.repeat(60));
          console.error('Shutting down...');
          console.error('='.repeat(60));
          process.exit(0);
        };
        
        const reconnectHandler = () => {
          clearInterval(healthCheck);
          process.removeListener('SIGINT', sigintHandler);
          process.removeListener('internal-reconnect', reconnectHandler);
          resolve(); // Break this iteration, outer loop will retry
        };
        
        process.once('SIGINT', sigintHandler);
        process.once('internal-reconnect', reconnectHandler);
      });
      
      // If we get here, connection dropped - outer loop will retry
      
    } catch (error) {
      console.error(`\n[ERROR] Unexpected error in main loop: ${error.message}`);
      console.error(error.stack);
      retryCount++;
      // Loop continues to retry
    }
  }
}

// Main entry point
async function main() {
  const args = process.argv.slice(2);
  const background = args.includes('--background');
  
  if (args.includes('--help') || args.includes('-h')) {
    console.log('Usage: node reverse_mcp.js [--background]');
    console.log('');
    console.log('Aura Friday Remote Tool Provider - Registers demo_tool_nodejs with MCP server');
    console.log('');
    console.log(DOCO);
    return 0;
  }
  
  if (background) {
    console.error(`Starting in background mode (PID: ${process.pid})...`);
    // In Node.js, we just run normally - the process can be backgrounded by the shell
    console.error(`[OK] Background worker started (PID: ${process.pid})`);
    console.error(`  Use 'kill ${process.pid}' to stop`);
  }
  
  return await mainWorker();
}

// Run if called directly
if (require.main === module) {
  main().then(code => process.exit(code || 0));
}

module.exports = { main, mainWorker };

