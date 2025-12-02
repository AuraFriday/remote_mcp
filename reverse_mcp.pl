#!/usr/bin/env perl
#
# CAUTION - THIS CODE DOES NOT WORK - PERL+TLS+THREADS PROBLEM - NEEDS A REWRITE TO BE NONBLOCKING SINGLETHREAD
# File: reverse_mcp.pl
# Project: Aura Friday MCP-Link Server - Remote Tool Provider Demo
# Component: Registers a demo tool with the MCP server and handles reverse calls
# Author: Christopher Nathan Drake (cnd)
# Created: 2025-11-03
# Last Modified: 2025-11-03 by cnd (Perl port from Python)
# SPDX-License-Identifier: Apache-2.0
# Copyright: (c) 2025 Christopher Nathan Drake. All rights reserved.
# "signature": "nυȢɌοƦk𝟛𝟣LΤw4qPBΗΡꓴƋⲘᏮƟⲔµwƦGbßеXĐȷԝ𝟪ƖƵZᴛƼȢуСlЗHոʋƊƬI𝐴уȜ7ꓰӠTΒɗуᴍМϜϨрꓑꓓ2pⲔѵŪdĐᴍųуᗅ1𝟙𝟛IⲞᑕᑕʋƻɌΗᎻᖴĸΒ𝟧৭𝙰ꓗꓴꓐSvƐďɅꓖꓪᏴ",
# "signdate": "2025-12-02T06:27:49.554Z",
#
# VERSION: 2025.11.03.001 - Remote Tool Provider Demo (Perl)
#
# This script demonstrates how to register a tool with the MCP server using the remote tool system.
# It acts as a tool provider that:
# 1. Connects to the MCP server via native messaging discovery
# 2. Registers a "demo_tool_perl" with the server
# 3. Listens for reverse tool calls from the server
# 4. Processes "echo" requests and sends back replies
# 5. Runs continuously until stopped with Ctrl+C
#
# BUILD/RUN INSTRUCTIONS:
#   No build required - Perl is interpreted
#   
#   Requirements:
#     - Perl 5.10+ (tested with Perl 5.38)
#     - Required CPAN modules:
#       cpan JSON LWP::UserAgent HTTP::Request File::HomeDir IO::Socket::SSL
#   
#   Run:
#     perl reverse_mcp.pl [--background]
#     perl reverse_mcp.pl --help
#
# Usage: perl reverse_mcp.pl [--background]

=head1 HOW TO USE THIS CODE

This code is a complete, self-contained reference template for integrating MCP (Model Context Protocol)
tool support into other applications like Fusion 360, Blender, Ghidra, and similar products.

=head2 HOW THIS WORKS

=over 4

=item 1. You create a new add-on or extension or plugin or similar for the application you want to let an AI control on your behalf. (hereafter addIn)

=item 2. This template gives your new addIn the facility to discover the correct endpoint where a local controller MCP server is running, and then:

=item 3. lets your addIn register itself with that server as a new tool, which any AI using that MCP server can then discover and access and use.

=item 4. and finally, this template processes incoming tool requests from the AI, which you implement in your addIn, and this template sends the results of those tool-calls back to the AI.

=item 5. BONUS: Your addIn can also CALL OTHER MCP tools on the server (sqlite, browser, user, etc.) - making it possible to orchestrate complex workflows!

=item *. The server installer can be found at https://github.com/aurafriday/mcp-link-server/releases

=back

=head2 ARCHITECTURE OVERVIEW

=over 4

=item 1. Native Messaging Discovery: Locates the MCP server by finding the Chrome native messaging manifest (com.aurafriday.shim.json) which is installed by the Aura Friday MCP-Link server.

=item 2. Server Configuration: Executes the native messaging binary to get the server's SSE endpoint URL and authentication token. The binary is a long-running stdio service, so we terminate it after reading the initial JSON config.

=item 3. SSE Connection: Establishes a persistent Server-Sent Events (SSE) connection to receive messages from the server. This runs in a background thread and routes incoming messages to the appropriate handlers.

=item 4. Dual-Channel Communication:
POST requests (via HTTP/HTTPS) to send JSON-RPC commands to the server
SSE stream (long-lived GET connection) to receive JSON-RPC responses and reverse tool calls

=item 5. Tool Registration: Uses the server's "remote" tool to register your custom tool with these components:
tool_name: Unique identifier for your tool
readme: Minimal summary for the AI (when to use this tool)
description: Comprehensive documentation for the AI (what it does, how to use it, examples)
parameters: JSON Schema defining the tool's input parameters
callback_endpoint: Identifier for routing reverse calls back to your client
TOOL_API_KEY: Authentication key for your tool

=item 6. Reverse Call Handling: After registration, your tool appears in the server's tool list. When an AI agent calls your tool, the server sends a "reverse" message via the SSE stream containing:
tool: Your tool's name
call_id: Unique ID for this invocation (used to send the reply)
input: The parameters passed by the AI

