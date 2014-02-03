#ifndef ERRORS_HPP
#define ERRORS_HPP

class error
{
    public:
        int line, column;
        char *msg;

        error(int line, int column, char *message);
        error(char *message);
        ~error(void);

        void emit(const char *prg, const char *tu, const char *source) const;
        void emit(void) const;
};

#endif
