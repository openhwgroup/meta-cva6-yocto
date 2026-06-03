#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define AES_UIO_NAME "zcu111-aes-gcm"
#define MAP_SIZE 0x1000UL

#define REG_ID 0x00
#define REG_VERSION 0x04
#define REG_CONTROL 0x08
#define REG_STATUS 0x0c
#define REG_IRQ_ENABLE 0x10
#define REG_IRQ_STATUS 0x14
#define REG_IRQ_ACK 0x18
#define REG_KEY0 0x20
#define REG_IV0 0x40
#define REG_AAD_BYTES 0x4c
#define REG_AAD0 0x50
#define REG_DATA_BYTES 0x60
#define REG_DATA_IN0 0x64
#define REG_DATA_OUT0 0x74
#define REG_TAG0 0x84

#define CONTROL_START (1U << 0)
#define CONTROL_DECRYPT (1U << 1)
#define CONTROL_CLEAR (1U << 2)

#define STATUS_DONE (1U << 1)
#define STATUS_TAG_VALID (1U << 2)

#define EXPECTED_ID 0x4147434dU
#define EXPECTED_VERSION 0x00010000U

struct kat {
  const char *name;
  uint8_t key[32];
  uint8_t iv[12];
  uint8_t aad[16];
  uint32_t aad_bytes;
  uint8_t plaintext[16];
  uint8_t ciphertext[16];
  uint32_t data_bytes;
  uint8_t tag[16];
};

static const struct kat kats[] = {
    {
        .name = "empty plaintext and aad",
        .tag = {0x53, 0x0f, 0x8a, 0xfb, 0xc7, 0x45, 0x36, 0xb9,
                0xa9, 0x63, 0xb4, 0xf1, 0xc4, 0xcb, 0x73, 0x8b},
    },
    {
        .name = "single plaintext block",
        .data_bytes = 16,
        .ciphertext = {0xce, 0xa7, 0x40, 0x3d, 0x4d, 0x60, 0x6b, 0x6e,
                       0x07, 0x4e, 0xc5, 0xd3, 0xba, 0xf3, 0x9d, 0x18},
        .tag = {0xd0, 0xd1, 0xc8, 0xa7, 0x99, 0x99, 0x6b, 0xf0,
                0x26, 0x5b, 0x98, 0xb5, 0xd4, 0x8a, 0xb9, 0x19},
    },
};

static volatile uint32_t *reg32(void *base, uint32_t offset)
{
  return (volatile uint32_t *)((volatile uint8_t *)base + offset);
}

static uint32_t pack_be32(const uint8_t *src)
{
  return ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) |
         ((uint32_t)src[2] << 8) | (uint32_t)src[3];
}

static void unpack_be32(uint32_t word, uint8_t *dst)
{
  dst[0] = (uint8_t)(word >> 24);
  dst[1] = (uint8_t)(word >> 16);
  dst[2] = (uint8_t)(word >> 8);
  dst[3] = (uint8_t)word;
}

static void write_words(void *map, uint32_t offset, const uint8_t *data,
                        size_t bytes)
{
  for (size_t i = 0; i < bytes; i += 4) {
    *reg32(map, offset + (uint32_t)i) = pack_be32(&data[i]);
  }
}

static void read_words(void *map, uint32_t offset, uint8_t *data, size_t bytes)
{
  for (size_t i = 0; i < bytes; i += 4) {
    unpack_be32(*reg32(map, offset + (uint32_t)i), &data[i]);
  }
}

static int discover_uio(char *path, size_t path_len)
{
  DIR *dir = opendir("/sys/class/uio");
  struct dirent *ent;

  if (dir == NULL) {
    fprintf(stderr, "ERROR: open /sys/class/uio failed: %s\n",
            strerror(errno));
    return -1;
  }

  while ((ent = readdir(dir)) != NULL) {
    char name_path[PATH_MAX];
    char name[128];
    FILE *file;

    if (strncmp(ent->d_name, "uio", 3) != 0) {
      continue;
    }

    snprintf(name_path, sizeof(name_path), "/sys/class/uio/%s/name",
             ent->d_name);
    file = fopen(name_path, "r");
    if (file == NULL) {
      continue;
    }
    if (fgets(name, sizeof(name), file) != NULL) {
      name[strcspn(name, "\r\n")] = '\0';
      if (strcmp(name, AES_UIO_NAME) == 0) {
        snprintf(path, path_len, "/dev/%s", ent->d_name);
        fclose(file);
        closedir(dir);
        return 0;
      }
    }
    fclose(file);
  }

  closedir(dir);
  fprintf(stderr, "ERROR: no UIO device named '%s' found\n", AES_UIO_NAME);
  return -1;
}

