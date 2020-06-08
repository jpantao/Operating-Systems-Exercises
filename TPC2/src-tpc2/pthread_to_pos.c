#include <pthread.h>
#include <stdlib.h>

struct map_t {
	pthread_t id;
	int pos;
	struct map_t* next;
};

static struct map_t* map_base = NULL;
static struct map_t* map_last;


static struct map_t* map_add_element(pthread_t id, int pos) {
	struct map_t* next = (struct map_t*) malloc (sizeof(struct map_t));
	next->id = id;
	next->pos = pos;
	next->next = NULL;

	return next;
}

int pthread_to_pos(pthread_t id) {
	if (map_base == NULL) {
		map_last = map_base = map_add_element(id, 0);
		return 0;
	}

	for (struct map_t* it = map_base; it != NULL; it = it->next) {
		if (it->id == id) 
			return it->pos;
	}

	int pos = map_last->pos + 1;
	map_last = map_last->next = map_add_element(id, pos);
	return pos;
}