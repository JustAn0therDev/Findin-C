#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

size_t FindInCurrentDirectory(char* path, char* search, char* fileExtension, char* currentSubDirectory, size_t* files);
size_t FindIn(char* search, char* filePath, char* subDirectoryToShow);
inline int EndsWith(char* str, char* subStr);
inline char* GetInnermostFileNameInPath(char* filePath);

int main(int argc, char** argv) {
#ifndef DEBUG
  if (argc != 3) {
    printf("A file extension and a search argument must be present.");
    return 1;
  }

  char* fileExtension = *(argv + 1);
  char* search = *(argv + 2);

  char currentDirectoryBuffer[MAX_PATH];

  unsigned long bytesWritten = GetCurrentDirectoryA(MAX_PATH, currentDirectoryBuffer);

  if (bytesWritten == 0) {
    printf("Couldn't find current directory");
    return 1;
  }
#else
  char* fileExtension = ".cs";
  char* search = "void";

  char currentDirectoryBuffer[MAX_PATH];
  char* path = "D:\\repos\\PokemonAdventureGame";

  strncpy_s(currentDirectoryBuffer, MAX_PATH, path, strlen(path));
#endif
  char* innermostDirectory = GetInnermostFileNameInPath(currentDirectoryBuffer);

  size_t files = 0;

  size_t occurrences = FindInCurrentDirectory(currentDirectoryBuffer, search, fileExtension, innermostDirectory, &files);

  printf("Found a total of %zd occurrences. Searched in %zd files.\n", occurrences, files);

  return 0;
}

size_t FindInCurrentDirectory(char* path, char* search, char* fileExtension, char* currentSubDirectory, size_t* files) {
  WIN32_FIND_DATA w32fd = { 0 };
  HANDLE hFind;

  char directoryBufferForLookup[MAX_PATH];
  char directoryBuffer[MAX_PATH];

  strncpy_s(directoryBufferForLookup, MAX_PATH, path, strlen(path));

  strncpy_s(directoryBuffer, MAX_PATH, directoryBufferForLookup, MAX_PATH);

  strncat_s(directoryBufferForLookup, MAX_PATH, "\\", 2);
  strncat_s(directoryBufferForLookup, MAX_PATH, "*.*", 3);

  if ((hFind = FindFirstFileA(directoryBufferForLookup, (LPWIN32_FIND_DATAA)&w32fd)) == INVALID_HANDLE_VALUE) {
    printf("Couldn't find any files with this extension");
    exit(1);
  }

  size_t totalOccurrences = 0;

  do {
    char* fileName = (char*)w32fd.cFileName;

    if (strcmp(".", fileName) == 0 || strcmp("..", fileName) == 0) {
      continue;
    }

    char subDirectoryToShow[MAX_PATH] = { 0 };

    if (currentSubDirectory) {
      strncpy_s(subDirectoryToShow, MAX_PATH, currentSubDirectory, strlen(currentSubDirectory));
      strncat_s(subDirectoryToShow, MAX_PATH, "/", 1);
      strncat_s(subDirectoryToShow, MAX_PATH, fileName, strlen(fileName));
    }
    else {
      strncpy_s(subDirectoryToShow, MAX_PATH, fileName, strlen(fileName));
    }

    if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      char subDirectoryPath[MAX_PATH] = { 0 };

      strncpy_s(subDirectoryPath, MAX_PATH, directoryBuffer, strlen(directoryBuffer));
      strncat_s(subDirectoryPath, MAX_PATH, "\\", 2);
      strncat_s(subDirectoryPath, MAX_PATH, fileName, strlen(fileName));

      totalOccurrences += FindInCurrentDirectory(subDirectoryPath, search, fileExtension, subDirectoryToShow, files);
    }
    else if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE && EndsWith(fileName, fileExtension)) {
      char path[MAX_PATH] = { 0 };

      strncpy_s(path, MAX_PATH, directoryBuffer, strlen(directoryBuffer));

      strncat_s(path, MAX_PATH, "\\", 2);
      strncat_s(path, MAX_PATH, fileName, strlen(fileName));

      totalOccurrences += FindIn(search, path, subDirectoryToShow);
      (*files)++;
    }
  } while (FindNextFileA(hFind, (LPWIN32_FIND_DATAA)&w32fd));

  FindClose(hFind);

  return totalOccurrences;
}

