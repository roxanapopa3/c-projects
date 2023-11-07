/* SPDX-License-Identifier: BSD-3-Clause */

#include <string.h>

char *strcpy(char *destination, const char *source) {
  int i = 0, j = 0;
  int len1 = strlen(destination);
  while (len1 != 0) {
    destination[j] = '\0';
    len1--;
    j++;
  }
  while (source[i] != '\0') {
    destination[i] = source[i];
    i++;
  }
  destination[i] = '\0';
  return destination;
}

char *strncpy(char *destination, const char *source, size_t len) {
  int copy = len;
  int i = 0;
  int len2 = strlen(source);
  while (source[i] != '\0' && len) {
    destination[i] = source[i];
    i++;
    len--;
  }
  if (copy > len2)
    destination[i] = '\0';
  return destination;
}

char *strcat(char *destination, const char *source) {
  int i = strlen(destination);
  int j = 0;
  while (source[j] != '\0') {
    destination[i] = source[j];
    i++;
    j++;
  }
  destination[i] = '\0';
  return destination;
}

char  *strncat(char *destination, const char *source, size_t len) {
  int i, j;
  for (i = 0; destination[i] != '\0'; i++) {}
  for (j = 0; source[j] != '\0' && j < (int)len; j++) {
    destination[i + j] = source[j];
  }
  destination[i + j] = '\0';
  return destination;
}

int strcmp(const char *str1, const char *str2) {
  int i = 0;
  while ((str1[i] != '\0' && str2[i] != '\0') && str1[i] == str2[i]) {
    i++;
  }
  if (str1[i] == str2[i]) {
    return 0;
  }
  if (str1[i] > str2[i]) {
    return 1;
  }
  return -1;
}

int strncmp(const char *str1, const char *str2, size_t len) {
  while (len && *str1 && (*str1 == *str2)) {
    ++str1;
    ++str2;
    --len;
  }
  if (len == 0) {
    return 0;
  }
  return ( *(unsigned char *) str1 - *(unsigned char *) str2);
}

size_t strlen(const char *str) {
  size_t i = 0;

  for (;*str != '\0'; str++, i++)
  ;

  return i;
}

char *strchr(const char *str, int c) {
  char ch = c;
  int i = 0;
  while (str[i] != '\0') {
    if (str[i] == ch)
      return (char *) str + i;
    i++;
  }
  return NULL;
}

char *strrchr(const char *str, int c) {
  char *p = NULL;
  char ch = c;
  while (*str != '\0') {
    if (*str == ch)
      p = (char *) str;
    str++;
  }
  return p;
}

char *strstr(const char *str1, const char *str2) {
  int len = strlen(str2);
  while (*str1 != '\0') {
    if (!memcmp(str1, str2, len)) {
      return (char *) str1;
    }
    str1++;
  }
  return NULL;
}

char * strrstr(const char *str1, const char *str2) {
  char *p = NULL;
  char *result = NULL;
  while (*str1 != '\0') {
    p = strstr(str1, str2);
    if (p == NULL)
      break;
    result = p;
    str1 = p + 1;
  }
  return result;
}

void *memcpy(void *destination, const void *source, size_t num) {
  char *dest = (char *) destination;
  const char *src = (const char *) source;
  int i = 0;
  while (num != 0) {
    dest[i] = src[i];
    i++;
    num--;
  }

  return destination;
}

void *memmove(void *destination, const void *source, size_t num) {
  char *dst = destination;
  char *src = (char *)source;
  int i = 0;
  while (num != 0) {
    dst[i] = src[i];
    i++;
    num--;
  }
  return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
  int status = 0;
  char *p1 = (char *) ptr1;
  char *p2 = (char *) ptr2;
  if (ptr1 == ptr2)
    return 0;
  while (num != 0) {
    if (*p1 > *p2) {
      status = 1;
    }
    if (*p1 < *p2) {
      status = -1;
    }
    num--;
    p1++;
    p2++;
  }
  return status;
}

void *memset(void *source, int value, size_t num) {
  char *p = source;
  while (num != 0) {
    *p = (char) value;
    p++;
    num--;
  }
  return (void *) source;
}
