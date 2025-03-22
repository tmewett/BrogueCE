#include "mcp_client.h"
#include <pthread.h>

// Global state
static mcp_config_t config = {
    .server_url = "http://localhost:3000/api/event",
    .connection_timeout_ms = 500,
    .retry_count = 3,
    .verbose_logging = false
};

static request_queue_t *request_queue = NULL;
static pthread_t worker_thread;
static int worker_running = 0;
static response_handler_t response_callback = NULL;

// Mock HTTP request function (to be replaced with actual HTTP library)
static int send_http_request(const char *url, const char *data, char *response, int response_size) {
    // This is a mock implementation - in a real implementation, this would use libcurl or similar
    // Return success for now
    if (config.verbose_logging) {
        printf("[MCP] Would send HTTP request to %s: %s\n", url, data);
    }
    
    // Mock response
    snprintf(response, response_size, "{\"status\":\"success\",\"narrative\":\"The dungeon seems to shift around you.\"}");
    
    return MCP_SUCCESS;
}

// Create a new request queue
request_queue_t* create_request_queue(int capacity) {
    request_queue_t *queue = (request_queue_t *)malloc(sizeof(request_queue_t));
    if (!queue) return NULL;
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->capacity = capacity;
    
    return queue;
}

// Add a request to the queue
void enqueue_request(request_queue_t *queue, char *data) {
    if (!queue || !data) return;
    
    // If queue is full, drop the oldest request
    if (queue->size >= queue->capacity && queue->head) {
        request_node_t *old_head = queue->head;
        queue->head = old_head->next;
        free(old_head->data);
        free(old_head);
        queue->size--;
    }
    
    // Create new node
    request_node_t *node = (request_node_t *)malloc(sizeof(request_node_t));
    if (!node) return;
    
    node->data = data;
    node->next = NULL;
    
    // Add to queue
    if (queue->tail) {
        queue->tail->next = node;
        queue->tail = node;
    } else {
        queue->head = queue->tail = node;
    }
    
    queue->size++;
}

// Remove and return the oldest request from the queue
char* dequeue_request(request_queue_t *queue) {
    if (!queue || !queue->head) return NULL;
    
    request_node_t *node = queue->head;
    char *data = node->data;
    
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    
    free(node);
    queue->size--;
    
    return data;
}

// Free the request queue
void free_request_queue(request_queue_t *queue) {
    if (!queue) return;
    
    request_node_t *current = queue->head;
    while (current) {
        request_node_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    
    free(queue);
}

// Worker thread function
void* request_worker(void *arg) {
    // Loop until shutdown
    while (worker_running) {
        char *request_data = dequeue_request(request_queue);
        if (request_data) {
            char response[MAX_RESPONSE_SIZE];
            
            // Send the request
            int result = send_http_request(config.server_url, request_data, response, MAX_RESPONSE_SIZE);
            
            // Handle response if successful and callback is registered
            if (result == MCP_SUCCESS && response_callback) {
                response_callback(response);
            }
            
            free(request_data);
        } else {
            // No requests, sleep briefly
            struct timespec sleep_time;
            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = 10000000; // 10ms
            nanosleep(&sleep_time, NULL);
        }
    }
    
    return NULL;
}

// Initialize the MCP client
int initialize_mcp_client(void) {
    // Initialize request queue
    request_queue = create_request_queue(50);
    if (!request_queue) {
        fprintf(stderr, "[MCP] Failed to create request queue\n");
        return MCP_ERROR;
    }
    
    // Start worker thread
    worker_running = 1;
    if (pthread_create(&worker_thread, NULL, request_worker, NULL) != 0) {
        fprintf(stderr, "[MCP] Failed to create worker thread\n");
        free_request_queue(request_queue);
        request_queue = NULL;
        return MCP_ERROR;
    }
    
    if (config.verbose_logging) {
        printf("[MCP] Client initialized, connecting to %s\n", config.server_url);
    }
    
    return MCP_SUCCESS;
}

// Shutdown the MCP client
void shutdown_mcp_client(void) {
    if (worker_running) {
        // Stop worker thread
        worker_running = 0;
        pthread_join(worker_thread, NULL);
    }
    
    // Free request queue
    if (request_queue) {
        free_request_queue(request_queue);
        request_queue = NULL;
    }
    
    if (config.verbose_logging) {
        printf("[MCP] Client shutdown\n");
    }
}

// Register response handler
void register_response_handler(response_handler_t handler) {
    response_callback = handler;
}

// Send game event to DM agent server
void send_game_event(const char *event_type, const char *event_data) {
    if (!worker_running || !request_queue) return;
    
    // Format the JSON payload
    char *payload = (char *)malloc(MAX_REQUEST_SIZE);
    if (!payload) return;
    
    snprintf(payload, MAX_REQUEST_SIZE, 
             "{\"eventType\":\"%s\",\"eventData\":%s,\"context\":{\"timestamp\":%ld}}",
             event_type, event_data, (long)time(NULL));
    
    // Enqueue the request
    enqueue_request(request_queue, payload);
} 