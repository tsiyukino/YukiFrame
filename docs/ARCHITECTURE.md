# Yuki-Frame Architecture

## Design Philosophy

### Everything is a Tool

Yuki-Frame follows a simple, elegant principle: **Everything is a tool.**

```
┌──────────────────────────────────────┐
│        Framework (Hub)               │
│                                      │
│  • Routes events                     │
│  • Manages tool lifecycle            │
│  • Processes commands                │
└──────────────────────────────────────┘
       ▲    ▲    ▲    ▲    ▲
       │    │    │    │    │
   (All use stdin/stdout pipes)
       │    │    │    │    │
    Tool1 Tool2 Tool3 Tool4 Tool5
   Console Monitor Alerter Logger Backup

Everything is equal!
Everything uses pipes!
No special cases!
```

---

## Why This Design?

### Traditional Approach (Asymmetric)

```
Framework
   ├─ Socket API ────> Console (special)
   └─ Pipes ─────────> Tools (regular)

Problems:
❌ Two communication methods
❌ Console is "special"
❌ More complex to implement
❌ Harder to test
❌ Less flexible
```

### Yuki-Frame Approach (Symmetric)

```
Framework
   └─ Pipes ─────────> All Tools (equal)
                       • Console
                       • Monitor
                       • Alerter
                       • Everything!

Benefits:
✅ One communication method
✅ Everything is equal
✅ Simple to implement
✅ Easy to test
✅ Maximum flexibility
```

---

## Communication Pattern

### Event Protocol

**Format:** `TYPE|sender|data\n`

**Examples:**
```
ALERT|monitor|High CPU: 95%
STATUS|backup|Completed
ERROR|database|Connection failed
COMMAND|console|start backup
RESPONSE|framework|Tool started PID 12345
```

### Flow

```
Tool sends event:
   print("ALERT|monitor|High CPU")
   stdout.flush()
      ↓
   Framework receives on tool's stdout
      ↓
   Framework routes to subscribers
      ↓
   Subscribers receive on their stdin
      ↓
   for line in stdin:
       process(line)
```

---

## Console as a Tool

### Traditional Console

```
User types command
     ↓
Console (separate executable)
     ↓
Socket connection
     ↓
Framework
     ↓
Execute command
     ↓
Response
```

**Complexity:** Socket server, connection handling, protocol parsing

### Yuki-Frame Console

```
User types command
     ↓
Console Tool (yuki-console.py)
     ↓
print("COMMAND|console|start backup")
     ↓
Framework (reads from console's stdout)
     ↓
Execute command
     ↓
Framework sends RESPONSE event to console's stdin
     ↓
Console displays
```

**Simplicity:** Just pipes! ✅

---

## Component Overview

### 1. Framework Core

**Responsibilities:**
- Spawn and manage tools
- Route events between tools
- Process COMMAND events from console
- Monitor tool health
- Automatic restart

**Key Files:**
- `src/core/main.c` - Main loop, command handler
- `src/core/tool.c` - Tool lifecycle management
- `src/core/event.c` - Event bus
- `src/core/config.c` - Configuration loading

---

### 2. Tools

**Characteristics:**
- Independent processes
- Communicate via stdin/stdout
- Can be any language
- Isolated from each other
- Managed by framework

**Types:**

**Event Processors:**
```python
# monitor.py
for line in stdin:
    event = parse(line)
    if cpu_high():
        print("ALERT|monitor|High CPU")
```

**Console (Control Interface):**
```python
# yuki-console.py
user_cmd = input("yuki> ")
print(f"COMMAND|console|{user_cmd}")
response = stdin.readline()
print(response)
```

**Both use same pattern!**

---

## Event Routing

### Pub/Sub Model

```
Tool1 publishes:
   print("ALERT|tool1|message")
   
Framework:
   1. Receives from tool1.stdout
   2. Checks subscriptions
   3. Routes to subscribers
   
Tool2, Tool3 (subscribed to ALERT):
   for line in stdin:
       if line.startswith("ALERT"):
           process()
```

### Command-Response Model

```
Console sends:
   print("COMMAND|console|list")
   
Framework:
   1. Receives COMMAND event
   2. Executes command (list tools)
   3. Generates response
   4. Sends to console
   
Console receives:
   response = stdin.readline()
   # "RESPONSE|framework|Tool1 RUNNING..."
```

---

## Why Hub Model?

### Alternatives Considered

**1. Peer-to-Peer (Rejected)**
```
Tool1 ←→ Tool2
  ↓        ↓
Tool3 ←→ Tool4

❌ Complex routing
❌ No central control
❌ Hard to debug
```

**2. Broker Model (Too Complex)**
```
Framework creates sockets
Tools connect directly
Tools communicate peer-to-peer

❌ Overkill for this scale
❌ Tools become complex
❌ Harder to manage
```

