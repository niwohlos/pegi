#include <cstdio>
#include <cstring>

#include "error.hpp"


error::error(int l, int c, char *m):
    line(l),
    column(c),
    msg(m)
{}


error::error(char *m):
    line(-1),
    column(-1),
    msg(m)
{}


error::~error(void)
{
    delete msg;
}


void error::emit(const char *prg, const char *tu, const char *source) const
{
    fprintf(stderr, "%s: %s:%i:%i: %s\n", prg, tu, line, column, msg);

    if (line < 1)
        return;

    for (int i = 1; *source && (i < line); source++)
        if (*source == '\n')
            i++;

    if (!*source)
        return;

    const char *line_end;
    for (line_end = source; *line_end && (*line_end != '\n'); line_end++);

    char error_line[line_end - source + 1];
    memcpy(error_line, source, line_end - source);
    error_line[line_end - source] = 0;

    fprintf(stderr, "%s\n%*c\n", error_line, column, '^');
}
