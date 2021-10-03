
// takes array of strings, last element of which is NULL
int stringArrayLen(char** arr) {
    int i = 0;
    while (*(arr + i)) {
        i += 1;
    }
    return i;
}