**3. Hub Model (Chosen)** ✅
```
   Framework
    ▲  ▲  ▲
    │  │  │
  Tool1 Tool2 Tool3

✅ Simple routing
✅ Central control
✅ Easy debugging
✅ Tools stay simple
```

---

## Scalability

### Current Design

**Good for:**
- 10-100 tools
- 1,000-10,000 events/second
- Single machine
- Simple tools

**Not designed for:**
- 1,000+ tools (use broker model)
- 100,000+ events/sec (use message queue)
- Distributed systems (use network protocol)

**For current use case: Perfect!** ✅

---

## Design Decisions

### 1. Why Pipes?

**Pros:**
- ✅ Universal (works in any language)
- ✅ Simple (just print/read)
- ✅ Standard (POSIX)
- ✅ Testable (echo | tool | cat)
- ✅ No dependencies

**Cons:**
- ❌ Text-based (need parsing)
- ❌ No type safety
- ❌ Async only

**Verdict:** Pros outweigh cons for this use case

---

### 2. Why Not Sockets?

**Could use sockets:**
```
Tool connects to framework socket
Tool sends/receives
```

**Why not:**
- More complex (socket code in every tool)
- Language barrier (not all langs have easy sockets)
- Coupling (tool depends on framework address)
- Testing harder (need framework running)

**Pipes are simpler!** ✅

---

### 3. Why Not Shared Memory?

**Could use shared memory:**
```
Framework creates shared memory
Tools read/write directly
```

**Why not:**
- **Very complex** (mutexes, semaphores, cleanup)
- Platform-specific
- Overkill for event passing
- Debugging nightmare

**Way too complex!** ❌

---

### 4. Why Console as Tool?

**Alternative: Socket API**
```
Framework has socket server
Console connects via socket
```

**Why tool is better:**
- ✅ Consistent (same as other tools)
- ✅ Simpler (no socket code in framework)
- ✅ Testable (just pipes)
- ✅ Flexible (multiple consoles easy)

**Consistency wins!** ✅

---

## Tool Isolation

### Benefits

**Security:**
- Tools can't see each other's memory
- Tools can't kill each other
- Framework controls all interactions

**Reliability:**
- Tool crash doesn't affect framework
- Tool crash doesn't affect other tools
- Framework can restart crashed tools

**Simplicity:**
- Tools don't need to know about each other
- Tools just process events
- Framework handles coordination

---

## Extension Points

### Adding New Tool

1. Write tool (any language)
2. Use stdin/stdout for events
3. Add to config
4. Framework handles rest

**No framework changes needed!** ✅

### Adding New Command

1. Add handler in `main.c`
2. Parse command
3. Execute
4. Send response

**Minimal code!** ✅

### Adding New Event Type

1. Tool publishes with new type
2. Other tools subscribe
3. Framework routes automatically

**No configuration!** ✅

---

## Testing Strategy

### Unit Tests

```bash
# Test tool in isolation
echo "ALERT|test|data" | ./tool.py
# Should output response
```

### Integration Tests

```bash
# Test with framework
./yuki-frame -c test.conf
# Check logs for correct routing
```

### Console Tests

```bash
# Console is just a tool!
echo "list" | ./yuki-console.py
# Should output COMMAND event
```

**Everything testable with pipes!** ✅

---

## Future Enhancements

### Possible (Without Breaking Design)

1. **Binary protocol** - More efficient
2. **Compression** - For large events
3. **Encryption** - For sensitive data
4. **Priority queues** - High-priority events
5. **Event replay** - Debugging tool

### Not Possible (Would Break Design)

1. **Synchronous RPC** - Design is async
2. **Direct tool-to-tool** - Breaks hub model
3. **Distributed** - Single-machine design

---

## Comparison

| Aspect | Yuki-Frame | Kafka | RabbitMQ | SystemD |
|--------|------------|-------|----------|---------|
| Scale | 100 tools | 1000s | 1000s | 1000s |
| Complexity | Low ✅ | High | High | Medium |
| Language | Any ✅ | Any | Any | Any |
| Distributed | No | Yes | Yes | No |
| Use Case | Local orchestration | Message bus | Message queue | Service manager |

**Yuki-Frame = Simple, local, tool orchestration** ✅

---

## Summary

### Core Principles

1. **Everything is a tool** - No exceptions
2. **Pipes for all** - One communication method
3. **Hub model** - Framework coordinates
4. **Tool isolation** - Separate processes
5. **Event-driven** - Async pub/sub

### Why This Works

- ✅ Simple to understand
- ✅ Easy to implement
- ✅ Simple to test
- ✅ Language-agnostic
- ✅ Flexible and extensible

### When to Reconsider

- You have 500+ tools
- You need distributed
- You need <1ms latency
- You need guaranteed delivery

**Until then: This design is perfect!** 🎯

---

**Architecture = Simple, Elegant, Effective** ✅
