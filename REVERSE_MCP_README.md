# Reverse MCP - Multi-Language Remote Tool Provider

This directory contains **6 complete implementations** of the same MCP remote tool provider in different programming languages. Each implementation demonstrates how to:

1. Connect to the MCP server via native messaging discovery
2. Register a language-specific demo tool with the server
3. Listen for reverse tool calls from the server
4. Process "echo" requests and send back replies
5. **Call OTHER MCP tools** from within the handler (sqlite, browser, etc.)
6. Run continuously until stopped with Ctrl+C
7. automatic connection (and re-connection!) as required

## Quick Reference

| Language | File | Tool Name | Build Required | Run Command | Status |
|----------|------|-----------|----------------|-------------|--------|
| **Python** | `reverse_mcp.py` | `demo_tool_python` | ❌ No | `python reverse_mcp.py` | ✅ Production Ready |
| **JavaScript** | `reverse_mcp.js` | `demo_tool_nodejs` | ❌ No | `node reverse_mcp.js` | ✅ Production Ready |
| **Go** | `reverse_mcp.go` | `demo_tool_go` | ✅ Yes | `reverse_mcp_go.exe` | ✅ Production Ready |
| **Java** | `ReverseMcp.java` | `demo_tool_java` | ✅ Yes | `java ReverseMcp` | ✅ Production Ready |
| **Kotlin** | `reverse_mcp.kt` | `demo_tool_kotlin` | ✅ Yes | `java -jar reverse_mcp_kt.jar` | ✅ Production Ready |
| **Perl** | `reverse_mcp.pl` | `demo_tool_perl` | ❌ No | `perl reverse_mcp.pl` | ⚠️ Proof of Concept* |
| **C++** | `reverse_mcp.cpp` | `demo_tool_cpp` | ✅ Yes | `reverse_mcp_cpp.exe` | ⚠️ Proof of Concept* |

