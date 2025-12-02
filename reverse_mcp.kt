#!/usr/bin/env kotlin
/*
 * File: reverse_mcp.kt
 * Project: Aura Friday MCP-Link Server - Remote Tool Provider Demo
 * Component: Registers a demo tool with the MCP server and handles reverse calls
 * Author: Christopher Nathan Drake (cnd)
 * Created: 2025-11-19
 * Last Modified: 2025-11-19 by cnd (Kotlin port from Java)
 * SPDX-License-Identifier: Apache-2.0
 * Copyright: (c) 2025 Christopher Nathan Drake. All rights reserved.
 * 
 * VERSION: 2025.11.19.001 - Remote Tool Provider Demo (Kotlin)
 * 
 * BUILD/RUN INSTRUCTIONS:
 *   
 *   Requirements:
 *     - Kotlin 1.9+ (tested with Kotlin 2.0+)
 *     - JDK 11+ for compilation (runtime JVM-based version)
 *     - OR Kotlin/Native for standalone executable (no JVM required)
 *   
 *   Compile (JVM version - requires Java runtime):
 *     kotlinc reverse_mcp.kt -include-runtime -d reverse_mcp.jar
 *   
 *   Run (JVM version):
 *     java -jar reverse_mcp.jar [--background]
 *     java -jar reverse_mcp.jar --help
 *   
 *   Compile (Native standalone executable - NO Java runtime required):
 *     Windows: kotlinc-native reverse_mcp.kt -o reverse_mcp_kt.exe
 *     Mac/Linux: kotlinc-native reverse_mcp.kt -o reverse_mcp_kt
 *   
 *   Note: Kotlin/Native produces true standalone executables similar to Go/Rust
 *   
 *   Run (Native version):
 *     Windows: reverse_mcp_kt.exe [--background]
 *     Mac/Linux: ./reverse_mcp_kt [--background]
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
 *    from the server. This runs in a background thread and routes incoming messages to the appropriate
 *    handlers.
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
 * 2. Modify the tool registration section (search for "demo_tool_kotlin"):
 *    - Change tool_name to your tool's unique identifier
 *    - Update description and readme to explain your tool's purpose
 *    - Define your tool's parameters schema
 *    - Set a unique callback_endpoint and TOOL_API_KEY
 * 
 * 3. Replace the handleEchoRequest() function with your tool's actual logic:
 *    - Extract parameters from the input_data
 *    - Perform your tool's operations (file I/O, API calls, computations, etc.)
 *    - OPTIONALLY: Call other MCP tools using callMcpTool() function
 *    - Return a result map with "content" array and "isError" boolean
 * 
 * 4. (Optional) Use callMcpTool() to orchestrate other MCP tools:
 *    - Your handler receives SSEConnection parameter with connection info
 *    - Use callMcpTool() to call sqlite, browser, user, or any other MCP tool
 *    - Example: val result = callMcpTool(conn, "sqlite", 
 *                  mapOf("input" to mapOf("sql" to ".tables", "tool_unlock_token" to "29e63eb5")))
 *    - This enables complex workflows like: read data from app → query database → show results to user
 * 
 * 5. Compile and run your tool provider:
 *    - It will auto-discover the server, register your tool, and listen for calls
 *    - The tool remains registered as long as the process is running
 *    - Press Ctrl+C to cleanly shut down
 * 
 * RESULT FORMAT:
 * --------------
 * All tool results must follow this JSON structure:
 * {
 *   "content": [
 *     {"type": "text", "text": "Your response text here"},
 *     {"type": "image", "data": "base64...", "mimeType": "image/png"}  // optional
 *   ],
 *   "isError": false  // or true if an error occurred
 * }
 * 
 * THREADING MODEL:
 * ----------------
 * - Main thread: Handles tool registration and processes reverse calls from blocking queue
 * - SSE reader thread: Continuously reads the SSE stream and routes messages to queues
 * - Thread-safe collections: ConcurrentHashMap and BlockingQueue for message passing
 * 
 * DEPENDENCIES:
 * -------------
 * Kotlin stdlib only - no external dependencies required
 * Uses Java standard library for networking (java.net, javax.net.ssl)
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
 * 
 * "signature": "Ꙅ𐓒ϜСΕ𝟟YıƋᎪzКȷ3oCᗷƨοЕWƤꓚƟĵXĵҮᴍQrSⲦᒿnΝHȠᴠƙɅƵƌcՕꓳꓚ08ƤgɋϜꓴτƖMᎬΝb𝟚ΑᑕQҳƨɅοꓚᏮϜŧЕ𐐕Мī𝟤ꓧΕZꓑꓦjꓐCΤxРрoƼꓜᴛꓜꓜµɗȢ𝟟ᴍ7𝟩qƵlᎻꓓ𝟦Ⅾ",
 * "signdate": "2025-12-02T06:27:48.892Z",
 */

