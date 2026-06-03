#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEFAULT_BASE 0x50000000ULL
#define MAP_SIZE 0x1000UL
#define REG_ID 0x00
#define REG_VERSION 0x04
#define REG_SCRATCH 0x08
#define REG_COUNTER_LO 0x10
#define REG_COUNTER_HI 0x14

#define EXPECTED_ID 0x5ac60111U
#define EXPECTED_VERSION 0x00010000U

static volatile uint32_t *reg32(void *base, uint32_t offset)
{
  return (volatile uint32_t *)((volatile uint8_t *)base + offset);
}

static uint64_t read_counter(void *base)
{
  uint32_t hi0;
  uint32_t lo;
  uint32_t hi1;

  do {
    hi0 = *reg32(base, REG_COUNTER_HI);
    lo = *reg32(base, REG_COUNTER_LO);
    hi1 = *reg32(base, REG_COUNTER_HI);
  } while (hi0 != hi1);

  return ((uint64_t)hi0 << 32) | lo;
}

static uint64_t parse_base(const char *text)
{
  char *end = NULL;
  errno = 0;
  uint64_t value = strtoull(text, &end, 0);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "ERROR: invalid base address '%s'\n", text);
    exit(2);
  }
  return value;
}

int main(int argc, char **argv)
{
  uint64_t base_addr = DEFAULT_BASE;

  if (argc > 2) {
    fprintf(stderr, "Usage: %s [base-address]\n", argv[0]);
    return 2;
  }
  if (argc == 2) {
    base_addr = parse_base(argv[1]);
  }

  printf("\nCVA6 ZCU111 PL peripheral MMIO test\n");
  printf("base: 0x%08" PRIx64 "\n", base_addr);

  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0) {
    fprintf(stderr, "ERROR: open /dev/mem failed: %s\n", strerror(errno));
    return 1;
  }

  void *map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)base_addr);
  if (map == MAP_FAILED) {
    fprintf(stderr, "ERROR: mmap 0x%08" PRIx64 " failed: %s\n", base_addr, strerror(errno));
    close(fd);
    return 1;
  }

  uint32_t id = *reg32(map, REG_ID);
  uint32_t version = *reg32(map, REG_VERSION);
  printf("id: 0x%08" PRIx32 "\n", id);
  printf("version: 0x%08" PRIx32 "\n", version);

  if (id != EXPECTED_ID) {
    fprintf(stderr, "FAIL: expected id 0x%08x\n", EXPECTED_ID);
    return 1;
  }
  if (version != EXPECTED_VERSION) {
    fprintf(stderr, "FAIL: expected version 0x%08x\n", EXPECTED_VERSION);
    return 1;
  }

  static const uint32_t patterns[] = {
    0x00000000U,
    0xffffffffU,
    0xa5a55a5aU,
    0x1234fedcU,
  };

  for (size_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
    *reg32(map, REG_SCRATCH) = patterns[i];
    uint32_t got = *reg32(map, REG_SCRATCH);
    if (got != patterns[i]) {
      fprintf(stderr, "FAIL: scratch wrote 0x%08" PRIx32 ", read 0x%08" PRIx32 "\n",
              patterns[i], got);
      return 1;
    }
  }
  printf("scratch: PASS\n");

  uint64_t counter0 = read_counter(map);
  for (volatile unsigned int i = 0; i < 100000; i++) {
  }
  uint64_t counter1 = read_counter(map);
  printf("counter: 0x%016" PRIx64 " -> 0x%016" PRIx64 "\n", counter0, counter1);
  if (counter1 <= counter0) {
    fprintf(stderr, "FAIL: counter did not advance\n");
    return 1;
  }
  printf("counter: PASS\n");

  if (munmap(map, MAP_SIZE) != 0) {
    fprintf(stderr, "WARN: munmap failed: %s\n", strerror(errno));
  }
  close(fd);

  printf("PASS: PL peripheral MMIO test completed\n");
  return 0;
}
