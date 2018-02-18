/* 
Dijkstra algorithm with reduced cost presented here. Equivalent to A* with given heuristic function.
Original implementation of the Dijkstra algorithm, which it has been built upon, can be found at the website: 
https://rosettacode.org/wiki/Dijkstra%27s_algorithm 
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <locale.h>
 
typedef struct {
    int vertex;
    int weight;
} edge_t;
 
typedef struct {
    edge_t **edges;
    int edges_len;
    int edges_size;
    int dist;
    int prev;
    int visited;
} vertex_t;
 
typedef struct {
    vertex_t **vertices;
    int vertices_len;
    int vertices_size;
} graph_t;
 
typedef struct {
    int *data;
    int *prio;
    int *index;
    int len;
    int size;
} heap_t;
 
void add_vertex (graph_t *g, int i) {
    if (g->vertices_size < i + 1) {
        int size = g->vertices_size * 2 > i ? g->vertices_size * 2 : i + 4;
        g->vertices = realloc(g->vertices, size * sizeof (vertex_t *));
        for (int j = g->vertices_size; j < size; j++)
            g->vertices[j] = NULL;
        g->vertices_size = size;
    }
    if (!g->vertices[i]) {
        g->vertices[i] = calloc(1, sizeof (vertex_t));
        g->vertices_len++;
    }
}
 
void add_edge (graph_t *g, int a, int b, int w) {
    add_vertex(g, a);
    add_vertex(g, b);
    vertex_t *v = g->vertices[a];
    if (v->edges_len >= v->edges_size) {
        v->edges_size = v->edges_size ? v->edges_size * 2 : 4;
        v->edges = realloc(v->edges, v->edges_size * sizeof (edge_t *));
    }
    edge_t *e = calloc(1, sizeof (edge_t));
    e->vertex = b;
    e->weight = w;
    v->edges[v->edges_len++] = e;
}
 
heap_t *create_heap (int n) {
    heap_t *h = calloc(1, sizeof (heap_t));
    h->data = calloc(n + 1, sizeof (int));
    h->prio = calloc(n + 1, sizeof (int));
    h->index = calloc(n, sizeof (int));
    return h;
}
 
void push_heap (heap_t *h, int v, int p) {
    int i = h->index[v] == 0 ? ++h->len : h->index[v];
    int j = i / 2;
    while (i > 1) {
        if (h->prio[j] < p)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
        j = j / 2;
    }
    h->data[i] = v;
    h->prio[i] = p;
    h->index[v] = i;
}
 
int min (heap_t *h, int i, int j, int k) {
    int m = i;
    if (j <= h->len && h->prio[j] < h->prio[m])
        m = j;
    if (k <= h->len && h->prio[k] < h->prio[m])
        m = k;
    return m;
}
 
int pop_heap (heap_t *h) {
    int v = h->data[1];
    int i = 1;
    while (1) {
        int j = min(h, h->len, 2 * i, 2 * i + 1);
        if (j == h->len)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
    }
    h->data[i] = h->data[h->len];
    h->prio[i] = h->prio[h->len];
    h->index[h->data[i]] = i;
    h->len--;
    return v;
}
 
void dijkstra (graph_t *g, int a, int b, int* m_lookup, int* heuristic, int* exit_node) {
    int i, j;
    int adj_weight;
    for (i = 0; i < g->vertices_len; i++) {
        vertex_t *v = g->vertices[i];
        v->dist = INT_MAX;
        v->prev = 0;
        v->visited = 0;
    }
    vertex_t *v = g->vertices[a];
    v->dist = 0;
    heap_t *h = create_heap(g->vertices_len);
    push_heap(h, a, v->dist);
    while (h->len) {
        i = pop_heap(h);
        // use lookup instead of direct index for termination check
        if (m_lookup[i] == b){
        	*exit_node = i;
        	break;
		}
            
        v = g->vertices[i];
        v->visited = 1;
        for (j = 0; j < v->edges_len; j++) {
            edge_t *e = v->edges[j];
            vertex_t *u = g->vertices[e->vertex];
            
            // e->weight is a cost, it should be reduced with heuristic: e->weight + heur(project_on_master(e->vertex)) - heur(project_on_master(i))
            // we can't use heur(i) as there is no such value, we have to project i to corresponding master node id first
            adj_weight = e->weight + heuristic[m_lookup[e->vertex]] - heuristic[m_lookup[i]];
            
            if (!u->visited && v->dist + adj_weight <= u->dist) {
                u->prev = i;
                u->dist = v->dist + adj_weight;
                push_heap(h, e->vertex, u->dist);
            }
        }
    }
}
 
void print_path (graph_t *g, int i, int* m_lookup, char (*names)[50]) {
    int n, j;
    vertex_t *v, *u;
    v = g->vertices[i];
    if (v->dist == INT_MAX) {
        printf("no path\n");
        return;
    }
    for (n = 1, u = v; u->dist; u = g->vertices[u->prev], n++)
        ;
    int *path = malloc(n);
    path[n - 1] = i;
    for (j = 0, u = v; u->dist; u = g->vertices[u->prev], j++)
        path[n - j - 2] = u->prev;
    printf("Adjusted distance: %d\n", v->dist);
    for (int k = 0; k < n-1; k++) printf("%s(%d) -> ", names[m_lookup[path[k]]], path[k]);
    printf("%s(%d)\n", names[m_lookup[path[n-1]]], path[n-1]);
}
 
int main () {	

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	
	printf("%s","Loading station names\n");
	int st_names_size;
	FILE *st_names_file;
	st_names_file = fopen("station_names_c.txt","r");
	fscanf(st_names_file, "%d", &st_names_size);
	char st_names [st_names_size][50];
	fgets(st_names[0],50,st_names_file); // move reseted counter 
	for(int i=0;i<st_names_size;i++) {
		fgets(st_names[i],50,st_names_file);
		st_names[i][strlen(st_names[i]) - 1] = '\0';
	}
	printf("%s","Completed loading station names\n");


	printf("%s","Loading master station lookup\n");
	/* read lookup table (array) for node to master node: start */
	int master_lookup_size;
	FILE *master_lookup_file;
	master_lookup_file = fopen("master_cpp.csv", "r");
	fscanf(master_lookup_file, "%d", &master_lookup_size);
	int* master_lookup = malloc(master_lookup_size*sizeof(int));
	for(int i=0;i<master_lookup_size;i++) if(!fscanf(master_lookup_file, "%d", &master_lookup[i])) break;
	fclose(master_lookup_file);
	/* read lookup table (array) for node to master node: end */
	printf("%s","Completed loading master station lookup\n");

	printf("%s","Loading heuristic table\n");
	/* read matrix of heuristic function: start */
	int h_row_size;
	int h_col_size;
	FILE *h_file; 
	h_file = fopen("adv_heuristic_cpp.csv", "r");
	fscanf(h_file, "%d", &h_row_size); fscanf(h_file, "%d", &h_col_size);
	int** h_mat = malloc(h_row_size*sizeof(int*)); 
	for(int i=0;i<h_row_size;++i) h_mat[i]=malloc(h_col_size*sizeof(int));
	for(int i=0;i<h_row_size;i++) for(int j=0;j<h_col_size;j++) if(!fscanf(h_file, "%d", &h_mat[i][j])) break;
	fclose(h_file);
	/* read matrix of heuristic function: end */
	printf("%s","Completed loading heuristic table\n");

	printf("%s","Loading and constructing graph connections\n");
	/* read connections data and build graph: start */
    graph_t *g = calloc(1, sizeof (graph_t));
    for(int i=0;i<809;i++) add_edge(g, i, i+1, 0);
    int from_node;
    int to_node;
    int edge_weight;
    
	int train_con_size;
    FILE *train_con_file;
    train_con_file = fopen("connections_train_cpp.txt","r");
    fscanf(train_con_file, "%d", &train_con_size);
    for(int i=0;i<train_con_size;i++) {
    	fscanf(train_con_file, "%d", &from_node);
    	fscanf(train_con_file, "%d", &to_node);
    	fscanf(train_con_file, "%d", &edge_weight);
    	add_edge(g, from_node, to_node, edge_weight);
	}
	fclose(train_con_file);
    printf("%s","Train connections added\n");

	printf("%s","Started loading wait connections\n");    
    int wait_con_size;
    FILE *wait_con_file;
    wait_con_file = fopen("connections_wait_cpp.txt","r");
    fscanf(wait_con_file, "%d", &wait_con_size);
    for(int i=0;i<wait_con_size;i++) {
    	fscanf(wait_con_file, "%d", &from_node);
    	fscanf(wait_con_file, "%d", &to_node);
    	fscanf(wait_con_file, "%d", &edge_weight);
    	add_edge(g, from_node, to_node, edge_weight);
	}
	fclose(wait_con_file);
    /* read connections data and build graph: end */
    printf("%s","Wait connections added\n");
    
    printf("%s","Ready to work!\n");
    for(;;){
	
    	int a; int b; 
		int *exit = calloc(1, sizeof (int)); *exit = 0;
		printf("%s","Enter id of timestamped start station and id of master end station\n");
    	scanf("%d", &a); scanf("%d", &b);
    	
    	if(a==-1) break;
    	
    	/* narrow down heuristic matrix to array: start */
    	int* h_arr = malloc(h_row_size*sizeof(int));
    	for(int i=0;i<h_row_size;i++) h_arr[i] = h_mat[i][b];
    	/* narrow down heuristic matrix to array: end */
    
    	dijkstra(g, a, b, master_lookup, h_arr, exit);
    	print_path(g, *exit, master_lookup, st_names);
	}
    return 0;
}