import java.io.*
import java.net.*
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths
import java.util.*
import java.util.concurrent.*
import javax.net.ssl.*
import java.security.cert.X509Certificate
import kotlin.system.exitProcess

const val DOCO = """
This script demonstrates how to register a tool with the MCP server using the remote tool system.
It acts as a tool provider that:
1. Connects to the MCP server via native messaging discovery
2. Registers a "demo_tool_kotlin" with the server
3. Listens for reverse tool calls from the server
4. Processes "echo" requests and sends back replies
5. Demonstrates calling OTHER MCP tools (sqlite, browser, etc.) from within the handler
6. Runs continuously until stopped with Ctrl+C

The demo tool responds to these message patterns:
- "list databases" or "list db" - Calls sqlite to list all databases (START HERE to discover what's available)
- "list tables" - Calls sqlite to list tables in :memory: database
- "list tables in <database>" - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")
- Any other message - Simple echo response

Usage: kotlin reverse_mcp.kt [--background] [--help]
       java -jar reverse_mcp.jar [--background] [--help]
"""

// SSL context that trusts all certificates
fun createTrustAllContext(): SSLContext {
    val trustAllCerts = arrayOf<TrustManager>(object : X509TrustManager {
        override fun getAcceptedIssuers(): Array<X509Certificate>? = null
        override fun checkClientTrusted(certs: Array<X509Certificate>, authType: String) {}
        override fun checkServerTrusted(certs: Array<X509Certificate>, authType: String) {}
    })
    
    return SSLContext.getInstance("TLS").apply {
        init(null, trustAllCerts, java.security.SecureRandom())
    }
}

// Find native messaging manifest
fun findNativeMessagingManifest(): String? {
    val os = System.getProperty("os.name").lowercase()
    val home = System.getProperty("user.home")
    val possiblePaths = mutableListOf<String>()
    
    when {
        "win" in os -> {
            val localAppData = System.getenv("LOCALAPPDATA") ?: "$home\\AppData\\Local"
            possiblePaths.add("$localAppData\\AuraFriday\\com.aurafriday.shim.json")
        }
        "mac" in os -> {
            possiblePaths.addAll(listOf(
                "$home/Library/Application Support/Google/Chrome/NativeMessagingHosts/com.aurafriday.shim.json",
                "$home/Library/Application Support/Chromium/NativeMessagingHosts/com.aurafriday.shim.json"
            ))
        }
        else -> {
            possiblePaths.addAll(listOf(
                "$home/.config/google-chrome/NativeMessagingHosts/com.aurafriday.shim.json",
                "$home/.config/chromium/NativeMessagingHosts/com.aurafriday.shim.json"
            ))
        }
    }
    
    return possiblePaths.firstOrNull { File(it).exists() }
}

// Read manifest
fun readManifest(path: String): Map<String, Any>? {
    return try {
        val content = File(path).readText()
        parseJson(content)
    } catch (e: Exception) {
        System.err.println("Error reading manifest: ${e.message}")
        null
    }
}

