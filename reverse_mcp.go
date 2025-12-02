package main

/*
File: reverse_mcp.go
Project: Aura Friday MCP-Link Server - Remote Tool Provider Demo
Component: Registers a demo tool with the MCP server and handles reverse calls
Author: Christopher Nathan Drake (cnd)
Created: 2025-11-03
Last Modified: 2025-11-03 by cnd (Go port from Python)
SPDX-License-Identifier: Apache-2.0
Copyright: (c) 2025 Christopher Nathan Drake. All rights reserved.
"signature": "Ə𝖠ⲞƎЕ𝟧оƴᏂƨѵƻᴍᒿp𝟧ďνHοm𝕌𝟛𝙰ⅠƳᏎ𝐴ďᏮvꓔȢQGΝᏎᎠ𝟛ꓣⅼMƶȜsⲞWоꙅĐꓑⲘ𝟪𝟧ꓜƌĐꓪАᎠυᗪsᴅm9𝟦wS6Þᑕ𝖠4ɪеᖴνDѡIꓔƌgꜱꓓųcųs𝕌ƽΗꓳH𝟥ųοꓦųμ8һꓝT𝟩ȷꓓP",
"signdate": "2025-12-02T06:27:47.586Z",

VERSION: 2025.11.03.001 - Remote Tool Provider Demo (Go)

This script demonstrates how to register a tool with the MCP server using the remote tool system.
It acts as a tool provider that:
1. Connects to the MCP server via native messaging discovery
2. Registers a "demo_tool_go" with the server
3. Listens for reverse tool calls from the server
4. Processes "echo" requests and sends back replies
5. Demonstrates calling OTHER MCP tools (sqlite, browser, etc.) from within the handler
6. Runs continuously until stopped with Ctrl+C

The demo tool responds to these message patterns:
- "list databases" or "list db" - Calls sqlite to list all databases (START HERE to discover what's available)
- "list tables" - Calls sqlite to list tables in :memory: database
- "list tables in <database>" - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")
- Any other message - Simple echo response

BUILD/RUN INSTRUCTIONS:
  Build:
    Windows:  go build -a -ldflags="-s -w" -o reverse_mcp_go.exe reverse_mcp.go
    Mac/Linux: go build -a -ldflags="-s -w" -o reverse_mcp_go reverse_mcp.go
  
  Requirements:
    - Go 1.16+ (tested with Go 1.21+)
    - Standard library only (no go get needed)
  
  Run:
    Windows:  reverse_mcp_go.exe [--background]
    Mac/Linux: ./reverse_mcp_go [--background]

Usage: reverse_mcp_go [--background] [--help]

HOW TO USE THIS CODE:
---------------------
This code is a complete, self-contained reference template for integrating MCP (Model Context Protocol)
tool support into other applications like Fusion 360, Blender, Ghidra, and similar products.

HOW THIS WORKS:
---------------
1. You create a new add-on or extension or plugin or similar for the application you want to let an AI 
   control on your behalf. (hereafter addIn)
2. This template gives your new addIn the facility to discover the correct endpoint where a local 
   controller MCP server is running, and then:
3. lets your addIn register itself with that server as a new tool, which any AI using that MCP server 
   can then discover and access and use.
4. and finally, this template processes incoming tool requests from the AI, which you implement in your 
   addIn, and this template sends the results of those tool-calls back to the AI.
5. BONUS: Your addIn can also CALL OTHER MCP tools on the server (sqlite, browser, user, etc.) - making 
   it possible to orchestrate complex workflows!
*. The server installer can be found at https://github.com/aurafriday/mcp-link-server/releases

ARCHITECTURE OVERVIEW:
----------------------
1. Native Messaging Discovery: Locates the MCP server by finding the Chrome native messaging manifest
   (com.aurafriday.shim.json) which is installed by the Aura Friday MCP-Link server.

2. Server Configuration: Executes the native messaging binary to get the server's SSE endpoint URL
   and authentication token. The binary is a long-running stdio service, so we terminate it after
   reading the initial JSON config.

3. SSE Connection: Establishes a persistent Server-Sent Events (SSE) connection to receive messages
   from the server. This runs in a background goroutine and routes incoming messages to the appropriate
   handlers.

4. Dual-Channel Communication:
   - POST requests (via HTTP/HTTPS) to send JSON-RPC commands to the server
   - SSE stream (long-lived GET connection) to receive JSON-RPC responses and reverse tool calls

5. Tool Registration: Uses the server's "remote" tool to register your custom tool with these components:
   - tool_name: Unique identifier for your tool
   - readme: Minimal summary for the AI (when to use this tool)
   - description: Comprehensive documentation for the AI (what it does, how to use it, examples)
   - parameters: JSON Schema defining the tool's input parameters
   - callback_endpoint: Identifier for routing reverse calls back to your client
   - TOOL_API_KEY: Authentication key for your tool

6. Reverse Call Handling: After registration, your tool appears in the server's tool list. When an
   AI agent calls your tool, the server sends a "reverse" message via the SSE stream containing:
   - tool: Your tool's name
   - call_id: Unique ID for this invocation (used to send the reply)
   - input: The parameters passed by the AI

7. Reply Mechanism: Your code processes the request and sends a "tools/reply" message back to the
   server with the call_id and result. The server forwards this to the AI.

INTEGRATION STEPS:
------------------
1. Copy this file to your project
2. Modify the tool registration section (search for "demo_tool_go"):
   - Change tool_name to your tool's unique identifier
   - Update description and readme to explain your tool's purpose
   - Define your tool's parameters schema
   - Set a unique callback_endpoint and TOOL_API_KEY

3. Replace the handleEchoRequest() function with your tool's actual logic:
   - Extract parameters from the input_data
   - Perform your tool's operations (file I/O, API calls, computations, etc.)
   - OPTIONALLY: Call other MCP tools using callMCPTool() function
   - Return a result map with "content" array and "isError" boolean

4. (Optional) Use callMCPTool() to orchestrate other MCP tools:
   - Your handler receives conn pointer with connection info
   - Use callMCPTool() to call sqlite, browser, user, or any other MCP tool
   - Example: sqliteResult, _ := callMCPTool(conn, "sqlite", 
                                              map[string]interface{}{"input": map[string]interface{}{
                                                "sql": ".tables", "tool_unlock_token": "29e63eb5"}})
   - This enables complex workflows like: read data from app → query database → show results to user

5. Build and run your tool provider:
   - It will auto-discover the server, register your tool, and listen for calls
   - The tool remains registered as long as the process is running
   - Press Ctrl+C to cleanly shut down

RESULT FORMAT:
--------------
All tool results must follow this structure:
{
  "content": [
    {"type": "text", "text": "Your response text here"},
    {"type": "image", "data": "base64...", "mimeType": "image/png"}  // optional
  ],
  "isError": false  // or true if an error occurred
}

CONCURRENCY MODEL:
------------------
- Main goroutine: Handles tool registration and processes reverse calls from channels
- SSE reader goroutine: Continuously reads the SSE stream and routes messages to channels
- Each JSON-RPC request gets its own response channel for thread-safe blocking waits

DEPENDENCIES:
-------------
Go 1.16+ with standard library only:
- net/http, crypto/tls: Network communication
- encoding/json: JSON parsing
- os/exec: Execute native messaging binary
- bufio, bytes: Stream processing

ERROR HANDLING & RECONNECTION:
-------------------------------
- SSL certificate verification is disabled (self-signed certs are common in local servers)
- Native binary timeout is 5 seconds (increase if needed)
- SSE response timeout is 10 seconds per request (configurable)
- All errors are logged to stderr for debugging
- Automatic reconnection with exponential backoff if SSE connection drops:
  * Retry delays: 2s, 4s, 8s, 16s, 32s, 60s (max), 60s, 60s...
  * After successful reconnection, retry counter resets
  * Tool is automatically re-registered after reconnection
  * Retries forever until manually stopped (Ctrl+C)
*/

