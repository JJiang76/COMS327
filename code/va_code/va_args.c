#include <stdio.h>
#include <stdarg.h> //needed for variadic functions
#include <math.h>

//cannot have function:  returntype func(...)
//must have at least one real argument, and "..." must be last

//"poor man's" printf - extracts variable arguments
void foo(const char *format, ...) {
  va_list ap; //data structure for parsing arguments on stack

  char *s;
  int d;
  float f;
  char c;

  va_start(ap, format); //inits ap; format is last arg before va_list starts

  //format string - skip the %'s
  //steps through each character in the string until null byte is reached
  while(*format) {
    switch (*format++) {
      case 's':
        s = va_arg(ap, char *); //extracts next arg from va_list - va_arg is a MACRO -
                                //second parameter is type being extracted
        printf("string: %s\n", s);
        break;
      case 'd':
        d = va_arg(ap, int);
        printf("int: %d\n", d);
        break;
      case 'f':
        f = va_arg(ap, double); //needs double because the compiler assumes args in ...
                                //is the most promoted type
                                //we then demote back to float, same as chars
        printf("float: %f\n", f);
        break;
      case 'c':
        c = va_arg(ap, int);
        printf("char: %c\n", c);
        break;
      default:
        fprintf(stderr, "Invalid format specifier: %c\n", *(format - 1));
    }
  }

  va_end(ap); //deinit
}

#define FOO(...) printf(__VA_ARGS__) //C99 - variatic macros

int main(int argc, char * argv[]) {
  foo("sfgdc", "Hello World!", M_PI, 42, 'q');

  FOO("%d", 3);

  return 0;
}
