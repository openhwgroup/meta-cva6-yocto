#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_TEST_BYTES (256ULL * 1024ULL * 1024ULL)
#define CACHELINE_BYTES 64ULL

static uint64_t rdcycle(void) {
  uint64_t value;
  __asm__ volatile("rdcycle %0" : "=r"(value));
  return value;
}

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    perror("clock_gettime");
    exit(1);
  }
  return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}

static uint64_t mix64(uint64_t value) {
  value ^= value >> 30;
  value *= 0xbf58476d1ce4e5b9ULL;
  value ^= value >> 27;
  value *= 0x94d049bb133111ebULL;
  value ^= value >> 31;
  return value;
}

static uint64_t pattern_for_word(uint64_t word_index, uint64_t seed) {
  return mix64(word_index + seed) ^ (0x9e3779b97f4a7c15ULL * (word_index | 1ULL));
}

static uint64_t parse_size(const char *text) {
  char *end = NULL;
  uint64_t multiplier = 1;
  errno = 0;
  uint64_t value = strtoull(text, &end, 0);

  if (errno != 0 || end == text) {
    fprintf(stderr, "ERROR: invalid size '%s'\n", text);
    exit(2);
  }

  if (*end != '\0') {
    if (end[1] != '\0') {
      fprintf(stderr, "ERROR: invalid size suffix in '%s'\n", text);
      exit(2);
    }
    switch (*end) {
    case 'k':
    case 'K':
      multiplier = 1024ULL;
      break;
    case 'm':
    case 'M':
      multiplier = 1024ULL * 1024ULL;
      break;
    case 'g':
    case 'G':
      multiplier = 1024ULL * 1024ULL * 1024ULL;
      break;
    default:
      fprintf(stderr, "ERROR: invalid size suffix '%c'\n", *end);
      exit(2);
    }
  }

  if (value > UINT64_MAX / multiplier) {
    fprintf(stderr, "ERROR: size overflow in '%s'\n", text);
    exit(2);
  }

  value *= multiplier;
  value &= ~7ULL;
  if (value < 4096ULL) {
    fprintf(stderr, "ERROR: size must be at least 4K\n");
    exit(2);
  }
  return value;
}

static void usage(const char *argv0) {
  fprintf(stderr, "Usage: %s [bytes|K|M|G]\n", argv0);
  fprintf(stderr, "Example: %s 512M\n", argv0);
}

static void report_phase(const char *name, uint64_t bytes, double start_sec,
                         uint64_t start_cycle) {
  double elapsed = now_seconds() - start_sec;
  uint64_t cycles = rdcycle() - start_cycle;
  double mib = (double)bytes / (1024.0 * 1024.0);
  double mib_s = elapsed > 0.0 ? mib / elapsed : 0.0;
  printf("%-22s %8.3f s  %8.2f MiB/s  cycles=%" PRIu64 "\n", name, elapsed,
         mib_s, cycles);
}

static void fill_pattern(uint64_t *mem, uint64_t words, uint64_t seed) {
  for (uint64_t i = 0; i < words; ++i) {
    mem[i] = pattern_for_word(i, seed);
  }
}

static void invert_pattern(uint64_t *mem, uint64_t words) {
  for (uint64_t i = 0; i < words; ++i) {
    mem[i] = ~mem[i];
  }
}

static void verify_pattern(uint64_t *mem, uint64_t words, uint64_t seed,
                           int inverted) {
  uint64_t checksum = 0;
  for (uint64_t i = 0; i < words; ++i) {
    uint64_t expected = pattern_for_word(i, seed);
    if (inverted) {
      expected = ~expected;
    }
    uint64_t actual = mem[i];
    checksum ^= mix64(actual + i);
    if (actual != expected) {
      fprintf(stderr,
              "FAIL: word=%" PRIu64 " offset=0x%" PRIx64
              " expected=0x%016" PRIx64 " actual=0x%016" PRIx64 "\n",
              i, (uint64_t)(i * 8ULL), expected, actual);
      exit(1);
    }
  }
  printf("  checksum=0x%016" PRIx64 "\n", checksum);
}

