#include"backup.h"

typedef struct file_info {
	char name[256];
	const char *path;
	uint64_t checksum;
	int8_t type;
	int8_t level;
	size_t size;
	time_t scantime;
	int8_t thread_number;
} file_t;

typedef struct node {
	file_t link;
	struct node *next;
} node_t;

static time_t t0;

node_t *files_head = NULL;
node_t *current_node = NULL;

size_t total_size = 0;
size_t max_allowed_thread_size = 0;
size_t current_thread_size = 0;

int8_t thread_number = 0;

static void push(node_t *head, file_t filestruct)
{
	node_t *current = head;
	while(current->next != NULL)
	{
		current = current->next;
	}
	current->next = malloc(sizeof(node_t));
	current->next->link = filestruct;
	current->next->next = NULL;
}

static void delete(node_t *head)
{
	node_t *current;
	while((current = head) != NULL)
	{
		head = head->next;
		free((void*)current->link.path);
		free(current);
		current = NULL;
	}
}

static void print_list(node_t *head)
{
	node_t *current = head;
	while(current != NULL)
	{
		printf("%s\n%s\n%i\n%li\n%li\n%"PRIu64"\n%i\n\n", 
			current->link.path, 
			current->link.name, 
			current->link.type, 
			current->link.size, 
			current->link.scantime,
			current->link.checksum,
			current->link.thread_number
		);
		if(current->next != NULL)
		{
			current = current->next;
		}
		else
		{
			break;
		}
	}
}

static uint64_t hash(const char path[4096], size_t size)
{
	if(size > FILEBUFFER)
	{
		size = FILEBUFFER;
	}

	FILE *fp = fopen(path, "rb");
	if(fp == NULL)
	{
		return 0;
	}

	int8_t buffer[size];
	XXH64_state_t state64;	
	size_t total_read = 1;
		
	XXH64_reset(&state64, 0);
	while(total_read)
	{
		total_read = fread(buffer, 1, size, fp);	
		XXH64_update(&state64, buffer, size);
	}

	fclose(fp);	
	return XXH64_digest(&state64);
}

static int filewalk_info_callback(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
	file_t current_file = {0};
	current_file.path = strdup(fpath);
	strcpy(current_file.name, fpath + ftwbuf->base);
	current_file.size = sb->st_size;
	current_file.type = tflag;
	current_file.scantime = t0;
	current_file.thread_number = thread_number;

	current_thread_size += sb->st_size;
	if(current_thread_size > max_allowed_thread_size)
	{
		++thread_number;
		current_thread_size = 0;
	}

	current_node->link = current_file;
	current_node->next = malloc(sizeof(node_t));
	current_node = current_node->next;
	current_node->next = NULL;

	return 0;
}

static int filewalk_size_callback(const char *fpath, const struct stat *sb, int typeflag)
{
	total_size += sb->st_size;
	return 0;
}

void *thread_hash(void* t)
{
	node_t *current = files_head;
	while(current != NULL)
	{
		if(current->link.thread_number == (uintptr_t)t)
		{
			current->link.checksum = hash(current->link.path, current->link.size);
		}
		if(current->next != NULL)
		{
			current = current->next;
		}
		else
		{
			break;
		}
	}

	pthread_exit((void*) t);
}

int backup(job_t *job_import)
{
	t0 = time(0);

	if(ftw(job_import->src_path, &filewalk_size_callback, 1))
	{
		printf("SIZE CALC ERROR\n");
		exit(EXIT_FAILURE);
	}

	job_import->max_threads = 4;
	max_allowed_thread_size = total_size / job_import->max_threads;

	files_head = malloc(sizeof(node_t));
	current_node = files_head;

	if(nftw(job_import->src_path, filewalk_info_callback, 20, 0) == -1)
	{
		fprintf(stderr, "ERROR NFTW\n");
		exit(EXIT_FAILURE);
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	void *status;

	pthread_t threads[job_import->max_threads];
	int rc;
	for(long t = 0; t < job_import->max_threads; t++)
	{
		rc = pthread_create(&threads[t], NULL, thread_hash, (void*) t);
		if(rc)
		{
			printf("ERROR FROM PTHREAD\nrc is %i\n", rc);
			exit(EXIT_FAILURE);
		}
	}
	
	for(long t = 0; t < job_import->max_threads; t++)
	{
		rc = pthread_join(threads[t], &status);
		if(rc)
		{
			puts("PTHREAD ERROR");
			exit(EXIT_FAILURE);
		}
	}

	//print_list(files_head);
	delete(files_head);
	pthread_exit(NULL);
	return 0;
}