import (
	"bufio"
	"bytes"
	"crypto/tls"
	"encoding/binary"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
	"time"
)

// Configuration structures
type MCPServer struct {
	URL     string            `json:"url"`
	Note    string            `json:"note"`
	Headers map[string]string `json:"headers"`
}

type Config struct {
	MCPServers map[string]MCPServer `json:"mcpServers"`
}

type Manifest struct {
	Name string `json:"name"`
	Type string `json:"type"`
	Path string `json:"path"`
}

// Message structures
type JSONRPCRequest struct {
	JSONRPC string      `json:"jsonrpc"`
	ID      string      `json:"id"`
	Method  string      `json:"method"`
	Params  interface{} `json:"params"`
}

type JSONRPCResponse struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      string          `json:"id"`
	Result  json.RawMessage `json:"result,omitempty"`
	Error   interface{}     `json:"error,omitempty"`
}

type ReverseCall struct {
	Tool    string          `json:"tool"`
	CallID  string          `json:"call_id"`
	Input   json.RawMessage `json:"input"`
	IsError bool            `json:"isError"`
}

type ReverseMessage struct {
	JSONRPC string      `json:"jsonrpc"`
	ID      string      `json:"id"`
	Reverse ReverseCall `json:"reverse"`
}

// SSE Connection
type SSEConnection struct {
	ServerURL       string
	AuthHeader      string
	SessionID       string
	MessageEndpoint string
	Client          *http.Client
	ReverseChannel  chan ReverseMessage
	ResponseChannel map[string]chan JSONRPCResponse
	StopChannel     chan bool
	IsAlive         *bool
}