=item 7. Reply Mechanism: Your code processes the request and sends a "tools/reply" message back to the server with the call_id and result. The server forwards this to the AI.

=back

=head2 INTEGRATION STEPS

=over 4

=item 1. Copy this file to your project

=item 2. Modify the tool registration section (search for "demo_tool_perl"):
Change tool_name to your tool's unique identifier
Update description and readme to explain your tool's purpose
Define your tool's parameters schema
Set a unique callback_endpoint and TOOL_API_KEY

=item 3. Replace the handle_echo_request() function with your tool's actual logic:
Extract parameters from the input_data
Perform your tool's operations (file I/O, API calls, computations, etc.)
OPTIONALLY: Call other MCP tools using call_mcp_tool() function
Return a result JSON string with "content" array and "isError" boolean

=item 4. (Optional) Use call_mcp_tool() to orchestrate other MCP tools:
Your handler receives $conn (SSE connection) parameter
Use call_mcp_tool() to call sqlite, browser, user, or any other MCP tool
Example: my $result = call_mcp_tool($conn, "sqlite", {"input" => {"sql" => ".tables", "tool_unlock_token" => "..."}});
This enables complex workflows like: read data from app → query database → show results to user

=item 5. Run your tool provider script:
It will auto-discover the server, register your tool, and listen for calls
The tool remains registered as long as the script is running
Press Ctrl+C to cleanly shut down

=back

=head2 RESULT FORMAT

All tool results must follow this structure:

  {
    "content": [
      {"type": "text", "text": "Your response text here"},
      {"type": "image", "data": "base64...", "mimeType": "image/png"}  # optional
    ],
    "isError": false  # or true if an error occurred
  }

=head2 THREADING MODEL

=over 4

=item * Main thread: Handles tool registration and processes reverse calls from the queue

=item * SSE reader thread: Continuously reads the SSE stream and routes messages to queues

=item * Thread::Queue: Thread-safe queues for message passing between threads

=back

=head2 DEPENDENCIES

Perl 5.10+ with CPAN modules:

=over 4

=item * JSON: JSON parsing and encoding

=item * LWP::UserAgent: HTTP/HTTPS communication

=item * IO::Socket::SSL: SSL/TLS support

=item * threads, Thread::Queue: Multi-threading support

=item * File::HomeDir, File::Spec: Cross-platform file operations

=back

=head2 ERROR HANDLING & RECONNECTION

=over 4

=item * SSL certificate verification is disabled (self-signed certs are common in local servers)

=item * Native binary timeout is 5 seconds (increase if needed)

=item * SSE response timeout is 10 seconds per request (configurable)

=item * All errors are logged to stderr for debugging

=item * Automatic reconnection with exponential backoff if SSE connection drops:
Retry delays: 2s, 4s, 8s, 16s, 32s, 60s (max), 60s, 60s...
After successful reconnection, retry counter resets
Tool is automatically re-registered after reconnection
Retries forever until manually stopped (Ctrl+C)

=back

=cut

use strict;
use warnings;
use JSON qw(decode_json encode_json);
use LWP::UserAgent;
use HTTP::Request;
use File::Spec;
use File::HomeDir;
use IO::Socket::SSL;
use threads;
use threads::shared;
use Thread::Queue;
use LWP::ConnCache;
use Time::HiRes qw(sleep time);

my $DOCO = <<'END_DOCO';
This script demonstrates how to register a tool with the MCP server using the remote tool system.
It acts as a tool provider that:
1. Connects to the MCP server via native messaging discovery
2. Registers a "demo_tool_perl" with the server
3. Listens for reverse tool calls from the server
4. Processes "echo" requests and sends back replies
5. Demonstrates calling OTHER MCP tools (sqlite, browser, etc.) from within the handler
6. Runs continuously until stopped with Ctrl+C

