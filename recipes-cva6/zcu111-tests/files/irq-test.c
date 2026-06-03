#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEFAULT_UIO "/dev/uio0"
#define MAP_SIZE 0x1000UL
#define REG_ID 0x00
#define REG_VERSION 0x04
#define REG_IRQ_STATUS 0x18
#define REG_IRQ_ENABLE 0x20
#define REG_IRQ_ACK 0x28
#define REG_IRQ_TRIGGER 0x30

#define EXPECTED_ID 0x5ac60111U
#define EXPECTED_VERSION 0x00010000U

static volatile uint32_t *reg32(void *base, uint32_t offset)
{
  return (volatile uint32_t *)((volatile uint8_t *)base + offset);
}

static int wait_for_irq(int fd, uint32_t *irq_count)
{
  ssize_t got;

  do {
    got = read(fd, irq_count, sizeof(*irq_count));
  } while (got < 0 && errno == EINTR);

  if (got != (ssize_t)sizeof(*irq_count)) {
    fprintf(stderr, "ERROR: UIO read failed: %s\n", got < 0 ? strerror(errno) : "short read");
    return -1;
  }

  return 0;
}

static int reenable_irq(int fd)
{
  uint32_t enable = 1;
  ssize_t wrote = write(fd, &enable, sizeof(enable));
  if (wrote != (ssize_t)sizeof(enable)) {
    fprintf(stderr, "ERROR: UIO re-enable failed: %s\n", wrote < 0 ? strerror(errno) : "short write");
    return -1;
  }
  return 0;
}

static int run_irq_round(int fd, void *map, unsigned int round)
{
  uint32_t irq_count;

  *reg32(map, REG_IRQ_ACK) = 1;
  *reg32(map, REG_IRQ_ENABLE) = 1;
  *reg32(map, REG_IRQ_TRIGGER) = 1;

  if (wait_for_irq(fd, &irq_count) != 0) {
    return -1;
  }

  printf("irq round %u: count=%" PRIu32 " status=0x%08" PRIx32 "\n",
         round, irq_count, *reg32(map, REG_IRQ_STATUS));

  if ((*reg32(map, REG_IRQ_STATUS) & 1U) == 0) {
    fprintf(stderr, "FAIL: IRQ status was not set after interrupt\n");
    return -1;
  }

  *reg32(map, REG_IRQ_ACK) = 1;
  if ((*reg32(map, REG_IRQ_STATUS) & 1U) != 0) {
    fprintf(stderr, "FAIL: IRQ status did not clear after ack\n");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  const char *uio_path = DEFAULT_UIO;

  if (argc > 2) {
    fprintf(stderr, "Usage: %s [/dev/uioN]\n", argv[0]);
    return 2;
  }
  if (argc == 2) {
    uio_path = argv[1];
  }

  printf("\nCVA6 ZCU111 PL peripheral IRQ test\n");
  printf("uio: %s\n", uio_path);

  int fd = open(uio_path, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "ERROR: open %s failed: %s\n", uio_path, strerror(errno));
    return 1;
  }

  void *map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    fprintf(stderr, "ERROR: mmap %s failed: %s\n", uio_path, strerror(errno));
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

  if (reenable_irq(fd) != 0) {
    return 1;
  }
  if (run_irq_round(fd, map, 1) != 0) {
    return 1;
  }
  if (reenable_irq(fd) != 0) {
    return 1;
  }
  if (run_irq_round(fd, map, 2) != 0) {
    return 1;
  }

  *reg32(map, REG_IRQ_ENABLE) = 0;
  *reg32(map, REG_IRQ_ACK) = 1;

  if (munmap(map, MAP_SIZE) != 0) {
    fprintf(stderr, "WARN: munmap failed: %s\n", strerror(errno));
  }
  close(fd);

  printf("PASS: PL peripheral IRQ test completed\n");
  return 0;
}