// Find native messaging manifest
func findNativeMessagingManifest() (string, error) {
	var possiblePaths []string
	homeDir, _ := os.UserHomeDir()

	switch runtime.GOOS {
	case "windows":
		localAppData := os.Getenv("LOCALAPPDATA")
		if localAppData == "" {
			localAppData = filepath.Join(homeDir, "AppData", "Local")
		}
		possiblePaths = append(possiblePaths, filepath.Join(localAppData, "AuraFriday", "com.aurafriday.shim.json"))

	case "darwin":
		possiblePaths = append(possiblePaths,
			filepath.Join(homeDir, "Library/Application Support/Google/Chrome/NativeMessagingHosts/com.aurafriday.shim.json"),
			filepath.Join(homeDir, "Library/Application Support/Chromium/NativeMessagingHosts/com.aurafriday.shim.json"),
			filepath.Join(homeDir, "Library/Application Support/Microsoft Edge/NativeMessagingHosts/com.aurafriday.shim.json"),
		)

	default: // linux
		possiblePaths = append(possiblePaths,
			filepath.Join(homeDir, ".config/google-chrome/NativeMessagingHosts/com.aurafriday.shim.json"),
			filepath.Join(homeDir, ".config/chromium/NativeMessagingHosts/com.aurafriday.shim.json"),
			filepath.Join(homeDir, ".config/microsoft-edge/NativeMessagingHosts/com.aurafriday.shim.json"),
		)
	}

	for _, path := range possiblePaths {
		if _, err := os.Stat(path); err == nil {
			return path, nil
		}
	}

	return "", fmt.Errorf("manifest not found")
}

// Read manifest
func readManifest(path string) (*Manifest, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}

	var manifest Manifest
	if err := json.Unmarshal(data, &manifest); err != nil {
		return nil, err
	}

	return &manifest, nil
}

// Discover MCP server endpoint
func discoverMCPServerEndpoint(manifest *Manifest) (*Config, error) {
	binaryPath := manifest.Path
	if _, err := os.Stat(binaryPath); err != nil {
		return nil, fmt.Errorf("binary not found: %s", binaryPath)
	}

	fmt.Fprintf(os.Stderr, "Running native binary: %s\n", binaryPath)
	fmt.Fprintf(os.Stderr, "[DEBUG] Native messaging protocol uses 4-byte length prefix (little-endian uint32)\n")

	cmd := exec.Command(binaryPath)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, err
	}

	if err := cmd.Start(); err != nil {
		return nil, err
	}

	// Read output using Chrome Native Messaging protocol
	// Protocol: 4-byte length (little-endian uint32) followed by JSON message
	done := make(chan *Config, 1)
	go func() {
		// Step 1: Read the 4-byte length prefix (little-endian uint32)
		lengthBytes := make([]byte, 4)
		n, err := io.ReadFull(stdout, lengthBytes)
		if err != nil || n != 4 {
			fmt.Fprintf(os.Stderr, "ERROR: Failed to read 4-byte length prefix (got %d bytes): %v\n", n, err)
			done <- nil
			return
		}

		// Convert little-endian bytes to uint32
		messageLength := binary.LittleEndian.Uint32(lengthBytes)
		fmt.Fprintf(os.Stderr, "[DEBUG] Message length from native binary: %d bytes\n", messageLength)

		if messageLength <= 0 || messageLength > 10000000 {
			fmt.Fprintf(os.Stderr, "ERROR: Invalid message length: %d\n", messageLength)
			done <- nil
			return
		}

		// Step 2: Read the JSON payload of the specified length
		jsonBytes := make([]byte, messageLength)
		n, err = io.ReadFull(stdout, jsonBytes)
		if err != nil || n != int(messageLength) {
			fmt.Fprintf(os.Stderr, "ERROR: Stream ended after %d bytes (expected %d): %v\n", n, messageLength, err)
			done <- nil
			return
		}

		// Step 3: Decode and parse the JSON
		text := string(jsonBytes)
		fmt.Fprintf(os.Stderr, "[DEBUG] Successfully read %d bytes of JSON\n", len(jsonBytes))
		if len(text) > 100 {
			fmt.Fprintf(os.Stderr, "[DEBUG] JSON preview: %s...\n", text[:100])
		} else {
			fmt.Fprintf(os.Stderr, "[DEBUG] JSON preview: %s\n", text)
		}

		var config Config
		if err := json.Unmarshal(jsonBytes, &config); err != nil {
			fmt.Fprintf(os.Stderr, "ERROR: Failed to parse JSON: %v\n", err)
			done <- nil
			return
		}

		done <- &config
	}()

	// Wait for result or timeout
	select {
	case config := <-done:
		cmd.Process.Kill()
		if config != nil {
			return config, nil
		}
		return nil, fmt.Errorf("no valid JSON received")
	case <-time.After(5 * time.Second):
		cmd.Process.Kill()
		return nil, fmt.Errorf("timeout")
	}
}