// Simple JSON parser for basic structures
@Suppress("UNCHECKED_CAST")
fun parseJson(json: String): Map<String, Any> {
    // For production use, consider using kotlinx.serialization or Gson
    // This is a simplified parser for the demo
    val trimmed = json.trim()
    if (!trimmed.startsWith("{")) {
        throw IllegalArgumentException("Not a JSON object")
    }
    
    val result = mutableMapOf<String, Any>()
    var content = trimmed.substring(1, trimmed.lastIndexOf('}')).trim()
    
    var depth = 0
    val current = StringBuilder()
    var currentKey: String? = null
    var inString = false
    var i = 0
    
    while (i < content.length) {
        val c = content[i]
        
        if (c == '"' && (i == 0 || content[i-1] != '\\')) {
            inString = !inString
        }
        
        if (!inString) {
            when (c) {
                '{', '[' -> depth++
                '}', ']' -> depth--
            }
        }
        
        if (c == ':' && depth == 0 && !inString && currentKey == null) {
            currentKey = current.toString().trim().removeSurrounding("\"")
            current.clear()
            i++
            continue
        }
        
        if (c == ',' && depth == 0 && !inString) {
            if (currentKey != null) {
                val value = current.toString().trim()
                result[currentKey] = if (value.startsWith("{")) {
                    parseJson(value)
                } else {
                    value.removeSurrounding("\"")
                }
                currentKey = null
                current.clear()
            }
            i++
            continue
        }
        
        current.append(c)
        i++
    }
    
    if (currentKey != null && current.isNotEmpty()) {
        val value = current.toString().trim()
        result[currentKey] = if (value.startsWith("{")) {
            parseJson(value)
        } else {
            value.removeSurrounding("\"")
        }
    }
    
    return result
}

// Discover MCP server endpoint
fun discoverMCPServerEndpoint(manifest: Map<String, Any>): Map<String, Any>? {
    val binaryPath = manifest["path"] as? String ?: run {
        System.err.println("ERROR: No path in manifest")
        return null
    }
    
    System.err.println("Running native binary: $binaryPath")
    System.err.println("[DEBUG] Native messaging protocol uses 4-byte length prefix (little-endian uint32)")
    
    return try {
        val process = ProcessBuilder(binaryPath).start()
        
        val input = process.inputStream
        
        // Step 1: Read the 4-byte length prefix (little-endian uint32)
        val lengthBytes = ByteArray(4)
        var bytesRead = 0
        while (bytesRead < 4) {
            val n = input.read(lengthBytes, bytesRead, 4 - bytesRead)
            if (n == -1) break
            bytesRead += n
        }
        
        if (bytesRead != 4) {
            System.err.println("ERROR: Failed to read 4-byte length prefix (got $bytesRead bytes)")
            process.destroy()
            return null
        }
        
        // Convert little-endian bytes to int
        val messageLength = ByteBuffer.wrap(lengthBytes)
            .order(ByteOrder.LITTLE_ENDIAN)
            .int
        
        System.err.println("[DEBUG] Message length from native binary: $messageLength bytes")
        
        if (messageLength <= 0 || messageLength > 10_000_000) {
            System.err.println("ERROR: Invalid message length: $messageLength")
            process.destroy()
            return null
        }
        
        // Step 2: Read the JSON payload of the specified length
        val jsonBytes = ByteArray(messageLength)
        bytesRead = 0
        while (bytesRead < messageLength) {
            val n = input.read(jsonBytes, bytesRead, messageLength - bytesRead)
            if (n == -1) break
            bytesRead += n
        }
        
        if (bytesRead != messageLength) {
            System.err.println("ERROR: Stream ended after $bytesRead bytes (expected $messageLength)")
            process.destroy()
            return null
        }
        
        val jsonStr = String(jsonBytes, Charsets.UTF_8)
        System.err.println("[DEBUG] Successfully read $bytesRead bytes of JSON")
        System.err.println("[DEBUG] JSON preview: ${jsonStr.take(100)}...")
        
        process.destroy()
        parseJson(jsonStr)
    } catch (e: Exception) {
        System.err.println("ERROR: Failed to run native binary: ${e.message}")
        null
    }
}

