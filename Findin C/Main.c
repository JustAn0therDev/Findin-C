#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#pragma warning(disable : 4996)

size_t FindInCurrentDirectory(char* path, char* search, char* fileExtension, char* currentSubDirectory);
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
	char* search = "o";

	char currentDirectoryBuffer[MAX_PATH];

	strcpy(currentDirectoryBuffer, "D:\\repos");
#endif
	char* innermostDirectory = GetInnermostFileNameInPath(currentDirectoryBuffer);

	size_t occurrences = FindInCurrentDirectory(currentDirectoryBuffer, search, fileExtension, innermostDirectory);

	printf("Found a total of %zd occurrences.\n", occurrences);

	return 0;
}

size_t FindInCurrentDirectory(char* path, char* search, char* fileExtension, char* currentSubDirectory) {
	WIN32_FIND_DATA w32fd = { 0 };
	HANDLE hFind;

	char directoryBufferForLookup[MAX_PATH];
	char directoryBuffer[MAX_PATH];

	strcpy(directoryBufferForLookup, path);

	strcpy(directoryBuffer, directoryBufferForLookup);

	strcat(directoryBufferForLookup, "\\");
	strcat(directoryBufferForLookup, "*.*");

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
			strcpy(subDirectoryToShow, currentSubDirectory);
			strcat(subDirectoryToShow, "/");
			strcat(subDirectoryToShow, fileName);
		}
		else {
			strcpy(subDirectoryToShow, fileName);
		}

		if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			char subDirectoryPath[MAX_PATH] = { 0 };

			strcpy(subDirectoryPath, directoryBuffer);
			strcat(subDirectoryPath, "\\");
			strcat(subDirectoryPath, fileName);

			totalOccurrences += FindInCurrentDirectory(subDirectoryPath, search, fileExtension, subDirectoryToShow);
		}
		else if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE && EndsWith(fileName, fileExtension)) {
			char path[MAX_PATH] = { 0 };

			strcpy(path, directoryBuffer);

			strcat(path, "\\");
			strcat(path, fileName);

			totalOccurrences += FindIn(search, path, subDirectoryToShow);
		}
	} while (FindNextFileA(hFind, (LPWIN32_FIND_DATAA)&w32fd));

	FindClose(hFind);

	return totalOccurrences;
}

size_t FindIn(char* search, char* filePath, char* subDirectoryToShow) {
	FILE* fp = fopen(filePath, "r");

	if (!fp) {
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

		if (line == 0) {
			line = malloc(sizeof(char) + 1);

			if (line == 0) {
				exit(1);
			}

			*(line + lineSize) = '\0';

			strncpy(line, &c, 1);
		}
		else {
			char* oldLineBuffer = line;
			line = realloc(line, sizeof(char) * (lineSize + 1));

			if (line == 0) {
				free(oldLineBuffer);
				printf("Realloc failed.\n");
				exit(1);
			}

			strncat(line, &c, 1);
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