static void stride_pattern(uint64_t *mem, uint64_t words, uint64_t seed) {
  const uint64_t stride_words = CACHELINE_BYTES / sizeof(uint64_t);
  for (uint64_t i = 0; i < words; i += stride_words) {
    mem[i] = pattern_for_word(i, seed);
  }
}

static void verify_stride(uint64_t *mem, uint64_t words, uint64_t seed) {
  const uint64_t stride_words = CACHELINE_BYTES / sizeof(uint64_t);
  uint64_t checksum = 0;
  for (uint64_t i = 0; i < words; i += stride_words) {
    uint64_t expected = pattern_for_word(i, seed);
    uint64_t actual = mem[i];
    checksum ^= mix64(actual + i);
    if (actual != expected) {
      fprintf(stderr,
              "FAIL: stride word=%" PRIu64 " offset=0x%" PRIx64
              " expected=0x%016" PRIx64 " actual=0x%016" PRIx64 "\n",
              i, (uint64_t)(i * 8ULL), expected, actual);
      exit(1);
    }
  }
  printf("  stride checksum=0x%016" PRIx64 "\n", checksum);
}

int main(int argc, char **argv) {
  uint64_t bytes = DEFAULT_TEST_BYTES;
  if (argc > 2) {
    usage(argv[0]);
    return 2;
  }
  if (argc == 2) {
    bytes = parse_size(argv[1]);
  }

  long page_size = sysconf(_SC_PAGESIZE);
  if (page_size <= 0) {
    perror("sysconf");
    return 1;
  }
  bytes = (bytes / (uint64_t)page_size) * (uint64_t)page_size;

  printf("\nCVA6 ZCU111 Linux DDR4 memstress\n");
  printf("Requested bytes : %" PRIu64 " (%.2f MiB)\n", bytes,
         (double)bytes / (1024.0 * 1024.0));

  uint64_t *mem = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  printf("Buffer virtual  : %p\n", (void *)mem);
  printf("Word count      : %" PRIu64 "\n", bytes / sizeof(uint64_t));
  printf("Cacheline stride: %u bytes\n\n", (unsigned)CACHELINE_BYTES);

  uint64_t words = bytes / sizeof(uint64_t);
  const uint64_t seed0 = 0x435641365a435531ULL;
  const uint64_t seed1 = 0x600dca5e600dca5eULL;
  double start_sec;
  uint64_t start_cycle;

  start_sec = now_seconds();
  start_cycle = rdcycle();
  fill_pattern(mem, words, seed0);
  report_phase("fill address pattern", bytes, start_sec, start_cycle);

  start_sec = now_seconds();
  start_cycle = rdcycle();
  verify_pattern(mem, words, seed0, 0);
  report_phase("verify pattern", bytes, start_sec, start_cycle);

  start_sec = now_seconds();
  start_cycle = rdcycle();
  invert_pattern(mem, words);
  report_phase("invert pattern", bytes, start_sec, start_cycle);

  start_sec = now_seconds();
  start_cycle = rdcycle();
  verify_pattern(mem, words, seed0, 1);
  report_phase("verify inverted", bytes, start_sec, start_cycle);

  start_sec = now_seconds();
  start_cycle = rdcycle();
  stride_pattern(mem, words, seed1);
  report_phase("stride write", bytes / (CACHELINE_BYTES / sizeof(uint64_t)),
               start_sec, start_cycle);

  start_sec = now_seconds();
  start_cycle = rdcycle();
  verify_stride(mem, words, seed1);
  report_phase("stride verify", bytes / (CACHELINE_BYTES / sizeof(uint64_t)),
               start_sec, start_cycle);

  if (munmap(mem, bytes) != 0) {
    perror("munmap");
    return 1;
  }

  printf("\nPASS: DDR4 memstress completed\n");
  return 0;
}
