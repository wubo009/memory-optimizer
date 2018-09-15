#include <numa.h>
#include <stdio.h>

#include <string>
#include <vector>
#include "ProcVmstat.h"

int ProcVmstat::load_vmstat()
{
  proc_vmstat = __load_vmstat("/proc/vmstat");
  return proc_vmstat.empty();
}

int ProcVmstat::load_numa_vmstat()
{
  char path[50];
  int rc = 0;

  int max_node = numa_max_node();

  numa_vmstat.clear();
  numa_vmstat.resize(max_node + 1);

  for (int i = 0; i <= max_node; ++i) {
    snprintf(path, sizeof(path), "/sys/devices/system/node/node%d/vmstat", i);
    numa_vmstat[i] = __load_vmstat(path);
    if (numa_vmstat[i].empty())
      ++rc;
  }

  return rc;
}

vmstat_map ProcVmstat::__load_vmstat(const char *path)
{
  vmstat_map map;
  char line[80];
  char key[50];
  unsigned long val;

  FILE *f = fopen(path, "r");
  if (!f) {
    perror(path);
    goto out;
  }

  while (fgets(line, sizeof(line), f))
  {
    int ret = sscanf(line, "%49s %lu\n", key, &val);
    if (ret < 2) {
      fprintf(stderr, "parse failed: %d %s\n%s", ret, path, line);
      continue;
    }
    map[key] = val;
  }

  fclose(f);
out:
  return map;
}

unsigned long ProcVmstat::vmstat(std::string name)
{
  if (proc_vmstat.empty())
    load_vmstat();

  return proc_vmstat.at(name);
}

unsigned long ProcVmstat::vmstat(int nid, std::string name)
{
  if (proc_vmstat.empty())
    load_numa_vmstat();

  return numa_vmstat.at(nid).at(name);
}