size_t FindIn(char* search, char* filePath, char* subDirectoryToShow) {
  FILE* fp = 0;

  if (fopen_s(&fp, filePath, "r") != 0) {
    printf("Couldn't open file: %s\n", filePath);
    return 0;
  }

  rewind(fp);

  size_t charIndex = 0;
  size_t lineNumber = 1;
  char c;
  char* line = 0;
  size_t lineSize = 0;
  size_t searchIndex = 0;
  size_t lastSearchIndexFound = 0;
  size_t foundInLine = 0;
  size_t occurrences = 0;

  while ((c = fgetc(fp)) != EOF) {
    if (lineSize == 0 && (c == ' ' || c == '\t')) {
      charIndex++;
      continue;
    }

    // A char of the string was found, the searchIndex is smaller than the size
    // of the search string (didn't find everything yet), AND 
         // (the currentIndex is + 1 bigger than the last one OR it's the first char)
         if (search[searchIndex] == c && searchIndex < strlen(search) &&
             (charIndex == (lastSearchIndexFound + 1) || lastSearchIndexFound == 0)) {
           lastSearchIndexFound = charIndex;
           searchIndex++;
         }
         else { // Nothing was found for now, so reset the state of the indices.
           searchIndex = 0;
           lastSearchIndexFound = 0;
         }

    // The searchIndex is now of the same size as the string.
    // Reset the state and look for more occurrences in the same line.
    if (searchIndex == strlen(search)) {
      foundInLine++;
      searchIndex = 0;
    }

    lineSize++;

    char toAppend[2] = { c, '\0'};

    if (line == 0) {
      line = malloc(sizeof(c) * 2);

      if (line == 0) {
        exit(1);
      }

      *(line + lineSize) = '\0';

      strncpy_s(line, lineSize + 1, toAppend, 1);
    }
    else {
      char* oldLineBuffer = line;
      line = realloc(line, sizeof(c) * (lineSize + 1));

      if (line == 0) {
        free(oldLineBuffer);
        printf("Realloc failed.\n");
        exit(1);
      }

      strncat_s(line, lineSize + 1, toAppend, 1);
    }

    if (c == '\n') {
      lineNumber++;

      if (foundInLine) {
        occurrences += foundInLine;
        char* fileNameFromPath = GetInnermostFileNameInPath(filePath);
        printf("[%s] at line %zd: %s", subDirectoryToShow, lineNumber, line);
      }

      // Reset the state of everything because we count info per line.
      free(line);
      line = 0;
      lineSize = 0;
      searchIndex = 0;
      lastSearchIndexFound = 0;
      foundInLine = 0;
    }

    charIndex++;
  }

  if (line) {
    free(line);
  }

  fclose(fp);

  return occurrences;
}

int EndsWith(char* c, char* subStr) {
  int subStrIndex = (int)strlen(subStr) - 1;
  char* subStrEnd = subStr + (strlen(subStr) - 1);
  char* cEnd = c + (strlen(c) - 1);

  while (subStrIndex > -1) {
    if (*(subStrEnd) != (*cEnd)) {
      return 0;
    }

    subStrEnd--;
    cEnd--;
    subStrIndex--;
  }

  return 1;
}

char* GetInnermostFileNameInPath(char* path) {
  if (path == 0) {
    return 0;
  }

  char* pathPtr = path + strlen(path);

  while (*pathPtr != '\\') {
    pathPtr--;
  }

  return pathPtr + sizeof(*path);
}