static int enable_uio_irq(int fd)
{
  uint32_t enable = 1;
  ssize_t wrote = write(fd, &enable, sizeof(enable));

  if (wrote != (ssize_t)sizeof(enable)) {
    fprintf(stderr, "ERROR: UIO enable failed: %s\n",
            wrote < 0 ? strerror(errno) : "short write");
    return -1;
  }
  return 0;
}

static int wait_uio_irq(int fd)
{
  uint32_t count;
  ssize_t got;

  do {
    got = read(fd, &count, sizeof(count));
  } while (got < 0 && errno == EINTR);

  if (got != (ssize_t)sizeof(count)) {
    fprintf(stderr, "ERROR: UIO wait failed: %s\n",
            got < 0 ? strerror(errno) : "short read");
    return -1;
  }
  return 0;
}

static int run_operation(int fd, void *map, const struct kat *kat, int decrypt)
{
  const uint8_t *input = decrypt ? kat->ciphertext : kat->plaintext;
  const uint8_t *expected = decrypt ? kat->plaintext : kat->ciphertext;
  uint8_t output[16] = {0};
  uint8_t tag[16] = {0};
  uint32_t status;

  *reg32(map, REG_CONTROL) = CONTROL_CLEAR;
  *reg32(map, REG_IRQ_ACK) = 1;
  write_words(map, REG_KEY0, kat->key, sizeof(kat->key));
  write_words(map, REG_IV0, kat->iv, sizeof(kat->iv));
  *reg32(map, REG_AAD_BYTES) = kat->aad_bytes;
  write_words(map, REG_AAD0, kat->aad, sizeof(kat->aad));
  *reg32(map, REG_DATA_BYTES) = kat->data_bytes;
  write_words(map, REG_DATA_IN0, input, sizeof(kat->plaintext));
  *reg32(map, REG_IRQ_ENABLE) = 1;

  if (enable_uio_irq(fd) != 0) {
    return -1;
  }

  *reg32(map, REG_CONTROL) = CONTROL_START |
                             (decrypt ? CONTROL_DECRYPT : 0U);

  if (wait_uio_irq(fd) != 0) {
    return -1;
  }

  status = *reg32(map, REG_STATUS);
  if ((status & (STATUS_DONE | STATUS_TAG_VALID)) !=
      (STATUS_DONE | STATUS_TAG_VALID)) {
    fprintf(stderr, "FAIL: %s %s status=0x%08" PRIx32 "\n", kat->name,
            decrypt ? "decrypt" : "encrypt", status);
    return -1;
  }

  read_words(map, REG_DATA_OUT0, output, sizeof(output));
  read_words(map, REG_TAG0, tag, sizeof(tag));
  if ((kat->data_bytes != 0 && memcmp(output, expected, kat->data_bytes) != 0) ||
      memcmp(tag, kat->tag, sizeof(tag)) != 0) {
    fprintf(stderr, "FAIL: %s %s result mismatch\n", kat->name,
            decrypt ? "decrypt" : "encrypt");
    return -1;
  }

  *reg32(map, REG_IRQ_ACK) = 1;
  printf("PASS: %s %s\n", kat->name, decrypt ? "decrypt" : "encrypt");
  return 0;
}

int main(int argc, char **argv)
{
  char detected_path[PATH_MAX];
  const char *uio_path;
  int fd;
  void *map;

  if (argc > 2) {
    fprintf(stderr, "Usage: %s [/dev/uioN]\n", argv[0]);
    return 2;
  }
  if (argc == 2) {
    uio_path = argv[1];
  } else {
    if (discover_uio(detected_path, sizeof(detected_path)) != 0) {
      return 1;
    }
    uio_path = detected_path;
  }

  printf("\nCVA6 ZCU111 AES-GCM MMIO/IRQ KAT\n");
  printf("uio: %s\n", uio_path);

  fd = open(uio_path, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "ERROR: open %s failed: %s\n", uio_path, strerror(errno));
    return 1;
  }

  map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    fprintf(stderr, "ERROR: mmap failed: %s\n", strerror(errno));
    close(fd);
    return 1;
  }

  if (*reg32(map, REG_ID) != EXPECTED_ID ||
      *reg32(map, REG_VERSION) != EXPECTED_VERSION) {
    fprintf(stderr, "FAIL: unexpected AES-GCM ID/version\n");
    return 1;
  }

  if (run_operation(fd, map, &kats[0], 0) != 0 ||
      run_operation(fd, map, &kats[1], 0) != 0 ||
      run_operation(fd, map, &kats[1], 1) != 0) {
    return 1;
  }

  *reg32(map, REG_IRQ_ENABLE) = 0;
  *reg32(map, REG_IRQ_ACK) = 1;
  munmap(map, MAP_SIZE);
  close(fd);
  printf("PASS: AES-256-GCM MMIO/IRQ known-answer tests completed\n");
  return 0;
}