*Both C++ and Perl versions demonstrate core concepts (native messaging, JSON parsing, process spawning) but use simplified HTTP/SSE handling. For production applications:
- **C++**: Use [cpp-httplib](https://github.com/yhirose/cpp-httplib), [Poco](https://pocoproject.org/), or [Boost.Beast](https://www.boost.org/doc/libs/1_83_0/libs/beast/doc/html/index.html)
- **Perl**: Use proper IPC/threading with [AnyEvent](https://metacpan.org/pod/AnyEvent) or [Mojo::UserAgent](https://docs.mojolicious.org/Mojo/UserAgent) for SSE

---

## Python Version

**File:** `reverse_mcp.py`  
**Tool Name:** `demo_tool_python`

### Requirements
- Python 3.7+ (tested with 3.11+)
- Standard library only (no pip install needed)

### Run
```bash
python reverse_mcp.py [--background]
python reverse_mcp.py --help
```

### Use Case
Perfect for:
- Fusion 360 add-ons
- Blender plugins
- Data science integrations
- Quick prototyping

---

## JavaScript/Node.js Version

**File:** `reverse_mcp.js`  
**Tool Name:** `demo_tool_nodejs`

### Requirements
- Node.js 14+ (tested with Node.js 18+)
- Standard library only (no npm install needed)

### Run
```bash
node reverse_mcp.js [--background]
node reverse_mcp.js --help
```

### Use Case
Perfect for:
- Chrome extensions
- VS Code extensions
- Electron apps
- Web automation

---

## Go Version

**File:** `reverse_mcp.go`  
**Executable:** `reverse_mcp_go.exe` (Windows) or `reverse_mcp_go` (Mac/Linux)  
**Tool Name:** `demo_tool_go`

### Requirements
- Go 1.16+ (tested with Go 1.21+)
- Standard library only (no go get needed)

### Build
```bash
# Windows
go build -a -ldflags="-s -w" -o reverse_mcp_go.exe reverse_mcp.go

# Mac/Linux
go build -a -ldflags="-s -w" -o reverse_mcp_go reverse_mcp.go
```

### Run
```bash
# Windows
reverse_mcp_go.exe [--background]

# Mac/Linux
./reverse_mcp_go [--background]
```

### Use Case
Perfect for:
- System utilities
- Native applications
- Kubernetes operators
- High-performance services

---

## Java Version

**File:** `ReverseMcp.java`  
**Class File:** `ReverseMcp.class`  
**Tool Name:** `demo_tool_java`

### Requirements
- Java JDK 11+ (tested with JDK 25)
- Standard library only (no Maven/Gradle needed)

### Build
```bash
javac ReverseMcp.java
```

### Run
```bash
java ReverseMcp [--background]
java ReverseMcp --help
```

### Use Case
Perfect for:
- Ghidra plugins
- IntelliJ IDEA plugins
- Android tools
- Enterprise applications

---

## Kotlin Version

**File:** `reverse_mcp.kt`  
**Tool Name:** `demo_tool_kotlin`  
**Documentation:** `reverse_mcp_kt_README.md` (comprehensive guide)

### Requirements
- Java JDK 11+ (tested with JDK 25)
- Gradle 9+ (optional - can use kotlinc directly)
- Standard library only (no external dependencies)

### Build
```bash
# Option 1: Using Gradle (recommended)
./reverse_mcp_kt_build.sh    # Linux/macOS
reverse_mcp_kt_build.bat      # Windows

# Option 2: Using kotlinc directly
kotlinc reverse_mcp.kt -include-runtime -d reverse_mcp_kt.jar
```

### Run
```bash
java -jar reverse_mcp_kt_build/libs/reverse_mcp_kt.jar [--background]
java -jar reverse_mcp_kt_build/libs/reverse_mcp_kt.jar --help
```

### Use Case
Perfect for:
- Android applications
- IntelliJ IDEA plugins
- Spring Boot services
- Any JVM-based application
- Developers who prefer modern, expressive syntax

### Files
All Kotlin files use the `reverse_mcp_kt_*` prefix for easy identification:
- `reverse_mcp.kt` - Main implementation
- `reverse_mcp_kt_README.md` - Complete documentation
- `reverse_mcp_kt_INTEGRATION_GUIDE.md` - Integration guide for developers
- `reverse_mcp_kt_build.gradle.kts` - Gradle build configuration
- `reverse_mcp_kt_build.sh` / `.bat` - Build scripts
- See `reverse_mcp_kt_README.md` for full file listing

---

## Perl Version

**File:** `reverse_mcp.pl`  
**Tool Name:** `demo_tool_perl`

### Requirements
- Perl 5.10+ (tested with Perl 5.38)
- Required CPAN modules:
  ```bash
  cpan JSON LWP::UserAgent HTTP::Request File::HomeDir IO::Socket::SSL
  ```

### Run
```bash
perl reverse_mcp.pl [--background]
perl reverse_mcp.pl --help
```

### Use Case
Perfect for:
- Legacy system integration
- Text processing tools
- System administration
- Bio-informatics (BioPerl)

---

## Common Features

All implementations include:

✅ **Platform Detection** - Auto-detects Windows, macOS, and Linux  
✅ **Native Messaging** - Discovers server via Chrome's native messaging manifest  
✅ **SSE Connection** - Long-lived event stream for real-time communication  
✅ **Dual-Channel** - POST for requests, SSE for responses  
✅ **Tool Registration** - Registers with the server's `remote` tool system  
✅ **Echo Operation** - Simple request/response for testing  
✅ **Tool Orchestration** - Can call other MCP tools (sqlite, browser, user, etc.)  
✅ **Background Mode** - Can run as a background service  
✅ **Cross-Platform** - Works on Windows, macOS, and Linux  
✅ **No Dependencies** - Uses only standard libraries (except Perl CPAN modules)

---

## Testing All Versions

You can test any tool using the MCP server's built-in demo tool:

```python
# In a Python script or Cursor AI chat with MCP tools available
import mcp

# Test Python version - Basic echo
result = client.call_tool("demo_tool_python", {"message": "Hello from Python!"})
# Returns: {"content": [{"type": "text", "text": "Echo: Hello from Python!"}]}

# Test Python version - Tool orchestration (calls sqlite)
result = client.call_tool("demo_tool_python", {"message": "list databases"})
# Returns: Echo + sqlite database listing

# Test Java version - Basic echo
result = client.call_tool("demo_tool_java", {"message": "Hello from Java!"})

# Test Java version - Tool orchestration (calls sqlite)
result = client.call_tool("demo_tool_java", {"message": "list tables in test.db"})
# Returns: Echo + sqlite table listing

# Test other versions
result = client.call_tool("demo_tool_nodejs", {"message": "Hello from Node.js!"})
result = client.call_tool("demo_tool_go", {"message": "Hello from Go!"})
result = client.call_tool("demo_tool_perl", {"message": "Hello from Perl!"})

# Note: C++ version is proof-of-concept and requires additional HTTP library for full functionality
```

### Demo Tool Message Patterns

All demo tools support these message patterns:

- **`"Hello World"`** - Basic echo response
- **`"list databases"`** or **`"list db"`** - Calls sqlite to list all databases (START HERE)
- **`"list tables"`** - Calls sqlite to list tables in :memory: database
- **`"list tables in <database>"`** - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")

This demonstrates both basic functionality AND tool-to-tool orchestration!

---

## C++ Version (Proof of Concept)

**File:** `reverse_mcp.cpp`  
**Tool Name:** `demo_tool_cpp`  
**Status:** ⚠️ Demonstrates concepts, not production-ready

### Build
```bash
# Windows
g++ -std=c++17 -o reverse_mcp_cpp.exe reverse_mcp.cpp -lws2_32 -lwinhttp

# Linux/macOS
g++ -std=c++17 -o reverse_mcp_cpp reverse_mcp.cpp -lcurl
```

### Run
```bash
./reverse_mcp_cpp [--background]
./reverse_mcp_cpp --help
```

### Current Status
The C++ version successfully demonstrates:
- ✅ Platform-aware native messaging manifest discovery
- ✅ Native binary execution and JSON config extraction
- ✅ Brace-depth JSON parsing (no external libs)
- ✅ Process spawning and I/O handling
- ⚠️ Simplified HTTP/SSE implementation (needs proper library)

### For Production C++ Applications
We recommend using a mature HTTP client library:
- **[cpp-httplib](https://github.com/yhirose/cpp-httplib)** - Header-only, easy to integrate
- **[Poco](https://pocoproject.org/)** - Comprehensive networking library
- **[Boost.Beast](https://www.boost.org/doc/libs/1_83_0/libs/beast/doc/html/index.html)** - Part of Boost, WebSocket + HTTP

The current C++ implementation is provided as a **template** showing the core logic you'd need to implement in your C++ application. Replace the simplified HTTP/SSE code with proper library calls for production use.

### Use Case
Reference implementation for:
- Game engine plugins (Unity, Unreal)
- CAD software integrations (AutoCAD, SolidWorks)
- 3D animation tools (Blender, Maya)
- High-performance applications

---

## Architecture

All implementations follow the same architecture:

```
┌─────────────────────────────────────────────────────┐
│  Your Application (Python/JS/Go/Java/Perl)          │
│                                                     │
│  1. Find native messaging manifest                  │
│  2. Run native binary to get server config          │
│  3. Connect to SSE endpoint                         │
│  4. Register demo_tool_<language> with server       │
│  5. Listen for reverse calls via SSE                │
│  6. Process requests and send replies               │
│  7. Call OTHER tools (sqlite, browser, etc.)  ◄─┐   │
└─────────────────────────────────────────────────┼───┘
                          ▲                       │
                          │ SSE + HTTPS           │
                          ▼                       │
┌─────────────────────────────────────────────────┼───┐
│  Aura Friday MCP Server                         │   │
│                                                 │   │
│  - Manages tool registry                        │   │
│  - Routes calls to registered tools             │   │
│  - Forwards responses back to clients           │   │
│  - Provides sqlite, browser, user tools ◄───────┘   │
└─────────────────────────────────────────────────────┘
                          ▲
                          │ SSE + HTTPS
                          ▼
┌─────────────────────────────────────────────────────┐
│  AI Agent (Cursor, Claude, etc.)                    │
│                                                     │
│  - Discovers available tools                        │
│  - Calls demo_tool_<language>                       │
│  - Receives echo + orchestrated tool results        │
└─────────────────────────────────────────────────────┘
```

---

## Extending This Code

To create your own remote tool:

1. **Choose your language** - Pick any of the 5 implementations
2. **Change tool name** - Update from `demo_tool_<lang>` to your tool name
3. **Update parameters** - Define your tool's input schema
4. **Implement handler** - Replace `handleEchoRequest()` with your logic
5. **Use `callMcpTool()`** - Optionally orchestrate other MCP tools (sqlite, browser, user, etc.)
6. **Test** - Run your tool and verify it registers correctly

### Example: Calling Other Tools

All implementations include a `callMcpTool()` function (or equivalent) that lets your tool call OTHER MCP tools:

```python
# Python example
sqlite_result = call_mcp_tool(
    sse_connection,
    server_url,
    auth_header,
    "sqlite",
    {"input": {"sql": ".tables", "database": "test.db", "tool_unlock_token": "29e63eb5"}}
)
```

```java
// Java example
Map<String, Object> sqliteResult = callMcpTool(
    conn,
    "sqlite",
    "{\"input\":{\"sql\":\".tables\",\"database\":\"test.db\",\"tool_unlock_token\":\"29e63eb5\"}}"
);
```

This enables complex workflows like:
- Read data from your app → Query database → Show results to user
- Get browser tabs → Process with your tool → Store in database
- Fetch user input → Process in your app → Display results via popup

---

## Production Considerations

When deploying these tools in production:

1. **Security**
   - Validate all input parameters
   - Use proper authentication (TOOL_API_KEY)
   - Limit tool permissions

2. **Reliability**
   - Implement reconnection logic
   - Handle network failures gracefully
   - Log errors for debugging

3. **Performance**
   - Use connection pooling
   - Implement request timeouts
   - Monitor resource usage

4. **Packaging**
   - Bundle all dependencies
   - Create installers for end users
   - Document system requirements

---

## License

Copyright (c) 2025 Christopher Nathan Drake. All rights reserved.  
SPDX-License-Identifier: Proprietary

---

## Support

For questions or issues:
- Check the individual file headers for detailed build/run instructions
- Review the inline code comments for implementation details
- Test with `--help` flag to see usage information

**Happy coding! 🚀**

<small><sup>Copyright © 2025 Christopher Drake. All rights reserved. "signature": "HᎠҮРΝƦОꓓеwgŪɅȢОʋʈÞЈHВ𐐕ᏮkΥRᎠƌþΑ𝟫Ꭰ4rKHꓴᎠųυĸuՕ0ųƌAƱmKWΤᎪ𝟢ԝⲘzƽЈⲟƟx𝟛𝟩𝙰ꓓtAƊЕµꙅƟƧ𝖠UᒿBꓣꓜ7ᎠЅɌȠ𝟧СƙΑꓐ×hυßᴜɌꓑ𝘈ÐНƊvսıQрƻƋɡ". "signdate":"2025-12-02T06:27:45.873Z" </sup></small>
