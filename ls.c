#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "deque.h"
#include "option.h"

typedef char* str;

// introduce str_deque and str_option
Deque(str);
Option(str);

deque_new(str)
deque_free(str)
deque_push_back(str)
deque_pop_front(str)

typedef struct {
	bool recursive : 1;
	bool one : 1;
	bool all : 1;
	bool long_listing : 1;
} Args;

int alphabetic_sort(const struct dirent**, const struct dirent**);
int exclude_hidden(const struct dirent*);

void print_dir_contents(const char*, const Args*);
int alphabetic_sort(const struct dirent**, const struct dirent**);
int exclude_hidden(const struct dirent*);
void handle_dir(str_deque*, const Args*);
void push_dir(str_deque*, const char*, const char*);
char* concat_paths(const char*, const char*);

int main(int argc, char ** argv) {
	Args args = { 0 };
	int opt;
	while ((opt = getopt(argc, argv, "1aRl")) != -1) {
		switch (opt) {
			case '1':
				args.one = true;
				break;
			case 'a':
				args.all = true;
				break;
			case 'R':
				args.recursive = true;
				break;
			case 'l':
				args.long_listing = true;
				break;
		}
	}

	str_deque* queue = str_deque_new();

	if (optind == argc) {
		push_dir(queue, NULL, ".");
		handle_dir(queue, &args);
	} else {
		if (args.recursive) {
			for (int i = optind; i < argc; i++) {
				push_dir(queue, NULL, argv[i]);
				handle_dir(queue, &args);
			}
		} else {
			for (int i = optind; i < argc; i++) {
				push_dir(queue, NULL, argv[i]);
			}
			handle_dir(queue, &args);
		}
	}

	str_deque_free(&queue);

	return 0;
}

void print_mode(__mode_t mode) {
		switch (mode & S_IFMT) {
			case S_IFREG:
				putchar('-');
				break;
			case S_IFDIR:
				putchar('d');
				break;
			case S_IFCHR:
				putchar('c');
				break;
			case S_IFBLK:
				putchar('b');
				break;
			case S_IFIFO:
				putchar('p');
				break;
			case S_IFSOCK:
				putchar('s');
				break;
			case S_IFLNK:
				putchar('l');
				break;
			default:
				putchar('?');
				break;
		}

    printf("%c%c%c%c%c%c%c%c%c",
        (mode & S_IRUSR) ? 'r' : '-', (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ?
            (((mode & S_ISUID)) ? 's' : 'x') :
            (((mode & S_ISUID)) ? 'S' : '-'),
        (mode & S_IRGRP) ? 'r' : '-', (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ?
            (((mode & S_ISGID)) ? 's' : 'x') :
            (((mode & S_ISGID)) ? 'S' : '-'),
        (mode & S_IROTH) ? 'r' : '-', (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ?
            (((mode & S_ISVTX)) ? 't' : 'x') :
            (((mode & S_ISVTX)) ? 'T' : '-'));
}

void handle_dir(str_deque* queue, const Args* args) {
	str_option opt;
	while ((opt = str_deque_pop_front(queue)).present) {
		const char* current_dir = opt.value;
		struct dirent** entries = NULL;

		size_t num_dirs = scandir(current_dir, &entries, (args->all) ? NULL : exclude_hidden, alphabetic_sort);

		if (args->recursive) {
			printf("%s:\n", current_dir);
		}

		if (entries == NULL) {
			if (!deque_is_empty(queue)) putchar('\n');
			continue;
		}

		for (size_t i = 0; i < num_dirs; i++) {
			struct dirent* entry = entries[i];
			if (entry->d_type == DT_DIR && args->recursive && strcmp(entry->d_name, ".") != 0
					&& strcmp(entry->d_name, "..") != 0) {
				push_dir(queue, current_dir, entry->d_name);
			}

			if (args->long_listing) {
				struct stat buf;
				char* path = concat_paths(current_dir, entry->d_name);
				stat(path, &buf);
				free(path);

				struct passwd* pwd = getpwuid(buf.st_uid);
				struct group* grp = getgrgid(buf.st_gid);

				static char timebuf[20];
				struct tm* timeinfo = localtime(&buf.st_mtime);
				strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);

				print_mode(buf.st_mode);
				printf(" %lu %s %s %lu %s %s\n", buf.st_nlink, grp->gr_name, pwd->pw_name, buf.st_size, timebuf, entry->d_name);

			} else if (args->one) {
				printf("%s\n", entry->d_name);
			} else {
				printf("%s  ", entry->d_name);
			}

			free(entry);
		}

		if (!(args->one || args->long_listing)) {
			putchar('\n');
		}

		if (args->recursive && !deque_is_empty(queue)) {
			putchar('\n');
		}

		free(entries);
		free(opt.value);
	}
}

/**
 * Helper function to enqueue a directory.
 * Handles adding path seperators and allocating new memory for each path.
 */
void push_dir(str_deque* queue, const char* current_dir, const char* subdir) {
	char* dir;

	if (current_dir == NULL) {
		size_t s_len = strnlen(subdir, 256);
		dir = calloc(s_len + 1, sizeof(char));
		strncpy(dir, subdir, s_len);
	} else {
		dir = concat_paths(current_dir, subdir);
	}

	str_deque_push_back(queue, dir);
}

char* concat_paths(const char* base, const char* file) {
	size_t b_len = strnlen(base, 256);
	size_t f_len = strnlen(file, 256);
	size_t total = b_len + f_len + 2;

	char* dir = calloc(total, sizeof(char));

	snprintf(dir, total, "%s/%s", base, file);

	return dir;
}

int alphabetic_sort(const struct dirent** a, const struct dirent** b) {
	return strncasecmp((*a)->d_name, (*b)->d_name, 256);
}

int exclude_hidden(const struct dirent* entry) {
	return entry->d_name[0] != '.';
}