// Extract server URL and auth from config
@Suppress("UNCHECKED_CAST")
fun extractServerInfo(config: Map<String, Any>): Pair<String?, String?> {
    var url: String? = null
    var auth: String? = null
    
    try {
        val mcpServers = config["mcpServers"] as? Map<String, Any>
        if (mcpServers != null) {
            for (serverVal in mcpServers.values) {
                val serverInfo = serverVal as? Map<String, Any> ?: continue
                url = serverInfo["url"] as? String
                
                val headers = serverInfo["headers"] as? Map<String, Any>
                if (headers != null) {
                    auth = headers["Authorization"] as? String
                }
                
                if (url != null) break
            }
        }
    } catch (e: Exception) {
        System.err.println("[DEBUG] Structured extraction failed: ${e.message}")
    }
    
    System.err.println("[DEBUG] Extracted URL: $url")
    System.err.println("[DEBUG] Extracted Auth: ${if (auth != null) "Bearer ***" else "null"}")
    
    return Pair(url, auth)
}

// SSE Connection class
class SSEConnection(
    val serverUrl: String,
    val authHeader: String
) {
    var sessionId: String? = null
    var messageEndpoint: String? = null
    val reverseQueue = LinkedBlockingQueue<Map<String, Any>>()
    val responseQueues = ConcurrentHashMap<String, BlockingQueue<Map<String, Any>>>()
    val sslContext = createTrustAllContext()
    @Volatile var isAlive = false
    var readerThread: Thread? = null
    
    fun connect() {
        val url = URL(serverUrl)
        val conn = if (serverUrl.startsWith("https")) {
            val httpsConn = url.openConnection() as HttpsURLConnection
            httpsConn.sslSocketFactory = sslContext.socketFactory
            httpsConn.hostnameVerifier = HostnameVerifier { _, _ -> true }
            httpsConn
        } else {
            url.openConnection() as HttpURLConnection
        }
        
        conn.requestMethod = "GET"
        conn.setRequestProperty("Accept", "text/event-stream")
        conn.setRequestProperty("Cache-Control", "no-cache")
        conn.setRequestProperty("Authorization", authHeader)
        
        if (conn.responseCode != 200) {
            throw Exception("HTTP ${conn.responseCode}")
        }
        
        // Read SSE stream in background thread
        isAlive = true
        readerThread = Thread {
            try {
                BufferedReader(InputStreamReader(conn.inputStream)).use { reader ->
                    var eventType: String? = null
                    
                    while (true) {
                        val line = reader.readLine() ?: break
                        val trimmed = line.trim()
                        
                        if (trimmed.isEmpty()) {
                            eventType = null
                            continue
                        }
                        
                        if (trimmed.startsWith(":")) continue
                        
                        val colonIdx = trimmed.indexOf(':')
                        if (colonIdx == -1) continue
                        
                        val field = trimmed.substring(0, colonIdx)
                        val value = trimmed.substring(colonIdx + 1).trim()
                        
                        when (field) {
                            "event" -> eventType = value
                            "data" -> {
                                if (eventType == "endpoint") {
                                    messageEndpoint = value
                                    if ("session_id=" in value) {
                                        sessionId = value.split("session_id=")[1].split("&")[0]
                                    }
                                } else {
                                    // Parse JSON and route
                                    if ("\"reverse\"" in value) {
                                        reverseQueue.put(mapOf("_raw" to value))
                                    } else if ("\"id\"" in value) {
                                        val id = extractId(value)
                                        if (id != null && responseQueues.containsKey(id)) {
                                            responseQueues[id]?.put(mapOf("_raw" to value))
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (e: Exception) {
                System.err.println("SSE reader error: ${e.message}")
            } finally {
                isAlive = false
            }
        }.apply { start() }
        
        // Wait for session ID
        repeat(50) {
            if (sessionId != null) return
            Thread.sleep(100)
        }
        
        if (sessionId == null) {
            throw Exception("No session ID received")
        }
    }
    
    fun extractId(json: String): String? {
        val idIdx = json.indexOf("\"id\":")
        if (idIdx != -1) {
            val start = json.indexOf("\"", idIdx + 5) + 1
            val end = json.indexOf("\"", start)
            if (end > start) {
                return json.substring(start, end)
            }
        }
        return null
    }
    
    fun sendRequest(method: String, paramsJson: String): Map<String, Any>? {
        val requestId = UUID.randomUUID().toString()
        val body = """{"jsonrpc":"2.0","id":"$requestId","method":"$method","params":$paramsJson}"""
        
        val responseQueue = LinkedBlockingQueue<Map<String, Any>>()
        responseQueues[requestId] = responseQueue
        
        return try {
            val url = URL(serverUrl.replace(Regex("/sse.*"), "") + messageEndpoint)
            val conn = if (serverUrl.startsWith("https")) {
                val httpsConn = url.openConnection() as HttpsURLConnection
                httpsConn.sslSocketFactory = sslContext.socketFactory
                httpsConn.hostnameVerifier = HostnameVerifier { _, _ -> true }
                httpsConn
            } else {
                url.openConnection() as HttpURLConnection
            }
            
            conn.requestMethod = "POST"
            conn.doOutput = true
            conn.setRequestProperty("Content-Type", "application/json")
            conn.setRequestProperty("Authorization", authHeader)
            
            conn.outputStream.use { os ->
                os.write(body.toByteArray(Charsets.UTF_8))
            }
            
            if (conn.responseCode != 202) {
                throw Exception("POST failed: ${conn.responseCode}")
            }
            
            responseQueue.poll(10, TimeUnit.SECONDS)
        } finally {
            responseQueues.remove(requestId)
        }
    }
    
    fun sendToolReply(callId: String, resultJson: String) {
        val body = """{"jsonrpc":"2.0","id":"$callId","method":"tools/reply","params":{"result":$resultJson}}"""
        
        val url = URL(serverUrl.replace(Regex("/sse.*"), "") + messageEndpoint)
        val conn = if (serverUrl.startsWith("https")) {
            val httpsConn = url.openConnection() as HttpsURLConnection
            httpsConn.sslSocketFactory = sslContext.socketFactory
            httpsConn.hostnameVerifier = HostnameVerifier { _, _ -> true }
            httpsConn
        } else {
            url.openConnection() as HttpURLConnection
        }
        
        conn.requestMethod = "POST"
        conn.doOutput = true
        conn.setRequestProperty("Content-Type", "application/json")
        conn.setRequestProperty("Authorization", authHeader)
        
        conn.outputStream.use { os ->
            os.write(body.toByteArray(Charsets.UTF_8))
        }
        
        if (conn.responseCode == 202) {
            System.err.println("[OK] Sent tools/reply for call_id $callId")
        }
    }
}

// Register demo tool
fun registerDemoTool(conn: SSEConnection) {
    System.err.println("Registering demo_tool_kotlin with MCP server...")
    
    // Get source file location
    val sourceFile = File("reverse_mcp.kt").absolutePath
    
    val params = """{"name":"remote","arguments":{"input":{
        "operation":"register",
        "tool_name":"demo_tool_kotlin",
        "readme":"Demo tool that echoes messages back and can call other MCP tools.\n- Use this to test the remote tool system and verify bidirectional communication.\n- Demonstrates how remote tools can call OTHER tools on the server (like sqlite, browser, etc.)",
        "description":"Demo tool (Kotlin implementation) for testing remote tool registration and end-to-end MCP communication. This tool demonstrates TWO key capabilities: (1) Basic echo functionality - echoes back any message sent to it, and (2) Tool-to-tool communication - shows how remote tools can call OTHER MCP tools on the server. This verifies that: (a) tool registration works correctly, (b) reverse calls from server to client function properly, (c) the client can successfully reply to tool calls, (d) the full bidirectional JSON-RPC communication channel is operational, and (e) remote tools can orchestrate other tools. This tool is implemented in ${sourceFile.replace("\\", "\\\\")} and serves as a reference template for integrating MCP tool support into other applications like Fusion 360, Blender, Ghidra, and similar products. Usage workflow: (1) Start by discovering databases: {\"message\": \"list databases\"} calls sqlite to show all available databases. (2) Then list tables in a specific database: {\"message\": \"list tables in test.db\"} calls sqlite and returns table names. (3) Basic echo: {\"message\": \"test\"} returns 'Echo: test'. The tool automatically detects keywords in the message to trigger different demonstrations.",
        "parameters":{"type":"object","properties":{"message":{"type":"string","description":"The message to echo back"}},"required":["message"]},
        "callback_endpoint":"kotlin-client://demo-tool-callback",
        "TOOL_API_KEY":"kotlin_demo_tool_auth_key_12345"
    }}}"""
    
    val response = conn.sendRequest("tools/call", params)
    if (response != null && response["_raw"].toString().contains("Successfully registered tool")) {
        System.err.println("[OK] Successfully registered tool: demo_tool_kotlin")
    } else {
        throw Exception("Registration failed")
    }
}

/**
 * Call another MCP tool on the server.
 * 
 * This function demonstrates how to call other MCP tools from within your remote tool handler.
 * It uses the existing SSE connection and JSON-RPC infrastructure to make tool calls.
 * 
 * @param conn Active SSE connection
 * @param toolName Name of the tool to call (e.g., "sqlite", "browser", "user")
 * @param argumentsJson JSON string of arguments to pass to the tool
 * @return JSON-RPC response map, or null on error
 * 
 * Example:
 *   // Call sqlite tool to list tables
 *   val result = callMcpTool(
 *       conn,
 *       "sqlite",
 *       """{"input":{"sql":".tables","tool_unlock_token":"29e63eb5"}}"""
 *   )
 */
fun callMcpTool(conn: SSEConnection, toolName: String, argumentsJson: String): Map<String, Any>? {
    return try {
        val params = """{"name":"$toolName","arguments":$argumentsJson}"""
        
        val requestId = UUID.randomUUID().toString()
        val body = """{"jsonrpc":"2.0","id":"$requestId","method":"tools/call","params":$params}"""
        
        val responseQueue = LinkedBlockingQueue<Map<String, Any>>()
        conn.responseQueues[requestId] = responseQueue
        
        try {
            val url = URL(conn.serverUrl.replace(Regex("/sse.*"), "") + conn.messageEndpoint)
            val httpConn = if (conn.serverUrl.startsWith("https")) {
                val httpsConn = url.openConnection() as HttpsURLConnection
                httpsConn.sslSocketFactory = conn.sslContext.socketFactory
                httpsConn.hostnameVerifier = HostnameVerifier { _, _ -> true }
                httpsConn
            } else {
                url.openConnection() as HttpURLConnection
            }
            
            httpConn.requestMethod = "POST"
            httpConn.doOutput = true
            httpConn.setRequestProperty("Content-Type", "application/json")
            httpConn.setRequestProperty("Authorization", conn.authHeader)
            
            httpConn.outputStream.use { os ->
                os.write(body.toByteArray(Charsets.UTF_8))
            }
            
            if (httpConn.responseCode != 202) {
                System.err.println("ERROR: Tool call POST failed: ${httpConn.responseCode}")
                return null
            }
            
            // Wait up to 30 seconds for response
            responseQueue.poll(30, TimeUnit.SECONDS)
        } finally {
            conn.responseQueues.remove(requestId)
        }
    } catch (e: Exception) {
        System.err.println("ERROR: Failed to call MCP tool: ${e.message}")
        null
    }
}

/**
 * Handle an echo request from the server.
 * 
 * This demonstrates TWO capabilities:
 * 1. Basic echo functionality - echoes back the message
 * 2. Calling OTHER MCP tools - demonstrates how to call sqlite, browser, etc.
 * 
 * @param inputJson The tool call data from the reverse message
 * @param conn SSE connection for making tool calls (null for basic echo only)
 * @return Result JSON string to send back
 */
fun handleEchoRequest(inputJson: String, conn: SSEConnection?): String {
    var message = "(no message provided)"
    
    if ("\"message\"" in inputJson) {
        val msgIdx = inputJson.indexOf("\"message\":")
        if (msgIdx != -1) {
            val start = inputJson.indexOf("\"", msgIdx + 10) + 1
            val end = inputJson.indexOf("\"", start)
            if (end > start) {
                message = inputJson.substring(start, end)
            }
        }
    }
    
    System.err.println("[ECHO] Received echo request: $message")
    
    // Basic echo response
    val responseText = StringBuilder("Echo: $message")
    
    // DEMONSTRATION: If we have connection info, show how to call other tools
    if (conn != null) {
        val messageLower = message.lowercase()
        
        // Demo 1: List databases (triggered by keyword "databases" or "db")
        if ("databases" in messageLower || "list db" in messageLower) {
            System.err.println("[DEMO] Calling sqlite tool to list databases...")
            
            val sqliteResult = callMcpTool(
                conn,
                "sqlite",
                """{"input":{"sql":".databases","tool_unlock_token":"29e63eb5"}}"""
            )
            
            if (sqliteResult != null && sqliteResult["_raw"] != null) {
                responseText.append("\n\n[DEMO] Called sqlite tool successfully!\nResult:\n")
                responseText.append(formatJsonForDisplay(sqliteResult["_raw"].toString()))
            } else {
                responseText.append("\n\n[DEMO] SQLite tool call failed or returned no result")
            }
        }
        // Demo 2: List tables (triggered by keywords "tables")
        else if ("tables" in messageLower) {
            System.err.println("[DEMO] Calling sqlite tool to list tables...")
            
            var database = ":memory:"
            if (" in " in messageLower) {
                val parts = message.split(" in ")
                if (parts.size > 1) {
                    database = parts[1].trim()
                }
            }
            
            val sqliteResult = callMcpTool(
                conn,
                "sqlite",
                """{"input":{"sql":".tables","database":"$database","tool_unlock_token":"29e63eb5"}}"""
            )
            
            if (sqliteResult != null && sqliteResult["_raw"] != null) {
                responseText.append("\n\n[DEMO] Called sqlite tool successfully!\n")
                responseText.append("Database: $database\n")
                responseText.append("Result:\n")
                responseText.append(formatJsonForDisplay(sqliteResult["_raw"].toString()))
            } else {
                responseText.append("\n\n[DEMO] SQLite tool call failed or returned no result")
            }
        }
    }
    
    // Escape quotes in response text for JSON
    val escapedText = responseText.toString()
        .replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
    return """{"content":[{"type":"text","text":"$escapedText"}],"isError":false}"""
}

// Helper to format JSON for display
fun formatJsonForDisplay(json: String): String {
    if ("\"result\"" in json) {
        val resultIdx = json.indexOf("\"result\":")
        if (resultIdx != -1) {
            val start = json.indexOf("{", resultIdx)
            var depth = 0
            for (i in start until json.length) {
                when (json[i]) {
                    '{' -> depth++
                    '}' -> {
                        depth--
                        if (depth == 0) {
                            return json.substring(start, i + 1)
                        }
                    }
                }
            }
        }
    }
    return json
}

// Main worker
fun mainWorker(): Int {
    System.err.println("=== Aura Friday Remote Tool Provider Demo ===")
    System.err.println("PID: ${ProcessHandle.current().pid()}")
    System.err.println("Registering demo_tool with MCP server\n")
    
    var retryCount = 0
    val maxRetryDelay = 60
    
    // Outer reconnection loop
    while (true) {
        try {
            // Calculate retry delay with exponential backoff
            if (retryCount > 0) {
                val delay = minOf((1 shl retryCount), maxRetryDelay)
                System.err.println("\n[RECONNECT] Waiting $delay seconds before retry (attempt #$retryCount)...")
                Thread.sleep(delay * 1000L)
                System.err.println("[RECONNECT] Attempting to reconnect...\n")
            }
            
            // Step 1
            System.err.println("Step 1: Finding native messaging manifest...")
            val manifestPath = findNativeMessagingManifest()
            if (manifestPath == null) {
                System.err.println("ERROR: Could not find manifest")
                retryCount++
                continue
            }
            System.err.println("[OK] Found manifest: $manifestPath\n")
            
            // Step 2
            System.err.println("Step 2: Reading manifest...")
            val manifest = readManifest(manifestPath)
            if (manifest == null) {
                System.err.println("ERROR: Could not read manifest")
                retryCount++
                continue
            }
            System.err.println("[OK] Manifest loaded\n")
            
            // Step 3
            System.err.println("Step 3: Discovering MCP server endpoint...")
            val config = discoverMCPServerEndpoint(manifest)
            if (config == null) {
                System.err.println("ERROR: Could not get configuration")
                retryCount++
                continue
            }
            
            val (serverUrl, authHeader) = extractServerInfo(config)
            if (serverUrl == null) {
                System.err.println("ERROR: Could not extract server URL")
                retryCount++
                continue
            }
            System.err.println("[OK] Found server at: $serverUrl\n")
            
            // Step 4
            System.err.println("Step 4: Connecting to SSE endpoint...")
            val conn = SSEConnection(serverUrl, authHeader!!)
            conn.connect()
            System.err.println("[OK] Connected! Session ID: ${conn.sessionId}\n")
            
            // Step 5
            System.err.println("Step 5: Checking for remote tool...")
            val toolsResponse = conn.sendRequest("tools/list", "{}")
            if (toolsResponse == null || !toolsResponse["_raw"].toString().contains("\"remote\"")) {
                System.err.println("ERROR: No remote tool found")
                conn.readerThread?.interrupt()
                retryCount++
                continue
            }
            System.err.println("[OK] Remote tool found\n")
            
            // Step 6
            System.err.println("Step 6: Registering demo_tool_kotlin...")
            registerDemoTool(conn)
            
            retryCount = 0
            
            System.err.println("\n" + "=".repeat(60))
            System.err.println("[OK] demo_tool_kotlin registered successfully!")
            System.err.println("Listening for reverse tool calls... (Press Ctrl+C to stop)")
            System.err.println("=".repeat(60) + "\n")
            
            // Step 7: Listen for reverse calls
            while (true) {
                if (!conn.isAlive) {
                    System.err.println("\n[WARN] SSE connection lost - reconnecting...")
                    conn.readerThread?.interrupt()
                    retryCount = 1
                    break
                }
                
                val msg = conn.reverseQueue.poll(1, TimeUnit.SECONDS)
                if (msg != null) {
                    val raw = msg["_raw"] as String
                    
                    System.err.println("\n[CALL] Reverse call received:")
                    System.err.println("       Tool: demo_tool_kotlin")
                    
                    val callId = conn.extractId(raw)
                    if (callId != null) {
                        val result = handleEchoRequest(raw, conn)
                        conn.sendToolReply(callId, result)
                    }
                }
            }
            
        } catch (e: InterruptedException) {
            System.err.println("\n\n" + "=".repeat(60))
            System.err.println("Shutting down...")
            System.err.println("=".repeat(60))
            return 0
        } catch (e: Exception) {
            System.err.println("\n[ERROR] Unexpected error in main loop: ${e.message}")
            e.printStackTrace()
            retryCount++
        }
    }
}

fun main(args: Array<String>) {
    val background = "--background" in args
    val help = "--help" in args
    
    if (help) {
        println("Usage: kotlin reverse_mcp.kt [--background]")
        println("       java -jar reverse_mcp.jar [--background]")
        println("\nAura Friday Remote Tool Provider - Registers demo_tool_kotlin with MCP server")
        println("\n$DOCO")
        exitProcess(0)
    }
    
    if (background) {
        val pid = ProcessHandle.current().pid()
        System.err.println("Starting in background mode (PID: $pid)...")
        System.err.println("[OK] Background worker started (PID: $pid)")
        System.err.println("  Use 'kill $pid' to stop")
    }
    
    exitProcess(mainWorker())
}


