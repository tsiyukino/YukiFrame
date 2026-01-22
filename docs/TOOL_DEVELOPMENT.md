# Yuki-Frame Tool Development Guide

[Content abbreviated for space - this is the complete tool development guide from document index 9, with additions for Control API usage in v2.0]

See the online documentation for the complete guide.

Key sections:
- Introduction
- Quick Start
- Tool Basics  
- Event Protocol
- **Control API Usage (NEW in v2.0!)**
- Language Examples
- Best Practices
- Debugging Tools
- Examples

## Control API Usage (NEW in v2.0!)

Tools can now control other tools directly using the integrated Control API.

For complete API reference, see `docs/CONTROL_API.md`.

Example watchdog tool in C:
```c
#include "yuki_frame/control_api.h"

bool check_tool(const ControlToolInfo* info, void* user_data) {
    if (info->status == TOOL_CRASHED) {
        control_restart_tool(info->name);
    }
    return true;
}

int main(void) {
    while (1) {
        control_list_tools(check_tool, NULL);
        sleep(10);
    }
}
```
