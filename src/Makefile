# MCP client sources
MCP_SOURCES = mcp/mcp_client.c mcp/event_hooks.c mcp/dm_main.c
MCP_OBJECTS = $(MCP_SOURCES:.c=.o)

# Add the MCP objects to the list of objects to compile
BROGUE_OBJECTS += $(MCP_OBJECTS)

# Additional compiler flags
CFLAGS += -I mcp
LDFLAGS += -lpthread 