#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "syscall.h"

int
main(void)
{
  char buf[16];
  int fd, n1, n2;

  // nollataan ja seurataan readeja
  getreadcount(-1);
  getreadcount(SYS_read);

  n1 = getreadcount(0);
  fd = open("README", 0);
  read(fd, buf, 10);
  read(fd, buf, 10);
  close(fd);
  n2 = getreadcount(0);

  printf(1, "read count alussa %d, sitten %d\n", n1, n2);

  // nollaus argumentilla -1
  getreadcount(-1);
  printf(1, "nollauksen jalkeen %d\n", getreadcount(0));

  // vaihdetaan seurantaan write (16)
  getreadcount(SYS_write);
  n1 = getreadcount(0);
  write(1, "write testi\n", 12);
  n2 = getreadcount(0);
  printf(1, "write count %d -> %d\n", n1, n2);

  exit();
}
