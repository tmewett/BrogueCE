/**
 * MCP Client for Brogue Dungeon Master AI
 *
 * Provides interface for Brogue to communicate with the DM agent server
 */

#ifndef MCP_CLIENT_H
#define MCP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MCP_SUCCESS 0
#define MCP_ERROR 1

#define MAX_REQUEST_SIZE 4096
#define MAX_RESPONSE_SIZE 4096

// Configuration structure for MCP connection
typedef struct {
    char server_url[128];
    int connection_timeout_ms;
    int retry_count;
    bool verbose_logging;
} mcp_config_t;

// Request queue structure for async processing
typedef struct request_node {
    char *data;
    struct request_node *next;
} request_node_t;

typedef struct {
    request_node_t *head;
    request_node_t *tail;
    int size;
    int capacity;
} request_queue_t;

// Function declarations
int initialize_mcp_client(void);
void shutdown_mcp_client(void);
void send_game_event(const char *event_type, const char *event_data);
request_queue_t* create_request_queue(int capacity);
void enqueue_request(request_queue_t *queue, char *data);
char* dequeue_request(request_queue_t *queue);
void free_request_queue(request_queue_t *queue);
void* request_worker(void *arg);

// Response handler type
typedef void (*response_handler_t)(const char *response);

// Register a callback for handling responses
void register_response_handler(response_handler_t handler);

#endif /* MCP_CLIENT_H */ 