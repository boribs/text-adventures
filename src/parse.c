static char *trim(char *s, size_t len) {
    char tmp[len + 1];
    size_t i;

    for (i = 0; i < len; ++i) { // remove left spaces
        if (s[i] != ' ') {
            memset(tmp, 0, (len + 1) * sizeof(char));
            strcpy(tmp, s + i);
            break;
        }
    }

    for (i = strlen(tmp) - 1; i > 0; --i) { // remove right spaces/newlines
        if (tmp[i] != ' ' && tmp[i] != '\n') {
            memset(s, 0, (len + 1) * sizeof(char));
            strncpy(s, tmp, i + 1);
            break;
        }
    }

    return realloc(s, (strlen(s) + 1) * sizeof(char));
}