// Connect to SSE endpoint
func connectSSE(serverURL, authHeader string) (*SSEConnection, error) {
	isAlive := true
	conn := &SSEConnection{
		ServerURL:       serverURL,
		AuthHeader:      authHeader,
		ReverseChannel:  make(chan ReverseMessage, 100),
		ResponseChannel: make(map[string]chan JSONRPCResponse),
		StopChannel:     make(chan bool, 1),
		IsAlive:         &isAlive,
		Client: &http.Client{
			Transport: &http.Transport{
				TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
			},
		},
	}

	req, err := http.NewRequest("GET", serverURL, nil)
	if err != nil {
		return nil, err
	}

	req.Header.Set("Accept", "text/event-stream")
	req.Header.Set("Cache-Control", "no-cache")
	req.Header.Set("Authorization", authHeader)

	resp, err := conn.Client.Do(req)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != 200 {
		return nil, fmt.Errorf("HTTP %d", resp.StatusCode)
	}

	// Read SSE stream
	go func() {
		defer func() {
			*conn.IsAlive = false
		}()
		
		scanner := bufio.NewScanner(resp.Body)
		var eventType string

		for scanner.Scan() {
			select {
			case <-conn.StopChannel:
				return
			default:
			}
			
			line := scanner.Text()
			line = strings.TrimRight(line, "\r")

			if line == "" {
				eventType = ""
				continue
			}

			if strings.HasPrefix(line, ":") {
				continue
			}

			colonIdx := strings.Index(line, ":")
			if colonIdx == -1 {
				continue
			}

			field := line[:colonIdx]
			value := line[colonIdx+1:]
			if strings.HasPrefix(value, " ") {
				value = value[1:]
			}

			switch field {
			case "event":
				eventType = value
			case "data":
				if eventType == "endpoint" {
					conn.MessageEndpoint = value
					// Extract session ID
					if strings.Contains(value, "session_id=") {
						parts := strings.Split(value, "session_id=")
						if len(parts) > 1 {
							conn.SessionID = strings.Split(parts[1], "&")[0]
						}
					}
				} else {
					// Parse as JSON
					var msg map[string]interface{}
					if err := json.Unmarshal([]byte(value), &msg); err == nil {
						if _, ok := msg["reverse"]; ok {
							var revMsg ReverseMessage
							json.Unmarshal([]byte(value), &revMsg)
							conn.ReverseChannel <- revMsg
						} else if id, ok := msg["id"].(string); ok {
							if ch, exists := conn.ResponseChannel[id]; exists {
								var response JSONRPCResponse
								json.Unmarshal([]byte(value), &response)
								ch <- response
								delete(conn.ResponseChannel, id)
							}
						}
					}
				}
			}
		}
	}()

	// Wait for session ID
	for i := 0; i < 50; i++ {
		if conn.SessionID != "" {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}

	if conn.SessionID == "" {
		return nil, fmt.Errorf("no session ID received")
	}

	return conn, nil
}

// Send JSON-RPC request
func (conn *SSEConnection) sendRequest(method string, params interface{}) (json.RawMessage, error) {
	requestID := fmt.Sprintf("%d", time.Now().UnixNano())

	request := JSONRPCRequest{
		JSONRPC: "2.0",
		ID:      requestID,
		Method:  method,
		Params:  params,
	}

	body, err := json.Marshal(request)
	if err != nil {
		return nil, err
	}

	// Create response channel
	respChan := make(chan JSONRPCResponse, 1)
	conn.ResponseChannel[requestID] = respChan

	// Parse URL
	u, _ := url.Parse(conn.ServerURL)
	fullURL := fmt.Sprintf("%s://%s%s", u.Scheme, u.Host, conn.MessageEndpoint)

	req, err := http.NewRequest("POST", fullURL, bytes.NewReader(body))
	if err != nil {
		delete(conn.ResponseChannel, requestID)
		return nil, err
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", conn.AuthHeader)

	resp, err := conn.Client.Do(req)
	if err != nil {
		delete(conn.ResponseChannel, requestID)
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != 202 {
		delete(conn.ResponseChannel, requestID)
		return nil, fmt.Errorf("POST failed: %d", resp.StatusCode)
	}

	// Wait for response via SSE
	select {
	case response := <-respChan:
		return response.Result, nil
	case <-time.After(10 * time.Second):
		delete(conn.ResponseChannel, requestID)
		return nil, fmt.Errorf("timeout")
	}
}

// Send tool reply
func (conn *SSEConnection) sendToolReply(callID string, result interface{}) error {
	params := map[string]interface{}{
		"result": result,
	}

	request := JSONRPCRequest{
		JSONRPC: "2.0",
		ID:      callID,
		Method:  "tools/reply",
		Params:  params,
	}

	body, err := json.Marshal(request)
	if err != nil {
		return err
	}

	u, _ := url.Parse(conn.ServerURL)
	fullURL := fmt.Sprintf("%s://%s%s", u.Scheme, u.Host, conn.MessageEndpoint)

	req, err := http.NewRequest("POST", fullURL, bytes.NewReader(body))
	if err != nil {
		return err
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", conn.AuthHeader)

	resp, err := conn.Client.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode == 202 {
		fmt.Fprintf(os.Stderr, "[OK] Sent tools/reply for call_id %s\n", callID)
		return nil
	}

	return fmt.Errorf("POST failed: %d", resp.StatusCode)
}

// Call another MCP tool on the server
//
// This function demonstrates how to call other MCP tools from within your remote tool handler.
// It uses the existing SSE connection and JSON-RPC infrastructure to make tool calls.
//
// Args:
//   conn: Active SSE connection
//   toolName: Name of the tool to call (e.g., "sqlite", "browser", "user")
//   arguments: Arguments to pass to the tool
//
// Returns:
//   JSON-RPC response as json.RawMessage, or error
//
// Example:
//   // Call sqlite tool to list tables
//   result, err := callMCPTool(conn, "sqlite",
//     map[string]interface{}{
//       "input": map[string]interface{}{
//         "sql": ".tables",
//         "tool_unlock_token": "29e63eb5",
//       },
//     })
//
//   // Call browser tool to list tabs
//   result, err := callMCPTool(conn, "browser",
//     map[string]interface{}{
//       "input": map[string]interface{}{
//         "operation": "list_tabs",
//         "tool_unlock_token": "e5076d",
//       },
//     })
func callMCPTool(conn *SSEConnection, toolName string, arguments interface{}) (json.RawMessage, error) {
	toolCallParams := map[string]interface{}{
		"name":      toolName,
		"arguments": arguments,
	}

	// Use longer timeout for tool calls (30 seconds)
	requestID := fmt.Sprintf("%d", time.Now().UnixNano())

	request := JSONRPCRequest{
		JSONRPC: "2.0",
		ID:      requestID,
		Method:  "tools/call",
		Params:  toolCallParams,
	}

	body, err := json.Marshal(request)
	if err != nil {
		return nil, err
	}

	// Create response channel
	respChan := make(chan JSONRPCResponse, 1)
	conn.ResponseChannel[requestID] = respChan

	// Parse URL
	u, _ := url.Parse(conn.ServerURL)
	fullURL := fmt.Sprintf("%s://%s%s", u.Scheme, u.Host, conn.MessageEndpoint)

	req, err := http.NewRequest("POST", fullURL, bytes.NewReader(body))
	if err != nil {
		delete(conn.ResponseChannel, requestID)
		return nil, err
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", conn.AuthHeader)

	resp, err := conn.Client.Do(req)
	if err != nil {
		delete(conn.ResponseChannel, requestID)
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != 202 {
		delete(conn.ResponseChannel, requestID)
		return nil, fmt.Errorf("POST failed: %d", resp.StatusCode)
	}

	// Wait for response via SSE (longer timeout for tool calls)
	select {
	case response := <-respChan:
		return response.Result, nil
	case <-time.After(30 * time.Second):
		delete(conn.ResponseChannel, requestID)
		return nil, fmt.Errorf("timeout")
	}
}

// Register demo tool
func registerDemoTool(conn *SSEConnection) error {
	fmt.Fprintln(os.Stderr, "Registering demo_tool_go with MCP server...")

	// Get executable path and derive source file location
	exePath, _ := os.Executable()
	sourceFile := strings.Replace(filepath.Base(exePath), filepath.Ext(exePath), ".go", 1)
	if sourceFile == filepath.Base(exePath) {
		sourceFile = "reverse_mcp.go"
	}

	params := map[string]interface{}{
		"name": "remote",
		"arguments": map[string]interface{}{
			"input": map[string]interface{}{
				"operation": "register",
				"tool_name": "demo_tool_go",
				"readme":    "Demo tool that echoes messages back and can call other MCP tools.\n- Use this to test the remote tool system and verify bidirectional communication.\n- Demonstrates how remote tools can call OTHER tools on the server (like sqlite, browser, etc.)", // MINIMAL: Tell the AI ONLY when to use this tool
				"description": fmt.Sprintf("Demo tool (Go implementation) for testing remote tool registration and end-to-end MCP communication. This tool demonstrates TWO key capabilities: (1) Basic echo functionality - echoes back any message sent to it, and (2) Tool-to-tool communication - shows how remote tools can call OTHER MCP tools on the server. This verifies that: (a) tool registration works correctly, (b) reverse calls from server to client function properly, (c) the client can successfully reply to tool calls, (d) the full bidirectional JSON-RPC communication channel is operational, and (e) remote tools can orchestrate other tools. This tool is implemented in %s (same directory as the compiled executable) and serves as a reference template for integrating MCP tool support into other applications like Fusion 360, Blender, Ghidra, and similar products. Usage workflow: (1) Start by discovering databases: {\"message\": \"list databases\"} calls sqlite to show all available databases. (2) Then list tables in a specific database: {\"message\": \"list tables in test.db\"} calls sqlite and returns table names. (3) Basic echo: {\"message\": \"test\"} returns 'Echo: test'. The tool automatically detects keywords in the message to trigger different demonstrations.", sourceFile), // COMPREHENSIVE: Tell the AI everything it needs to know to use this tool
				"parameters": map[string]interface{}{
					"type": "object",
					"properties": map[string]interface{}{
						"message": map[string]interface{}{
							"type":        "string",
							"description": "The message to echo back",
						},
					},
					"required": []string{"message"},
				},
				"callback_endpoint": "go-client://demo-tool-callback",
				"TOOL_API_KEY":      "go_demo_tool_auth_key_12345",
			},
		},
	}

	result, err := conn.sendRequest("tools/call", params)
	if err != nil {
		return err
	}

	var response map[string]interface{}
	if err := json.Unmarshal(result, &response); err == nil {
		if content, ok := response["content"].([]interface{}); ok && len(content) > 0 {
			if item, ok := content[0].(map[string]interface{}); ok {
				if text, ok := item["text"].(string); ok && strings.Contains(text, "Successfully registered tool") {
					fmt.Fprintf(os.Stderr, "[OK] %s\n", text)
					return nil
				}
			}
		}
	}

	return fmt.Errorf("unexpected registration response")
}

// Handle echo request
//
// This demonstrates TWO capabilities:
// 1. Basic echo functionality - echoes back the message
// 2. Calling OTHER MCP tools - demonstrates how to call sqlite, browser, etc.
//
// Args:
//   inputData: The tool call data from the reverse message
//   conn: Optional SSE connection for making tool calls (nil for basic echo only)
//
// Returns:
//   Result map to send back
//
// Example usage from AI:
//   # Basic echo
//   {"message": "Hello World"}
//
//   # Step 1: Discover what databases exist
//   {"message": "list databases"}
//
//   # Step 2: List tables in a specific database
//   {"message": "list tables in test.db"}
//
//   # Or use default :memory: database
//   {"message": "list tables"}
func handleEchoRequest(inputData json.RawMessage, conn *SSEConnection) map[string]interface{} {
	var callData map[string]interface{}
	json.Unmarshal(inputData, &callData)

	params, _ := callData["params"].(map[string]interface{})
	arguments, _ := params["arguments"].(map[string]interface{})
	message, _ := arguments["message"].(string)

	if message == "" {
		message = "(no message provided)"
	}

	fmt.Fprintf(os.Stderr, "[ECHO] Received echo request: %s\n", message)

	// Basic echo response
	responseText := fmt.Sprintf("Echo: %s", message)

	// DEMONSTRATION: If we have connection info, show how to call other tools
	if conn != nil {
		messageLower := strings.ToLower(message)

		// Demo 1: List databases (triggered by keyword "databases" or "db")
		// Check this FIRST because it's more specific and helps users discover what databases exist
		if strings.Contains(messageLower, "databases") || strings.Contains(messageLower, "list db") {
			fmt.Fprintln(os.Stderr, "[DEMO] Calling sqlite tool to list databases...")

			// Call the sqlite tool to list databases
			sqliteResult, err := callMCPTool(conn, "sqlite",
				map[string]interface{}{
					"input": map[string]interface{}{
						"sql":               ".databases",
						"tool_unlock_token": "29e63eb5",
					},
				})

			// Append the result to our response
			if err == nil && sqliteResult != nil {
				resultJSON, _ := json.MarshalIndent(json.RawMessage(sqliteResult), "", "  ")
				responseText += fmt.Sprintf("\n\n[DEMO] Called sqlite tool successfully!\nResult:\n%s", string(resultJSON))
			} else {
				responseText += fmt.Sprintf("\n\n[DEMO] SQLite tool call failed: %v", err)
			}

		// Demo 2: List tables (triggered by keywords "tables" - check AFTER databases to avoid conflicts)
		} else if strings.Contains(messageLower, "tables") {
			fmt.Fprintln(os.Stderr, "[DEMO] Calling sqlite tool to list tables...")

			// Extract database name if specified (e.g., "list tables in test.db")
			database := ":memory:"
			if strings.Contains(messageLower, " in ") {
				parts := strings.Split(message, " in ")
				if len(parts) > 1 {
					database = strings.TrimSpace(parts[1])
				}
			}

			// Call the sqlite tool to list tables
			sqliteResult, err := callMCPTool(conn, "sqlite",
				map[string]interface{}{
					"input": map[string]interface{}{
						"sql":               ".tables",
						"database":          database,
						"tool_unlock_token": "29e63eb5",
					},
				})

			// Append the result to our response
			if err == nil && sqliteResult != nil {
				resultJSON, _ := json.MarshalIndent(json.RawMessage(sqliteResult), "", "  ")
				responseText += fmt.Sprintf("\n\n[DEMO] Called sqlite tool successfully!\nDatabase: %s\nResult:\n%s", database, string(resultJSON))
			} else {
				responseText += fmt.Sprintf("\n\n[DEMO] SQLite tool call failed: %v", err)
			}
		}
	}

	return map[string]interface{}{
		"content": []map[string]interface{}{
			{
				"type": "text",
				"text": responseText,
			},
		},
		"isError": false,
	}
}

// Main worker
func mainWorker() int {
	fmt.Fprintln(os.Stderr, "=== Aura Friday Remote Tool Provider Demo ===")
	fmt.Fprintf(os.Stderr, "PID: %d\n", os.Getpid())
	fmt.Fprintln(os.Stderr, "Registering demo_tool with MCP server\n")

	// Connection state for reconnection logic
	retryCount := 0
	maxRetryDelay := 60 // Max 1 minute between retries

	// Setup signal handling for Ctrl+C
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGTERM)

	// Outer reconnection loop - keeps trying forever
	for {
		// Calculate retry delay with exponential backoff
		if retryCount > 0 {
			delay := 1 << uint(retryCount) // 2^retryCount
			if delay > maxRetryDelay {
				delay = maxRetryDelay
			}
			fmt.Fprintf(os.Stderr, "\n[RECONNECT] Waiting %d seconds before retry (attempt #%d)...\n", delay, retryCount)
			
			// Interruptible sleep
			select {
			case <-time.After(time.Duration(delay) * time.Second):
				fmt.Fprintln(os.Stderr, "[RECONNECT] Attempting to reconnect...\n")
			case <-sigChan:
				fmt.Fprintln(os.Stderr, "\n\n"+strings.Repeat("=", 60))
				fmt.Fprintln(os.Stderr, "Shutting down...")
				fmt.Fprintln(os.Stderr, strings.Repeat("=", 60))
				return 0
			}
		}

		// Step 1: Find manifest
		fmt.Fprintln(os.Stderr, "Step 1: Finding native messaging manifest...")
		manifestPath, err := findNativeMessagingManifest()
		if err != nil {
			fmt.Fprintln(os.Stderr, "ERROR: Could not find native messaging manifest")
			retryCount++
			continue
		}
		fmt.Fprintf(os.Stderr, "[OK] Found manifest: %s\n\n", manifestPath)

		// Step 2: Read manifest
		fmt.Fprintln(os.Stderr, "Step 2: Reading manifest...")
		manifest, err := readManifest(manifestPath)
		if err != nil {
			fmt.Fprintln(os.Stderr, "ERROR: Could not read manifest")
			retryCount++
			continue
		}
		fmt.Fprintln(os.Stderr, "[OK] Manifest loaded\n")

		// Step 3: Discover endpoint
		fmt.Fprintln(os.Stderr, "Step 3: Discovering MCP server endpoint...")
		config, err := discoverMCPServerEndpoint(manifest)
		if err != nil {
			fmt.Fprintln(os.Stderr, "ERROR: Could not get configuration from native binary")
			fmt.Fprintln(os.Stderr, "Is the Aura Friday MCP server running?")
			retryCount++
			continue
		}

		var serverURL, authHeader string
		for _, server := range config.MCPServers {
			serverURL = server.URL
			authHeader = server.Headers["Authorization"]
			break
		}

		if serverURL == "" {
			fmt.Fprintln(os.Stderr, "ERROR: Could not extract server URL")
			retryCount++
			continue
		}
		fmt.Fprintf(os.Stderr, "[OK] Found server at: %s\n\n", serverURL)

		// Step 4: Connect to SSE
		fmt.Fprintln(os.Stderr, "Step 4: Connecting to SSE endpoint...")
		conn, err := connectSSE(serverURL, authHeader)
		if err != nil {
			fmt.Fprintf(os.Stderr, "ERROR: Could not connect: %v\n", err)
			retryCount++
			continue
		}
		fmt.Fprintf(os.Stderr, "[OK] Connected! Session ID: %s\n\n", conn.SessionID)

		// Step 5: Check for remote tool
		fmt.Fprintln(os.Stderr, "Step 5: Checking for remote tool...")
		toolsResult, err := conn.sendRequest("tools/list", map[string]interface{}{})
		if err != nil {
			fmt.Fprintln(os.Stderr, "ERROR: Could not get tools list")
			conn.StopChannel <- true
			retryCount++
			continue
		}

		var toolsResponse map[string]interface{}
		json.Unmarshal(toolsResult, &toolsResponse)
		tools, _ := toolsResponse["tools"].([]interface{})
		hasRemote := false
		for _, tool := range tools {
			if t, ok := tool.(map[string]interface{}); ok {
				if t["name"] == "remote" {
					hasRemote = true
					break
				}
			}
		}

		if !hasRemote {
			fmt.Fprintln(os.Stderr, "ERROR: Server does not have 'remote' tool")
			conn.StopChannel <- true
			retryCount++
			continue
		}
		fmt.Fprintln(os.Stderr, "[OK] Remote tool found\n")

		// Step 6: Register demo_tool_go
		fmt.Fprintln(os.Stderr, "Step 6: Registering demo_tool_go...")
		if err := registerDemoTool(conn); err != nil {
			fmt.Fprintf(os.Stderr, "ERROR: Failed to register: %v\n", err)
			conn.StopChannel <- true
			retryCount++
			continue
		}

		// Reset retry count after successful connection and registration
		retryCount = 0

		fmt.Fprintln(os.Stderr, "\n"+strings.Repeat("=", 60))
		fmt.Fprintln(os.Stderr, "[OK] demo_tool_go registered successfully!")
		fmt.Fprintln(os.Stderr, "Listening for reverse tool calls... (Press Ctrl+C to stop)")
		fmt.Fprintln(os.Stderr, strings.Repeat("=", 60)+"\n")

		// Step 7: Listen for reverse calls
		ticker := time.NewTicker(1 * time.Second)
		defer ticker.Stop()

	innerLoop:
		for {
			select {
			case msg := <-conn.ReverseChannel:
				fmt.Fprintln(os.Stderr, "\n[CALL] Reverse call received:")
				fmt.Fprintf(os.Stderr, "       Tool: %s\n", msg.Reverse.Tool)
				fmt.Fprintf(os.Stderr, "       Call ID: %s\n", msg.Reverse.CallID)

				if msg.Reverse.Tool == "demo_tool_go" {
					// Handle the echo request (pass connection so it can call other tools)
					result := handleEchoRequest(msg.Reverse.Input, conn)
					conn.sendToolReply(msg.Reverse.CallID, result)
				} else {
					fmt.Fprintf(os.Stderr, "[WARN] Unknown tool: %s\n", msg.Reverse.Tool)
				}

			case <-ticker.C:
				// Check if SSE connection is still alive
				if !*conn.IsAlive {
					fmt.Fprintln(os.Stderr, "\n[WARN] SSE connection lost - reconnecting...")
					conn.StopChannel <- true
					retryCount = 1 // Start with first retry delay
					break innerLoop
				}

			case <-sigChan:
				fmt.Fprintln(os.Stderr, "\n\n"+strings.Repeat("=", 60))
				fmt.Fprintln(os.Stderr, "Shutting down...")
				fmt.Fprintln(os.Stderr, strings.Repeat("=", 60))
				conn.StopChannel <- true
				return 0
			}
		}

		// If we get here, connection dropped - outer loop will retry
	}
}

func main() {
	background := flag.Bool("background", false, "Run in background mode")
	help := flag.Bool("help", false, "Show help")
	flag.Parse()

	if *help {
		fmt.Println("Usage: reverse_mcp_go [--background]")
		fmt.Println("\nAura Friday Remote Tool Provider - Registers demo_tool_go with MCP server")
		return
	}

	if *background {
		fmt.Fprintf(os.Stderr, "Starting in background mode (PID: %d)...\n", os.Getpid())
		fmt.Fprintf(os.Stderr, "[OK] Background worker started (PID: %d)\n", os.Getpid())
		fmt.Fprintf(os.Stderr, "  Use 'kill %d' to stop\n", os.Getpid())
	}

	os.Exit(mainWorker())
}