The demo tool responds to these message patterns:
- "list databases" or "list db" - Calls sqlite to list all databases (START HERE to discover what's available)
- "list tables" - Calls sqlite to list tables in :memory: database
- "list tables in <database>" - Calls sqlite to list tables in specific database (e.g., "list tables in test.db")
- Any other message - Simple echo response

Usage: perl reverse_mcp.pl [--background]
END_DOCO

# Find native messaging manifest
sub find_native_messaging_manifest {
    my $os = $^O;
    my $home = File::HomeDir->my_home;
    my @possible_paths;
    
    if ($os eq 'MSWin32') {
        my $local_appdata = $ENV{LOCALAPPDATA} || File::Spec->catdir($home, 'AppData', 'Local');
        push @possible_paths, File::Spec->catfile($local_appdata, 'AuraFriday', 'com.aurafriday.shim.json');
    } elsif ($os eq 'darwin') {
        push @possible_paths, (
            File::Spec->catfile($home, 'Library/Application Support/Google/Chrome/NativeMessagingHosts/com.aurafriday.shim.json'),
            File::Spec->catfile($home, 'Library/Application Support/Chromium/NativeMessagingHosts/com.aurafriday.shim.json')
        );
    } else {
        push @possible_paths, (
            File::Spec->catfile($home, '.config/google-chrome/NativeMessagingHosts/com.aurafriday.shim.json'),
            File::Spec->catfile($home, '.config/chromium/NativeMessagingHosts/com.aurafriday.shim.json')
        );
    }
    
    for my $path (@possible_paths) {
        return $path if -e $path;
    }
    
    return undef;
}

# Read manifest
sub read_manifest {
    my ($path) = @_;
    open my $fh, '<', $path or return undef;
    local $/;
    my $content = <$fh>;
    close $fh;
    return decode_json($content);
}

# Discover MCP server endpoint
sub discover_mcp_server_endpoint {
    my ($manifest) = @_;
    my $binary_path = $manifest->{path};
    
    unless ($binary_path && -e $binary_path) {
        warn "ERROR: Binary not found: $binary_path\n";
        return undef;
    }
    
    warn "Running native binary: $binary_path\n";
    warn "[DEBUG] Native messaging protocol uses 4-byte length prefix (little-endian uint32)\n";
    
    # Start the process
    my $pid = open my $fh, '-|', $binary_path;
    unless ($pid) {
        warn "ERROR: Failed to run binary\n";
        return undef;
    }
    
    binmode($fh);  # Binary mode for reading bytes
    
    # Read output using Chrome Native Messaging protocol
    # Protocol: 4-byte length (little-endian uint32) followed by JSON message
    my $config;
    
    eval {
        local $SIG{ALRM} = sub { die "timeout\n" };
        alarm 5;
        
        # Step 1: Read the 4-byte length prefix (little-endian uint32)
        my $length_bytes = '';
        my $bytes_read = read($fh, $length_bytes, 4);
        
        unless ($bytes_read == 4) {
            warn "ERROR: Failed to read 4-byte length prefix (got $bytes_read bytes)\n";
            alarm 0;
            kill 'TERM', $pid if $pid;
            close $fh;
            return undef;
        }
        
        # Convert little-endian bytes to uint32
        my $message_length = unpack('V', $length_bytes);  # 'V' is little-endian uint32
        warn "[DEBUG] Message length from native binary: $message_length bytes\n";
        
        if ($message_length <= 0 || $message_length > 10_000_000) {
            warn "ERROR: Invalid message length: $message_length\n";
            alarm 0;
            kill 'TERM', $pid if $pid;
            close $fh;
            return undef;
        }
        
        # Step 2: Read the JSON payload of the specified length
        my $json_bytes = '';
        my $total_read = 0;
        
        while ($total_read < $message_length) {
            my $chunk;
            my $n = read($fh, $chunk, $message_length - $total_read);
            last unless $n;
            $json_bytes .= $chunk;
            $total_read += $n;
        }
        
        unless ($total_read == $message_length) {
            warn "ERROR: Stream ended after $total_read bytes (expected $message_length)\n";
            alarm 0;
            kill 'TERM', $pid if $pid;
            close $fh;
            return undef;
        }
        
        # Step 3: Decode and parse the JSON
        warn "[DEBUG] Successfully read $total_read bytes of JSON\n";
        my $preview = substr($json_bytes, 0, 100);
        warn "[DEBUG] JSON preview: $preview...\n";
        
        $config = decode_json($json_bytes);
        
        alarm 0;
    };
    
    if ($@) {
        warn "ERROR: $@\n" unless $@ eq "timeout\n";
    }
    
    kill 'TERM', $pid if $pid;
    close $fh;
    
    return $config;
}

# Extract server info
sub extract_server_info {
    my ($config) = @_;
    my $mcp_servers = $config->{mcpServers} || {};
    my ($first_server) = values %$mcp_servers;
    
    return (
        $first_server->{url},
        $first_server->{headers}{Authorization}
    );
}

# SSE Connection class
package SSEConnection;

use threads;
use threads::shared;
use Thread::Queue;
use LWP::UserAgent;
use LWP::ConnCache;

sub new {
  my ($class, $server_url, $auth_header) = @_;

  my $ua = LWP::UserAgent->new(
    agent      => 'AuraFriday-Perl/1.0',
    keep_alive => 1,
  );
  $ua->conn_cache(LWP::ConnCache->new());
  $ua->timeout(30);
  $ua->ssl_opts(
    verify_hostname => 0,
    SSL_verify_mode => 0,
  );

  my $reverse_queue  = Thread::Queue->new();
  my $response_queue = Thread::Queue->new();

  my $session_id :shared       = '';
  my $message_endpoint :shared = '';
  my $stop_flag :shared        = 0;
  my $is_alive :shared         = 0;

  my $self = bless {
    server_url           => $server_url,
    auth_header          => $auth_header,
    ua                   => $ua,
    reverse_queue        => $reverse_queue,
    response_queue       => $response_queue,
    session_id_ref       => \$session_id,
    message_endpoint_ref => \$message_endpoint,
    stop_flag_ref        => \$stop_flag,
    is_alive_ref         => \$is_alive,
    sse_thread           => undef,
  }, $class;

  return $self;
}

sub session_id {
  my ($self) = @_;
  return ${ $self->{session_id_ref} };
}

sub message_endpoint {
  my ($self) = @_;
  return ${ $self->{message_endpoint_ref} };
}

sub stop_flag {
  my ($self) = @_;
  return ${ $self->{stop_flag_ref} };
}

sub is_alive {
  my ($self) = @_;
  return ${ $self->{is_alive_ref} };
}

sub reverse_queue {
  my ($self) = @_;
  return $self->{reverse_queue};
}

sub connect {
  my ($self) = @_;

  ${ $self->{stop_flag_ref} } = 0;
  ${ $self->{is_alive_ref} } = 1;

  $self->{sse_thread} = threads->create(
    \&_sse_reader,
    $self->{server_url},
    $self->{auth_header},
    $self->{reverse_queue},
    $self->{response_queue},
    $self->{session_id_ref},
    $self->{message_endpoint_ref},
    $self->{stop_flag_ref},
    $self->{is_alive_ref},
  );

  my $deadline = time + 10;
  while (time < $deadline) {
    last if ${ $self->{session_id_ref} } && ${ $self->{message_endpoint_ref} };
    sleep 0.1;
  }

  die "No session ID received\n"       unless ${ $self->{session_id_ref} };
  die "No message endpoint received\n" unless ${ $self->{message_endpoint_ref} };

  return 1;
}

sub _sse_reader {
  my (
    $server_url,
    $auth_header,
    $reverse_queue,
    $response_queue,
    $session_ref,
    $endpoint_ref,
    $stop_ref,
    $is_alive_ref,
  ) = @_;

  my $ua = LWP::UserAgent->new(
    agent      => 'AuraFriday-Perl/1.0',
    keep_alive => 1,
  );
  $ua->conn_cache(LWP::ConnCache->new());
  $ua->timeout(0);
  $ua->ssl_opts(
    verify_hostname => 0,
    SSL_verify_mode => 0,
  );

  while (!${$stop_ref}) {
    my $req = HTTP::Request->new(GET => $server_url);
    $req->header('Accept' => 'text/event-stream');
    $req->header('Cache-Control' => 'no-cache');
    $req->header('Authorization' => $auth_header);

    my $buffer     = '';
    my $event_type = '';
    my @data_lines = ();

    my $response = eval {
      $ua->simple_request(
        $req,
        sub {
          my ($chunk, $res) = @_;
          $buffer .= $chunk;

          while ($buffer =~ s/^(.*?\n)//) {
            my $line = $1;
            $line =~ s/\r?\n$//;

            if ($line eq '') {
              my $data = join("\n", @data_lines);
              @data_lines = ();

              if ($event_type eq 'endpoint') {
                ${$endpoint_ref} = $data if $data ne '';
                if ($data =~ /session_id=([^&]+)/) {
                  ${$session_ref} = $1;
                }
              } elsif ($data ne '') {
                my $decoded = eval { decode_json($data) };
                if (!$@ && ref $decoded eq 'HASH') {
                  warn "[DEBUG] SSE message: " . encode_json($decoded) . "\n";
                  if (exists $decoded->{reverse}) {
                    $reverse_queue->enqueue($decoded);
                  } elsif (exists $decoded->{id}) {
                    $response_queue->enqueue($decoded);
                  }
                }
              }

              $event_type = '';
              next;
            }

            next if $line =~ /^:/;

            if ($line =~ /^event:\s*(.+)/) {
              $event_type = $1;
              next;
            }

            if ($line =~ /^data:\s*(.*)$/) {
              push @data_lines, $1;
            }
          }

          return ${$stop_ref} ? 0 : length($chunk);
        }
      );
    };

    last if ${$stop_ref};

    if ($@) {
      warn "[DEBUG] SSE reader error: $@\n";
    } elsif (!$response->is_success) {
      warn "[DEBUG] SSE connection closed: " . $response->status_line . "\n";
    }

    sleep 0.5 unless ${$stop_ref};
  }
  
  # Mark connection as not alive when thread ends
  ${$is_alive_ref} = 0;
}

sub send_request {
  my ($self, $method, $params_json) = @_;

  my $request_id = sprintf("%d-%04d", int(time * 1000), int(rand(10000)));

  my $message_endpoint = ${ $self->{message_endpoint_ref} } || '';
  my $url;
  if ($message_endpoint =~ m{^https?://}) {
    $url = $message_endpoint;
  } else {
    $url = $self->{server_url};
    $url =~ s{/sse.*}{};
    $url .= $message_endpoint;
  }

  my $body = sprintf(
    '{"jsonrpc":"2.0","id":"%s","method":"%s","params":%s}',
    $request_id, $method, $params_json
  );

  my $req = HTTP::Request->new(POST => $url);
  $req->header('Content-Type' => 'application/json');
  $req->header('Authorization' => $self->{auth_header});
  $req->content($body);

  my $response = $self->{ua}->request($req);

  unless ($response->code == 202) {
    warn "[DEBUG] HTTP POST failed for $method: " . $response->status_line . "\n";
    return undef;
  }

  my $deadline = time + 10;
  my @stash;
  while (time < $deadline) {
    my $remaining = $deadline - time;
    $remaining = 0.2 if $remaining > 0.2;
    my $msg = $self->{response_queue}->dequeue_timed($remaining);
    next unless defined $msg;
    if ($msg->{id} && $msg->{id} eq $request_id) {
      for my $pending (@stash) {
        $self->{response_queue}->enqueue($pending);
      }
      return $msg;
    }
    push @stash, $msg;
  }

  for my $pending (@stash) {
    $self->{response_queue}->enqueue($pending);
  }

  warn "[DEBUG] Timeout waiting for response to request $request_id ($method)\n";
  return undef;
}

sub send_tool_reply {
  my ($self, $call_id, $result_json) = @_;

  my $message_endpoint = ${ $self->{message_endpoint_ref} } || '';
  my $url;
  if ($message_endpoint =~ m{^https?://}) {
    $url = $message_endpoint;
  } else {
    $url = $self->{server_url};
    $url =~ s{/sse.*}{};
    $url .= $message_endpoint;
  }

  my $body = sprintf(
    '{"jsonrpc":"2.0","id":"%s","method":"tools/reply","params":{"result":%s}}',
    $call_id, $result_json
  );

  my $req = HTTP::Request->new(POST => $url);
  $req->header('Content-Type' => 'application/json');
  $req->header('Authorization' => $self->{auth_header});
  $req->content($body);

  my $response = $self->{ua}->request($req);
  if ($response->code == 202) {
    warn "[OK] Sent tools/reply for call_id $call_id\n";
    return 1;
  }

  warn "[DEBUG] tools/reply failed for call_id $call_id: " . $response->status_line . "\n";
  return 0;
}

sub close {
  my ($self) = @_;
  return unless $self->{stop_flag_ref};

  ${ $self->{stop_flag_ref} } = 1;

  if ($self->{sse_thread} && threads->tid() != $self->{sse_thread}->tid()) {
    eval { $self->{sse_thread}->join(); };
    $self->{sse_thread} = undef;
  }
}

sub DESTROY {
  my ($self) = @_;
  $self->close();
}

1;

package main;

# Call another MCP tool
sub call_mcp_tool {
  my ($conn, $tool_name, $arguments) = @_;
  
  # Call another MCP tool on the server.
  # 
  # This function demonstrates how to call other MCP tools from within your remote tool handler.
  # It uses the existing SSE connection and JSON-RPC infrastructure to make tool calls.
  # 
  # Args:
  #   $conn: Active SSE connection object
  #   $tool_name: Name of the tool to call (e.g., "sqlite", "browser", "user")
  #   $arguments: Hash reference with arguments to pass to the tool
  # 
  # Returns:
  #   Hash reference with JSON-RPC response, or undef on error
  # 
  # Example:
  #   # Call sqlite tool to list tables
  #   my $result = call_mcp_tool(
  #     $conn,
  #     "sqlite",
  #     {"input" => {"sql" => ".tables", "tool_unlock_token" => "29e63eb5"}}
  #   );
  #   
  #   # Call browser tool to list tabs
  #   my $result = call_mcp_tool(
  #     $conn,
  #     "browser",
  #     {"input" => {"operation" => "list_tabs", "tool_unlock_token" => "e5076d"}}
  #   );
  
  my $tool_call_params = {
    name => $tool_name,
    arguments => $arguments
  };
  
  my $params_json = encode_json($tool_call_params);
  
  # Use longer timeout for tool calls (30 seconds)
  my $request_id = sprintf("%d-%04d", int(time * 1000), int(rand(10000)));
  
  my $message_endpoint = ${ $conn->{message_endpoint_ref} } || '';
  my $url;
  if ($message_endpoint =~ m{^https?://}) {
    $url = $message_endpoint;
  } else {
    $url = $conn->{server_url};
    $url =~ s{/sse.*}{};
    $url .= $message_endpoint;
  }
  
  my $body = sprintf(
    '{"jsonrpc":"2.0","id":"%s","method":"tools/call","params":%s}',
    $request_id, $params_json
  );
  
  my $req = HTTP::Request->new(POST => $url);
  $req->header('Content-Type' => 'application/json');
  $req->header('Authorization' => $conn->{auth_header});
  $req->content($body);
  
  my $response = $conn->{ua}->request($req);
  
  unless ($response->code == 202) {
    warn "[DEBUG] HTTP POST failed for tools/call: " . $response->status_line . "\n";
    return undef;
  }
  
  # Wait for response with 30 second timeout
  my $deadline = time + 30;
  my @stash;
  while (time < $deadline) {
    my $remaining = $deadline - time;
    $remaining = 0.2 if $remaining > 0.2;
    my $msg = $conn->{response_queue}->dequeue_timed($remaining);
    next unless defined $msg;
    if ($msg->{id} && $msg->{id} eq $request_id) {
      for my $pending (@stash) {
        $conn->{response_queue}->enqueue($pending);
      }
      return $msg;
    }
    push @stash, $msg;
  }
  
  for my $pending (@stash) {
    $conn->{response_queue}->enqueue($pending);
  }
  
  warn "[DEBUG] Timeout waiting for tool call response to $tool_name\n";
  return undef;
}

# Register demo tool
sub register_demo_tool {
  my ($conn) = @_;
  
  warn "Registering demo_tool_perl with MCP server...\n";
  
  # Get source file location
  my $source_file = $0;  # $0 contains the script's filename
  
  # Build params with dynamic source file path
  # Note: readme = MINIMAL (when to use), description = COMPREHENSIVE (how to use, examples)
  my $description = "Demo tool (Perl implementation) for testing remote tool registration and end-to-end MCP communication. This tool demonstrates TWO key capabilities: (1) Basic echo functionality - echoes back any message sent to it, and (2) Tool-to-tool communication - shows how remote tools can call OTHER MCP tools on the server. This verifies that: (a) tool registration works correctly, (b) reverse calls from server to client function properly, (c) the client can successfully reply to tool calls, (d) the full bidirectional JSON-RPC communication channel is operational, and (e) remote tools can orchestrate other tools. This tool is implemented in $source_file and serves as a reference template for integrating MCP tool support into other applications like Fusion 360, Blender, Ghidra, and similar products. Usage workflow: (1) Start by discovering databases: {\\\"message\\\": \\\"list databases\\\"} calls sqlite to show all available databases. (2) Then list tables in a specific database: {\\\"message\\\": \\\"list tables in test.db\\\"} calls sqlite and returns table names. (3) Basic echo: {\\\"message\\\": \\\"test\\\"} returns 'Echo: test'. The tool automatically detects keywords in the message to trigger different demonstrations.";
  
  my $params_hash = {
    name => 'remote',
    arguments => {
      input => {
        operation => 'register',
        tool_name => 'demo_tool_perl',
        readme => "Demo tool that echoes messages back and can call other MCP tools.\\n- Use this to test the remote tool system and verify bidirectional communication.\\n- Demonstrates how remote tools can call OTHER tools on the server (like sqlite, browser, etc.)",
        description => $description,
        parameters => {
          type => 'object',
          properties => {
            message => {
              type => 'string',
              description => 'The message to echo back'
            }
          },
          required => ['message']
        },
        callback_endpoint => 'perl-client://demo-tool-callback',
        TOOL_API_KEY => 'perl_demo_tool_auth_key_12345'
      }
    }
  };
  
  my $params = encode_json($params_hash);
  
  my $response = $conn->send_request('tools/call', $params);

  unless ($response) {
    warn "ERROR: Registration failed\n";
    return 0;
  }

  if ($response->{error}) {
    my $message = $response->{error}{message} // 'unknown error';
    warn "ERROR: Registration failed - $message\n";
    return 0;
  }

  my $result   = $response->{result} || {};
  my $content  = $result->{content};
  my $text_out = '';
  if (ref $content eq 'ARRAY' && @$content) {
    $text_out = $content->[0]{text} // '';
  } elsif (ref $result eq 'HASH' && defined $result->{text}) {
    $text_out = $result->{text};
  }

  if ($text_out =~ /Successfully registered tool/) {
    warn "[OK] Successfully registered tool: demo_tool_perl\n";
    return 1;
  }

  warn "ERROR: Unexpected registration response\n";
  warn "[DEBUG] Response: " . encode_json($response) . "\n";
  return 0;
}

# Handle echo request
sub handle_echo_request {
  my ($input_data, $conn) = @_;
  
  # Handle an echo request from the server.
  # 
  # This demonstrates TWO capabilities:
  # 1. Basic echo functionality - echoes back the message
  # 2. Calling OTHER MCP tools - demonstrates how to call sqlite, browser, etc.
  # 
  # Args:
  #   $input_data: The tool call data from the reverse message
  #   $conn: Optional SSE connection for making tool calls
  # 
  # Returns:
  #   JSON string with result
  # 
  # Example usage from AI:
  #   # Basic echo
  #   {"message": "Hello World"}
  #   
  #   # Step 1: Discover what databases exist
  #   {"message": "list databases"}
  #   
  #   # Step 2: List tables in a specific database
  #   {"message": "list tables in test.db"}
  #   
  #   # Or use default :memory: database
  #   {"message": "list tables"}

  my $message = '(no message provided)';

  if (ref $input_data eq 'HASH') {
    if (ref($input_data->{arguments}) eq 'HASH' && defined $input_data->{arguments}{message}) {
      $message = $input_data->{arguments}{message};
    } elsif (ref($input_data->{params}) eq 'HASH' && defined $input_data->{params}{message}) {
      $message = $input_data->{params}{message};
    }
  }

  warn "[ECHO] Received echo request: $message\n";

  # Basic echo response
  my $response_text = "Echo: $message";
  
  # DEMONSTRATION: If we have connection info, show how to call other tools
  if (defined $conn) {
    my $message_lower = lc($message);
    
    # Demo 1: List databases (triggered by keyword "databases" or "db")
    # Check this FIRST because it's more specific and helps users discover what databases exist
    if ($message_lower =~ /databases/ || $message_lower =~ /list db/) {
      warn "[DEMO] Calling sqlite tool to list databases...\n";
      
      # Call the sqlite tool to list databases
      my $sqlite_result = call_mcp_tool(
        $conn,
        "sqlite",
        {"input" => {"sql" => ".databases", "tool_unlock_token" => "29e63eb5"}}
      );
      
      # Append the result to our response
      if ($sqlite_result && $sqlite_result->{result}) {
        $response_text .= "\n\n[DEMO] Called sqlite tool successfully!\n";
        $response_text .= "Result:\n" . encode_json($sqlite_result->{result});
      } else {
        $response_text .= "\n\n[DEMO] SQLite tool call failed or returned no result:\n" . encode_json($sqlite_result);
      }
    }
    # Demo 2: List tables (triggered by keywords "tables" - check AFTER databases to avoid conflicts)
    elsif ($message_lower =~ /tables/) {
      warn "[DEMO] Calling sqlite tool to list tables...\n";
      
      # Extract database name if specified (e.g., "list tables in test.db")
      my $database = ":memory:";
      if ($message =~ / in (.+)$/i) {
        $database = $1;
        $database =~ s/^\s+|\s+$//g;  # trim whitespace
      }
      
      # Call the sqlite tool to list tables
      my $sqlite_result = call_mcp_tool(
        $conn,
        "sqlite",
        {"input" => {"sql" => ".tables", "database" => $database, "tool_unlock_token" => "29e63eb5"}}
      );
      
      # Append the result to our response
      if ($sqlite_result && $sqlite_result->{result}) {
        $response_text .= "\n\n[DEMO] Called sqlite tool successfully!\n";
        $response_text .= "Database: $database\n";
        $response_text .= "Result:\n" . encode_json($sqlite_result->{result});
      } else {
        $response_text .= "\n\n[DEMO] SQLite tool call failed or returned no result:\n" . encode_json($sqlite_result);
      }
    }
  }

  my $result_payload = {
    content => [
      {
        type => 'text',
        text => $response_text,
      }
    ],
    isError => JSON::false,
  };

  return encode_json($result_payload);
}

# Main worker
sub main_worker {
  warn "=== Aura Friday Remote Tool Provider Demo ===\n";
  warn "PID: $$\n";
  warn "Registering demo_tool with MCP server\n\n";

  # Connection state for reconnection logic
  my $retry_count = 0;
  my $max_retry_delay = 60; # Max 1 minute between retries
  
  # Outer reconnection loop - keeps trying forever
  while (1) {
    eval {
      # Calculate retry delay with exponential backoff
      if ($retry_count > 0) {
        my $delay = (2 ** $retry_count);
        $delay = $max_retry_delay if $delay > $max_retry_delay;
        warn "\n[RECONNECT] Waiting $delay seconds before retry (attempt #$retry_count)...\n";
        sleep $delay;
        warn "[RECONNECT] Attempting to reconnect...\n\n";
      }
      
      # Step 1
      warn "Step 1: Finding native messaging manifest...\n";
      my $manifest_path = find_native_messaging_manifest();
      unless ($manifest_path) {
        warn "ERROR: Could not find manifest\n";
        $retry_count++;
        return; # Return from eval to retry
      }
      warn "[OK] Found manifest: $manifest_path\n\n";

      # Step 2
      warn "Step 2: Reading manifest...\n";
      my $manifest = read_manifest($manifest_path);
      unless ($manifest) {
        warn "ERROR: Could not read manifest\n";
        $retry_count++;
        return;
      }
      warn "[OK] Manifest loaded\n\n";

      # Step 3
      warn "Step 3: Discovering MCP server endpoint...\n";
      my $config = discover_mcp_server_endpoint($manifest);
      unless ($config) {
        warn "ERROR: Could not discover endpoint\n";
        warn "Is the Aura Friday MCP server running?\n";
        $retry_count++;
        return;
      }

      my ($server_url, $auth_header) = extract_server_info($config);
      unless ($server_url) {
        warn "ERROR: Could not extract server URL\n";
        $retry_count++;
        return;
      }
      warn "[OK] Found server at: $server_url\n\n";

      # Step 4
      warn "Step 4: Connecting to SSE endpoint...\n";
      my $conn = SSEConnection->new($server_url, $auth_header);
      my $connected = eval { $conn->connect(); 1 };
      unless ($connected) {
        my $err = $@ || 'unknown error';
        warn "ERROR: Could not connect: $err";
        $conn->close();
        $retry_count++;
        return;
      }
      warn "[OK] Connected! Session ID: " . ${ $conn->{session_id_ref} } . "\n\n";

      # Step 5
      warn "Step 5: Checking for remote tool...\n";
      my $tools_response = $conn->send_request('tools/list', '{}');
      unless ($tools_response && $tools_response->{result}) {
        warn "ERROR: Unable to obtain tools list\n";
        $conn->close();
        $retry_count++;
        return;
      }

      my $tools = $tools_response->{result}{tools};
      $tools = [] unless ref $tools eq 'ARRAY';
      my $has_remote = 0;
      for my $tool (@$tools) {
        next unless ref $tool eq 'HASH';
        if (($tool->{name} // '') eq 'remote') {
          $has_remote = 1;
          last;
        }
      }

      unless ($has_remote) {
        warn "ERROR: No remote tool found\n";
        $conn->close();
        $retry_count++;
        return;
      }
      warn "[OK] Remote tool found\n\n";

      # Step 6
      warn "Step 6: Registering demo_tool_perl...\n";
      unless (register_demo_tool($conn)) {
        $conn->close();
        $retry_count++;
        return;
      }
      
      # Reset retry count after successful connection and registration
      $retry_count = 0;

      warn "\n" . ("=" x 60) . "\n";
      warn "[OK] demo_tool_perl registered successfully!\n";
      warn "Listening for reverse tool calls... (Press Ctrl+C to stop)\n";
      warn(("=" x 60) . "\n\n");

      $SIG{INT} = sub {
        warn "\n\n" . ("=" x 60) . "\n";
        warn "Shutting down...\n";
        warn(("=" x 60) . "\n");
        $conn->close();
        exit 0;
      };

      while (1) {
        # Check if SSE connection is still alive
        unless ($conn->is_alive()) {
          warn "\n[WARN] SSE connection lost - reconnecting...\n";
          $conn->close();
          $retry_count = 1; # Start with first retry delay
          return; # Break inner loop to trigger reconnection
        }
        
        my $msg = $conn->reverse_queue->dequeue_timed(1);
        next unless defined $msg;

        my $reverse = $msg->{reverse};
        next unless ref $reverse eq 'HASH';

        my $tool_name = $reverse->{tool} // 'unknown';
        my $call_id   = $reverse->{call_id};
        my $input     = $reverse->{input} || {};

        warn "\n[CALL] Reverse call received:\n";
        warn "       Tool: $tool_name\n";
        warn "       Call ID: " . ($call_id // '(none)') . "\n";

        unless ($call_id) {
          warn "[WARN] Missing call_id in reverse message\n";
          next;
        }

        # Handle the echo request (pass connection so it can call other tools)
        my $result_json = handle_echo_request($input, $conn);
        $conn->send_tool_reply($call_id, $result_json);
      }
      
      # If we get here, connection dropped - outer loop will retry
    };
    
    # Handle Ctrl+C in outer loop too
    if ($@ && $@ =~ /interrupt/i) {
      warn "\n\n" . ("=" x 60) . "\n";
      warn "Shutting down...\n";
      warn(("=" x 60) . "\n");
      return 0;
    }
    
    # Log any unexpected errors and retry
    if ($@) {
      warn "\n[ERROR] Unexpected error in main loop: $@\n";
      $retry_count++;
      # Loop continues to retry
    }
  }
}

# Main entry point
sub main {
    my $background = grep { $_ eq '--background' } @ARGV;
    my $help = grep { $_ eq '--help' } @ARGV;
    
  if ($help) {
    print "Usage: perl reverse_mcp.pl [--background]\n\n";
    print "Aura Friday Remote Tool Provider - Registers demo_tool_perl with MCP server\n\n";
        print $DOCO;
        exit 0;
    }
    
    if ($background) {
        warn "Starting in background mode (PID: $$)...\n";
        warn "[OK] Background worker started (PID: $$)\n";
        warn "  Use 'kill $$' to stop\n";
    }
    
    exit main_worker();
}

main() unless caller;

__END__

