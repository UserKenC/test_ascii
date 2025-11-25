#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char *strcat(char *dest, const char *src)
{
  char *d = dest;
  while (*d)
    d++;
  while ((*d++ = *src++) != 0)
    ;
  return dest;
}

// Helper: compare two inodes (same device and inode number)
int same_inode(struct stat *a, struct stat *b)
{
  return a->ino == b->ino && a->dev == b->dev;
}

void print_path(char *path)
{
  if (strlen(path) == 0)
    printf(1, "/\n");
  else
    printf(1, "%s\n", path);
}

void pwd(void)
{
  char path[512] = "";
  char name[DIRSIZ + 1];
  struct stat st, parent_st;
  int fd;

  while (1)
  {
    // Stat current directory
    if (stat(".", &st) < 0)
      return;

    // Stat parent directory
    if (stat("..", &parent_st) < 0)
      return;

    // If current dir == root dir, stop
    if (same_inode(&st, &parent_st))
      break;

    // Open parent directory
    if ((fd = open("..", 0)) < 0)
      return;

    struct dirent de;
    // Scan parent entries
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if (de.inum == 0)
        continue;

      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      // Copy and null-terminate name
      memmove(name, de.name, DIRSIZ);
      name[DIRSIZ] = 0;

      // Build full path for stat lookup: "../name"
      char pathbuf[512];
      strcpy(pathbuf, "../");
      strcat(pathbuf, name);

      // Stat this entry relative to current dir
      struct stat test_st;
      if (stat(pathbuf, &test_st) < 0)
        continue;

      // Found our current directory in the parent
      if (test_st.ino == st.ino)
      {
        char temp[512];
        strcpy(temp, "/");
        strcat(temp, name);
        strcat(temp, path);
        strcpy(path, temp);
        break;
      }
    }

    close(fd);

    // Move up one directory
    if (chdir("..") < 0)
      return;
  }

  print_path(path);
}

int main(int argc, char *argv[])
{
  pwd();
  exit();
}
