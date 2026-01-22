#!/bin/bash
# yuki-control - Simple control script for Yuki-Frame
# Writes commands to command file and displays response

COMMAND_FILE="yuki-frame.cmd"
RESPONSE_FILE="yuki-frame.response"
MAX_WAIT=5

show_usage() {
    echo "Yuki-Frame Control Script v2.0"
    echo ""
    echo "Usage: $0 COMMAND [TOOL_NAME]"
    echo ""
    echo "Commands:"
    echo "  start <tool>     Start a tool"
    echo "  stop <tool>      Stop a tool"
    echo "  restart <tool>   Restart a tool"
    echo "  list             List all tools and their status"
    echo "  status <tool>    Show detailed status of a tool"
    echo "  shutdown         Shutdown the framework"
    echo "  help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 list"
    echo "  $0 start my_tool"
    echo "  $0 status my_tool"
    echo "  $0 stop my_tool"
    echo ""
}

if [ $# -lt 1 ]; then
    show_usage
    exit 1
fi

COMMAND=$1
TOOL_NAME=$2

# Handle help
if [ "$COMMAND" = "help" ] || [ "$COMMAND" = "-h" ] || [ "$COMMAND" = "--help" ]; then
    show_usage
    exit 0
fi

# Remove old response file
rm -f "$RESPONSE_FILE"

# Write command to file
if [ -n "$TOOL_NAME" ]; then
    echo "$COMMAND $TOOL_NAME" > "$COMMAND_FILE"
else
    echo "$COMMAND" > "$COMMAND_FILE"
fi

echo "Sent command: $COMMAND $TOOL_NAME"
echo -n "Waiting for response"

# Wait for response
count=0
while [ $count -lt $MAX_WAIT ]; do
    if [ -f "$RESPONSE_FILE" ]; then
        echo ""
        echo ""
        cat "$RESPONSE_FILE"
        rm -f "$RESPONSE_FILE"
        rm -f "$COMMAND_FILE"
        exit 0
    fi
    echo -n "."
    sleep 1
    count=$((count + 1))
done

echo ""
echo ""
echo "Error: Timeout waiting for response."
echo "Is yuki-frame running? Check with: ps aux | grep yuki-frame"
rm -f "$COMMAND_FILE"
exit 1
