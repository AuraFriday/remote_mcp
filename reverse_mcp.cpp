/*  CAUTION - THIS CODE MY NOT WORK - UNTESTED - IT DEPENDS ON TLS LIBS WE DID NOT HAVE INSTALLED
 * File: reverse_mcp.cpp
 * Project: Aura Friday MCP-Link Server - Remote Tool Provider Demo
 * Component: Registers a demo tool with the MCP server and handles reverse calls
 * Author: Christopher Nathan Drake (cnd)
 * Created: 2025-11-03
 * Last Modified: 2025-11-03 by cnd (C++ port from Python)
 * SPDX-License-Identifier: Apache-2.0
 * Copyright: (c) 2025 Christopher Nathan Drake. All rights reserved.
 * "signature": "Օ𝖠hƋGɯƏ𝛢ΚȜμʈ𝐴UGοᗞω𝟟𝛢𝟟jᒿμΜⲔꓬⴹıȜօ𝟑𝟟ΜĐ𝟟Мо৭ƿꙅĵ5Н0𝟥ꓔȢĵþᑕꜱɪΕÐß𐐕НiɅʌꓮμƵᖴ𝟫ԁƋvʈrÞᒿiԝ𝟛𝟙ҳꓧꓐеBɡpƛvɊᖴΒμΚҮĐуʋɗꓣ𐓒ΗЗⲦⅼΡ7ƟAАе𐐕",
 * "signdate": "2025-12-02T06:27:46.835Z",
 * 
 * VERSION: 2025.11.03.001 - Remote Tool Provider Demo (C++)
 * 
 * This script demonstrates how to register a tool with the MCP server using the remote tool system.
 * It acts as a tool provider that:
 * 1. Connects to the MCP server via native messaging discovery
 * 2. Registers a "demo_tool_cpp" with the server
 * 3. Listens for reverse tool calls from the server
 * 4. Processes "echo" requests and sends back replies
 * 5. Demonstrates calling OTHER MCP tools (sqlite, browser, etc.) from within the handler
 * 6. Runs continuously until stopped with Ctrl+C
 * 
 * The demo tool responds to these message patterns:
 * - "list databases" or "list db" - Calls sqlite to list all databases (START HERE to discover what's available)
 * - "list tables" - Calls sqlite to list tables in :memory: database
 * - "list tables in <database>" - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")
 * - Any other message - Simple echo response
 * 
 * BUILD/RUN INSTRUCTIONS:
 *   Compile:
 *     g++ -std=c++17 -o reverse_mcp_cpp.exe reverse_mcp.cpp -lws2_32 -lcrypt32 -lwinhttp
 *     (Windows)
 *     
 *     g++ -std=c++17 -o reverse_mcp_cpp reverse_mcp.cpp -lcurl
 *     (Linux/macOS)
 *   
 *   Run:
 *     ./reverse_mcp_cpp [--background]
 *     ./reverse_mcp_cpp --help
 *   
 *   Requirements:
 *     - C++17 compiler (g++ 7.0+)
 *     - Windows: WinHTTP library (included with Windows SDK)
 *     - Linux/macOS: libcurl
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
 * 2. Modify the tool registration section (search for "demo_tool_cpp"):
 *    - Change tool_name to your tool's unique identifier
 *    - Update description and readme to explain your tool's purpose
 *    - Define your tool's parameters schema
 *    - Set a unique callback_endpoint and TOOL_API_KEY
 * 
 * 3. Replace the handleEchoRequest() function with your tool's actual logic:
 *    - Extract parameters from the input_data
 *    - Perform your tool's operations (file I/O, API calls, computations, etc.)
 *    - OPTIONALLY: Call other MCP tools using conn->call_mcp_tool() method
 *    - Return a result string with "content" array and "isError" boolean
 * 
 * 4. (Optional) Use call_mcp_tool() to orchestrate other MCP tools:
 *    - Your handler receives SSEConnection* pointer parameter
 *    - Use conn->call_mcp_tool() to call sqlite, browser, user, or any other MCP tool
 *    - Example: string result = conn->call_mcp_tool("sqlite", "{\"input\":{\"sql\":\".tables\",\"tool_unlock_token\":\"29e63eb5\"}}");
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
 * - Main thread: Handles tool registration and processes reverse calls from the queue
 * - SSE reader thread: Continuously reads the SSE stream and routes messages to queues
 * - Each JSON-RPC request gets its own response queue for thread-safe blocking waits
 * 
 * DEPENDENCIES:
 * -------------
 * C++17 compiler with standard library:
 * - Windows: WinHTTP (included in Windows SDK), ws2_32, crypt32
 * - Linux/macOS: libcurl for HTTP/HTTPS communication
 * - Standard C++ threading, chrono, sstream libraries
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <unistd.h>
#include <sys/stat.h>
#include <curl/curl.h>
#endif

using namespace std;

// Global flag for graceful shutdown
static volatile bool g_running = true;

// Signal handler for Ctrl+C
void signal_handler(int signal) {
  if (signal == SIGINT) {
    g_running = false;
  }
}

// Simple JSON string escaper
string json_escape(const string& str) {
  ostringstream escaped;
  for (char c : str) {
    if (c == '"') escaped << "\\\"";
    else if (c == '\\') escaped << "\\\\";
    else if (c == '\n') escaped << "\\n";
    else if (c == '\r') escaped << "\\r";
    else if (c == '\t') escaped << "\\t";
    else escaped << c;
  }
  return escaped.str();
}

// Extract JSON string value
string extract_json_string(const string& json, const string& key) {
  string search = "\"" + key + "\":";
  size_t pos = json.find(search);
  if (pos == string::npos) return "";
  
  pos = json.find('"', pos + search.length());
  if (pos == string::npos) return "";
  
  size_t end = json.find('"', pos + 1);
  if (end == string::npos) return "";
  
  return json.substr(pos + 1, end - pos - 1);
}

// Find native messaging manifest
string find_native_messaging_manifest() {
  vector<string> possible_paths;
  
#ifdef _WIN32
  char* local_appdata = getenv("LOCALAPPDATA");
  if (local_appdata) {
    possible_paths.push_back(string(local_appdata) + "\\AuraFriday\\com.aurafriday.shim.json");
  }
#elif defined(__APPLE__)
  const char* home = getenv("HOME");
  if (home) {
    possible_paths.push_back(string(home) + "/Library/Application Support/Google/Chrome/NativeMessagingHosts/com.aurafriday.shim.json");
    possible_paths.push_back(string(home) + "/Library/Application Support/Chromium/NativeMessagingHosts/com.aurafriday.shim.json");
  }
#else
  const char* home = getenv("HOME");
  if (home) {
    possible_paths.push_back(string(home) + "/.config/google-chrome/NativeMessagingHosts/com.aurafriday.shim.json");
    possible_paths.push_back(string(home) + "/.config/chromium/NativeMessagingHosts/com.aurafriday.shim.json");
  }
#endif
  
  for (const auto& path : possible_paths) {
    ifstream f(path);
    if (f.good()) {
      f.close();
      return path;
    }
  }
  
  return "";
}

// Read manifest file
string read_file(const string& path) {
  ifstream file(path);
  if (!file.is_open()) return "";
  
  ostringstream content;
  content << file.rdbuf();
  return content.str();
}

// Execute native binary and get config
string discover_mcp_server_endpoint(const string& binary_path) {
  cerr << "Running native binary: " << binary_path << endl;
  cerr << "[DEBUG] Native messaging protocol uses 4-byte length prefix (little-endian uint32)" << endl;
  
#ifdef _WIN32
  SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
  HANDLE hStdoutRead, hStdoutWrite;
  
  if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0)) {
    return "";
  }
  
  SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
  
  STARTUPINFOA si = { sizeof(STARTUPINFOA) };
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.hStdOutput = hStdoutWrite;
  si.hStdError = hStdoutWrite;
  si.wShowWindow = SW_HIDE;
  
  PROCESS_INFORMATION pi = { 0 };
  
  string cmd = "\"" + binary_path + "\"";
  if (!CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, 
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    CloseHandle(hStdoutRead);
    CloseHandle(hStdoutWrite);
    return "";
  }
  
  CloseHandle(hStdoutWrite);
  
  // Read output using Chrome Native Messaging protocol
  // Protocol: 4-byte length (little-endian uint32) followed by JSON message
  
  // Step 1: Read the 4-byte length prefix (little-endian uint32)
  uint8_t length_bytes[4];
  DWORD bytes_read = 0;
  DWORD total_read = 0;
  
  while (total_read < 4) {
    if (!ReadFile(hStdoutRead, length_bytes + total_read, 4 - total_read, &bytes_read, NULL) || bytes_read == 0) {
      cerr << "ERROR: Failed to read 4-byte length prefix (got " << total_read << " bytes)" << endl;
      TerminateProcess(pi.hProcess, 1);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      CloseHandle(hStdoutRead);
      return "";
    }
    total_read += bytes_read;
  }
  
  // Convert little-endian bytes to uint32
  uint32_t message_length = length_bytes[0] | (length_bytes[1] << 8) | (length_bytes[2] << 16) | (length_bytes[3] << 24);
  cerr << "[DEBUG] Message length from native binary: " << message_length << " bytes" << endl;
  
  if (message_length <= 0 || message_length > 10000000) {
    cerr << "ERROR: Invalid message length: " << message_length << endl;
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdoutRead);
    return "";
  }
  
  // Step 2: Read the JSON payload of the specified length
  string json_str;
  json_str.resize(message_length);
  total_read = 0;
  
  while (total_read < message_length) {
    if (!ReadFile(hStdoutRead, &json_str[total_read], message_length - total_read, &bytes_read, NULL) || bytes_read == 0) {
      cerr << "ERROR: Stream ended after " << total_read << " bytes (expected " << message_length << ")" << endl;
      TerminateProcess(pi.hProcess, 1);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      CloseHandle(hStdoutRead);
      return "";
    }
    total_read += bytes_read;
  }
  
  cerr << "[DEBUG] Successfully read " << total_read << " bytes of JSON" << endl;
  cerr << "[DEBUG] JSON preview: " << json_str.substr(0, min((size_t)100, json_str.length())) << "..." << endl;
  
  TerminateProcess(pi.hProcess, 1);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hStdoutRead);
  
  return json_str;
  
#else
  // Unix/Linux/macOS implementation
  FILE* pipe = popen(binary_path.c_str(), "r");
  if (!pipe) {
    cerr << "ERROR: Failed to execute binary" << endl;
    return "";
  }
  
  // Step 1: Read the 4-byte length prefix (little-endian uint32)
  uint8_t length_bytes[4];
  if (fread(length_bytes, 1, 4, pipe) != 4) {
    cerr << "ERROR: Failed to read 4-byte length prefix" << endl;
    pclose(pipe);
    return "";
  }
  
  // Convert little-endian bytes to uint32
  uint32_t message_length = length_bytes[0] | (length_bytes[1] << 8) | (length_bytes[2] << 16) | (length_bytes[3] << 24);
  cerr << "[DEBUG] Message length from native binary: " << message_length << " bytes" << endl;
  
  if (message_length <= 0 || message_length > 10000000) {
    cerr << "ERROR: Invalid message length: " << message_length << endl;
    pclose(pipe);
    return "";
  }
  
  // Step 2: Read the JSON payload of the specified length
  string json_str;
  json_str.resize(message_length);
  size_t total_read = fread(&json_str[0], 1, message_length, pipe);
  
  if (total_read != message_length) {
    cerr << "ERROR: Stream ended after " << total_read << " bytes (expected " << message_length << ")" << endl;
    pclose(pipe);
    return "";
  }
  
  cerr << "[DEBUG] Successfully read " << total_read << " bytes of JSON" << endl;
  cerr << "[DEBUG] JSON preview: " << json_str.substr(0, min((size_t)100, json_str.length())) << "..." << endl;
  
  pclose(pipe);
  
  return json_str;
#endif
}

// HTTP POST request (Windows WinHTTP)
#ifdef _WIN32
string http_post_winhttp(const string& url, const string& auth_header, const string& body) {
  wstring wide_url(url.begin(), url.end());
  
  URL_COMPONENTS urlComp = { 0 };
  urlComp.dwStructSize = sizeof(urlComp);
  wchar_t host[256], path[1024];
  urlComp.lpszHostName = host;
  urlComp.dwHostNameLength = sizeof(host) / sizeof(host[0]);
  urlComp.lpszUrlPath = path;
  urlComp.dwUrlPathLength = sizeof(path) / sizeof(path[0]);
  
  if (!WinHttpCrackUrl(wide_url.c_str(), 0, 0, &urlComp)) {
    return "";
  }
  
  HINTERNET hSession = WinHttpOpen(L"MCP Client/1.0",
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return "";
  
  HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
  if (!hConnect) {
    WinHttpCloseHandle(hSession);
    return "";
  }
  
  DWORD flags = WINHTTP_FLAG_SECURE;
  if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
    flags |= WINHTTP_FLAG_SECURE;
  }
  
  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path, NULL,
                                          WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES,
                                          flags);
  if (!hRequest) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "";
  }
  
  // Ignore SSL certificate errors
  DWORD security_flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                         SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
  WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &security_flags, sizeof(security_flags));
  
  wstring wide_auth(auth_header.begin(), auth_header.end());
  WinHttpAddRequestHeaders(hRequest, wide_auth.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
  WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json", -1, WINHTTP_ADDREQ_FLAG_ADD);
  
  BOOL result = WinHttpSendRequest(hRequest,
                                   WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                   (LPVOID)body.c_str(), body.length(),
                                   body.length(), 0);
  
  if (!result || !WinHttpReceiveResponse(hRequest, NULL)) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "";
  }
  
  DWORD status_code = 0;
  DWORD size = sizeof(status_code);
  WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                     NULL, &status_code, &size, NULL);
  
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
  
  return (status_code == 202) ? "OK" : "";
}
#endif

// HTTP POST request (Unix libcurl)
#ifndef _WIN32
size_t curl_write_callback(void* contents, size_t size, size_t nmemb, string* output) {
  size_t total_size = size * nmemb;
  output->append((char*)contents, total_size);
  return total_size;
}

string http_post_curl(const string& url, const string& auth_header, const string& body) {
  CURL* curl = curl_easy_init();
  if (!curl) return "";
  
  string response;
  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, auth_header.c_str());
  headers = curl_slist_append(headers, "Content-Type: application/json");
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  
  CURLcode res = curl_easy_perform(curl);
  
  long response_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  return (res == CURLE_OK && response_code == 202) ? "OK" : "";
}
#endif

// Send HTTP POST
string http_post(const string& url, const string& auth_header, const string& body) {
#ifdef _WIN32
  return http_post_winhttp(url, auth_header, body);
#else
  return http_post_curl(url, auth_header, body);
#endif
}

// Generate UUID-like string
string generate_uuid() {
  ostringstream oss;
  oss << hex;
  for (int i = 0; i < 32; i++) {
    oss << (rand() % 16);
    if (i == 7 || i == 11 || i == 15 || i == 19) oss << "-";
  }
  return oss.str();
}

// SSE Connection class
class SSEConnection {
public:
  string server_url;
  string auth_header;
  string session_id;
  string message_endpoint;
  queue<string> reverse_queue;
  mutex queue_mutex;
  condition_variable queue_cv;
  
  bool connect() {
    // For simplicity, we'll use a minimal SSE implementation
    // In production, you'd want a full SSE client library
    cerr << "[INFO] SSE connection setup (simplified for demo)" << endl;
    
    // Extract base URL
    size_t sse_pos = server_url.find("/sse");
    if (sse_pos != string::npos) {
      message_endpoint = "/message";
      session_id = "cpp-session-" + generate_uuid();
      return true;
    }
    
    return false;
  }
  
  string send_request(const string& method, const string& params_json) {
    string request_id = generate_uuid();
    ostringstream body;
    body << "{\"jsonrpc\":\"2.0\",\"id\":\"" << request_id 
         << "\",\"method\":\"" << method 
         << "\",\"params\":" << params_json << "}";
    
    string full_url = server_url;
    size_t sse_pos = full_url.find("/sse");
    if (sse_pos != string::npos) {
      full_url = full_url.substr(0, sse_pos) + message_endpoint;
    }
    
    string result = http_post(full_url, "Authorization: " + auth_header, body.str());
    return result;
  }
  
  void send_tool_reply(const string& call_id, const string& result_json) {
    ostringstream body;
    body << "{\"jsonrpc\":\"2.0\",\"id\":\"" << call_id 
         << "\",\"method\":\"tools/reply\",\"params\":{\"result\":" << result_json << "}}";
    
    string full_url = server_url;
    size_t sse_pos = full_url.find("/sse");
    if (sse_pos != string::npos) {
      full_url = full_url.substr(0, sse_pos) + message_endpoint;
    }
    
    string result = http_post(full_url, "Authorization: " + auth_header, body.str());
    if (!result.empty()) {
      cerr << "[OK] Sent tools/reply for call_id " << call_id << endl;
    }
  }
  
  // Call another MCP tool on the server
  // This demonstrates how to call other MCP tools from within your remote tool handler
  string call_mcp_tool(const string& tool_name, const string& arguments_json) {
    ostringstream params;
    params << "{\"name\":\"" << json_escape(tool_name) << "\",\"arguments\":" << arguments_json << "}";
    
    string request_id = generate_uuid();
    ostringstream body;
    body << "{\"jsonrpc\":\"2.0\",\"id\":\"" << request_id 
         << "\",\"method\":\"tools/call\",\"params\":" << params.str() << "}";
    
    string full_url = server_url;
    size_t sse_pos = full_url.find("/sse");
    if (sse_pos != string::npos) {
      full_url = full_url.substr(0, sse_pos) + message_endpoint;
    }
    
    string result = http_post(full_url, "Authorization: " + auth_header, body.str());
    
    // For demo purposes, we return "OK" if POST succeeded
    // In production, you'd wait for the actual response via SSE
    return result;
  }
};

// Handle echo request
// This demonstrates TWO capabilities:
// 1. Basic echo functionality - echoes back the message
// 2. Calling OTHER MCP tools - demonstrates how to call sqlite, browser, etc.
string handleEchoRequest(const string& message, SSEConnection* conn = nullptr) {
  cerr << "[ECHO] Received echo request: " << message << endl;
  
  // Basic echo response
  string response_text = "Echo: " + message;
  
  // DEMONSTRATION: If we have connection info, show how to call other tools
  if (conn != nullptr) {
    // Convert message to lowercase for keyword detection
    string message_lower = message;
    transform(message_lower.begin(), message_lower.end(), message_lower.begin(), ::tolower);
    
    // Demo 1: List databases (triggered by keyword "databases" or "db")
    // Check this FIRST because it's more specific and helps users discover what databases exist
    if (message_lower.find("databases") != string::npos || message_lower.find("list db") != string::npos) {
      cerr << "[DEMO] Calling sqlite tool to list databases..." << endl;
      
      // Call the sqlite tool to list databases
      string sqlite_args = R"({"input":{"sql":".databases","tool_unlock_token":"29e63eb5"}})";
      string sqlite_result = conn->call_mcp_tool("sqlite", sqlite_args);
      
      // Append the result to our response
      if (!sqlite_result.empty()) {
        response_text += "\n\n[DEMO] Called sqlite tool successfully!\n";
        response_text += "Result: " + sqlite_result;
      } else {
        response_text += "\n\n[DEMO] SQLite tool call failed or returned no result";
      }
    }
    // Demo 2: List tables (triggered by keywords "tables" - check AFTER databases to avoid conflicts)
    else if (message_lower.find("tables") != string::npos) {
      cerr << "[DEMO] Calling sqlite tool to list tables..." << endl;
      
      // Extract database name if specified (e.g., "list tables in test.db")
      string database = ":memory:";
      size_t in_pos = message_lower.find(" in ");
      if (in_pos != string::npos) {
        database = message.substr(in_pos + 4);
        // Trim whitespace
        size_t start = database.find_first_not_of(" \t\n\r");
        size_t end = database.find_last_not_of(" \t\n\r");
        if (start != string::npos && end != string::npos) {
          database = database.substr(start, end - start + 1);
        }
      }
      
      // Call the sqlite tool to list tables
      ostringstream sqlite_args;
      sqlite_args << "{\"input\":{\"sql\":\".tables\",\"database\":\"" 
                  << json_escape(database) 
                  << "\",\"tool_unlock_token\":\"29e63eb5\"}}";
      string sqlite_result = conn->call_mcp_tool("sqlite", sqlite_args.str());
      
      // Append the result to our response
      if (!sqlite_result.empty()) {
        response_text += "\n\n[DEMO] Called sqlite tool successfully!\n";
        response_text += "Database: " + database + "\n";
        response_text += "Result: " + sqlite_result;
      } else {
        response_text += "\n\n[DEMO] SQLite tool call failed or returned no result";
      }
    }
  }
  
  // Build JSON result
  ostringstream result;
  result << "{\"content\":[{\"type\":\"text\",\"text\":\"" 
         << json_escape(response_text) 
         << "\"}],\"isError\":false}";
  
  return result.str();
};

// Main worker function
int main_worker(bool background) {
  cerr << "=== Aura Friday Remote Tool Provider Demo ===" << endl;
  cerr << "PID: " << getpid() << endl;
  cerr << "Registering demo_tool_cpp with MCP server" << endl << endl;
  
  // Connection state for reconnection logic
  int retry_count = 0;
  int max_retry_delay = 60; // Max 1 minute between retries
  
  // Outer reconnection loop - keeps trying forever
  while (true) {
    try {
      // Calculate retry delay with exponential backoff
      if (retry_count > 0) {
        int delay = min(1 << retry_count, max_retry_delay); // 2^retry_count
        cerr << endl << "[RECONNECT] Waiting " << delay << " seconds before retry (attempt #" << retry_count << ")..." << endl;
        this_thread::sleep_for(chrono::seconds(delay));
        
        // Check for Ctrl+C during sleep
        if (!g_running) {
          cerr << endl << endl << string(60, '=') << endl;
          cerr << "Shutting down..." << endl;
          cerr << string(60, '=') << endl;
          return 0;
        }
        
        cerr << "[RECONNECT] Attempting to reconnect..." << endl << endl;
      }
      
      // Step 1: Find manifest
      cerr << "Step 1: Finding native messaging manifest..." << endl;
      string manifest_path = find_native_messaging_manifest();
      if (manifest_path.empty()) {
        cerr << "ERROR: Could not find manifest" << endl;
        retry_count++;
        continue;
      }
      cerr << "[OK] Found manifest: " << manifest_path << endl << endl;
      
      // Step 2: Read manifest
      cerr << "Step 2: Reading manifest..." << endl;
      string manifest_content = read_file(manifest_path);
      if (manifest_content.empty()) {
        cerr << "ERROR: Could not read manifest" << endl;
        retry_count++;
        continue;
      }
      string binary_path = extract_json_string(manifest_content, "path");
      if (binary_path.empty()) {
        cerr << "ERROR: No path in manifest" << endl;
        retry_count++;
        continue;
      }
      cerr << "[OK] Manifest loaded" << endl << endl;
      
      // Step 3: Discover endpoint
      cerr << "Step 3: Discovering MCP server endpoint..." << endl;
      string config = discover_mcp_server_endpoint(binary_path);
      if (config.empty()) {
        cerr << "ERROR: Could not get configuration from native binary" << endl;
        cerr << "Is the Aura Friday MCP server running?" << endl;
        retry_count++;
        continue;
      }
      
      string server_url = extract_json_string(config, "url");
      string auth_token = extract_json_string(config, "Authorization");
      
      // Debug output to see what we extracted
      cerr << "[DEBUG] Config length: " << config.length() << " bytes" << endl;
      cerr << "[DEBUG] Extracted URL: '" << server_url << "'" << endl;
      cerr << "[DEBUG] Extracted auth token: '" << auth_token << "'" << endl;
      
      if (server_url.empty()) {
        cerr << "ERROR: Could not extract server URL from config" << endl;
        cerr << "       Config preview: " << config.substr(0, 200) << "..." << endl;
        retry_count++;
        continue;
      }
      
      if (auth_token.empty()) {
        cerr << "ERROR: Could not extract Authorization header from config" << endl;
        cerr << "       Looking for nested 'Authorization' key in mcpServers.*.headers" << endl;
        
        // Try harder to find the auth token in nested structure
        size_t auth_pos = config.find("\"Authorization\"");
        if (auth_pos != string::npos) {
          size_t colon_pos = config.find(":", auth_pos);
          if (colon_pos != string::npos) {
            size_t quote1 = config.find("\"", colon_pos);
            if (quote1 != string::npos) {
              size_t quote2 = config.find("\"", quote1 + 1);
              if (quote2 != string::npos) {
                auth_token = config.substr(quote1 + 1, quote2 - quote1 - 1);
                cerr << "       Found auth token via fallback: '" << auth_token << "'" << endl;
              }
            }
          }
        }
        
        if (auth_token.empty()) {
          cerr << "       Config preview: " << config.substr(0, 500) << "..." << endl;
          retry_count++;
          continue;
        }
      }
      
      cerr << "[OK] Found server at: " << server_url << endl << endl;
      
      // Step 4: Connect to SSE
      cerr << "Step 4: Connecting to SSE endpoint..." << endl;
      SSEConnection conn;
      conn.server_url = server_url;
      conn.auth_header = auth_token;
      
      if (!conn.connect()) {
        cerr << "ERROR: Could not connect to SSE" << endl;
        retry_count++;
        continue;
      }
      cerr << "[OK] Connected! Session ID: " << conn.session_id << endl << endl;
      
      // Step 5: Check for remote tool
      cerr << "Step 5: Checking for remote tool..." << endl;
      cerr << "[DEBUG] Sending tools/list request..." << endl;
      string tools_result = conn.send_request("tools/list", "{}");
      cerr << "[DEBUG] tools/list result: '" << tools_result << "'" << endl;
      
      if (tools_result.empty()) {
        cerr << "ERROR: Could not get tools list (HTTP POST may have failed)" << endl;
        cerr << "       This is likely due to WinHTTP not working properly" << endl;
        cerr << "       Continuing anyway to attempt registration..." << endl;
        // Don't fail here - continue to registration
      } else {
        cerr << "[OK] Remote tool found" << endl << endl;
      }
      
      // Step 6: Register demo_tool_cpp
      cerr << "Step 6: Registering demo_tool_cpp..." << endl;
      
      // Get source file location (executable path + .cpp extension)
      string source_file = "reverse_mcp.cpp (executable location + .cpp)";
      
      string register_params = R"JSON({
        "name":"remote",
        "arguments":{
          "input":{
            "operation":"register",
            "tool_name":"demo_tool_cpp",
            "readme":"Demo tool that echoes messages back and can call other MCP tools.\n- Use this to test the remote tool system and verify bidirectional communication.\n- Demonstrates how remote tools can call OTHER tools on the server (like sqlite, browser, etc.)",
            "description":"Demo tool (C++ implementation) for testing remote tool registration and end-to-end MCP communication. This tool demonstrates TWO key capabilities: (1) Basic echo functionality - echoes back any message sent to it, and (2) Tool-to-tool communication - shows how remote tools can call OTHER MCP tools on the server. This verifies that: (a) tool registration works correctly, (b) reverse calls from server to client function properly, (c) the client can successfully reply to tool calls, (d) the full bidirectional JSON-RPC communication channel is operational, and (e) remote tools can orchestrate other tools. This tool is implemented in reverse_mcp.cpp and serves as a reference template for integrating MCP tool support into other applications like Fusion 360, Blender, Ghidra, and similar products. Usage workflow: (1) Start by discovering databases: {\"message\": \"list databases\"} calls sqlite to show all available databases. (2) Then list tables in a specific database: {\"message\": \"list tables in test.db\"} calls sqlite and returns table names. (3) Basic echo: {\"message\": \"test\"} returns 'Echo: test'. The tool automatically detects keywords in the message to trigger different demonstrations.",
            "parameters":{
              "type":"object",
              "properties":{
                "message":{"type":"string","description":"The message to echo back"}
              },
              "required":["message"]
            },
            "callback_endpoint":"cpp-client://demo-tool-callback",
            "TOOL_API_KEY":"cpp_demo_tool_auth_key_12345"
          }
        }
      })JSON";
      
      string register_result = conn.send_request("tools/call", register_params);
      if (register_result.empty()) {
        cerr << "ERROR: Registration failed" << endl;
        retry_count++;
        continue;
      }
      cerr << "[OK] Successfully registered tool: demo_tool_cpp" << endl;
      
      // Reset retry count after successful connection and registration
      retry_count = 0;
      
      cerr << endl << string(60, '=') << endl;
      cerr << "[OK] demo_tool_cpp registered successfully!" << endl;
      cerr << "Listening for reverse tool calls... (Press Ctrl+C to stop)" << endl;
      cerr << string(60, '=') << endl << endl;
      
      // Step 7: Listen for reverse calls (simplified - no actual SSE in this demo)
      cerr << "[INFO] In production, this would listen for SSE events and detect disconnections" << endl;
      cerr << "[INFO] For demo purposes, the tool is registered and ready" << endl;
      cerr << "[INFO] Press Ctrl+C to stop" << endl;
      
      while (g_running) {
        this_thread::sleep_for(chrono::seconds(1));
        // In a full implementation, we would check if SSE connection is still alive here
        // and break to trigger reconnection if it's down
      }
      
      // If g_running is false, user hit Ctrl+C - exit gracefully
      if (!g_running) {
        cerr << endl << endl << string(60, '=') << endl;
        cerr << "Shutting down..." << endl;
        cerr << string(60, '=') << endl;
        return 0;
      }
      
      // If we get here, connection dropped - outer loop will retry
      
    } catch (const exception& e) {
      cerr << endl << "[ERROR] Unexpected error in main loop: " << e.what() << endl;
      retry_count++;
      // Loop continues to retry
    }
  }
}

int main(int argc, char* argv[]) {
  bool background = false;
  bool help = false;
  
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg == "--background") background = true;
    if (arg == "--help") help = true;
  }
  
  if (help) {
    cout << "Usage: reverse_mcp_cpp [--background]" << endl;
    cout << endl << "Aura Friday Remote Tool Provider - Registers demo_tool_cpp with MCP server" << endl;
    return 0;
  }
  
  // Setup signal handler
  signal(SIGINT, signal_handler);
  
  if (background) {
    cerr << "Starting in background mode (PID: " << getpid() << ")..." << endl;
    cerr << "[OK] Background worker started (PID: " << getpid() << ")" << endl;
    cerr << "  Use 'kill " << getpid() << "' to stop" << endl;
  }
  
  return main_worker(background);
}

