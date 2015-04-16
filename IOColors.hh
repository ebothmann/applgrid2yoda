// Copied from http://heplife.blogspot.de/2012/05/colorize-c-output-in-unix-i-always.html
// Usage example:
//   std::cout << blue << "This text is blue." << clearatt << std::endl

#ifndef IOCOLORS_HH
#define IOCOLORS_HH

#include <ostream>

/*foreground colors*/
inline std::ostream& red      (std::ostream &s) { s << "\033[31m"; return s; }
inline std::ostream& green    (std::ostream &s) { s << "\033[32m"; return s; }
inline std::ostream& yellow   (std::ostream &s) { s << "\033[33m"; return s; }
inline std::ostream& blue     (std::ostream &s) { s << "\033[34m"; return s; }
inline std::ostream& magenta  (std::ostream &s) { s << "\033[35m"; return s; }
inline std::ostream& cyan     (std::ostream &s) { s << "\033[36m"; return s; }
inline std::ostream& white    (std::ostream &s) { s << "\033[37m"; return s; }
inline std::ostream& black    (std::ostream &s) { s << "\033[0m" ; return s; }

/*background colors*/
inline std::ostream& redbg      (std::ostream &s) { s << "\033[41m"; return s; }
inline std::ostream& greenbg    (std::ostream &s) { s << "\033[42m"; return s; }
inline std::ostream& yellowbg   (std::ostream &s) { s << "\033[43m"; return s; }
inline std::ostream& bluebg     (std::ostream &s) { s << "\033[44m"; return s; }
inline std::ostream& magentabg  (std::ostream &s) { s << "\033[45m"; return s; }
inline std::ostream& cyanbg     (std::ostream &s) { s << "\033[46m"; return s; }
inline std::ostream& whitebg    (std::ostream &s) { s << "\033[47m"; return s; }

/*special*/
inline std::ostream& bold       (std::ostream &s) { s << "\033[1m" ; return s; }
inline std::ostream& underline  (std::ostream &s) { s << "\033[4m" ; return s; }
inline std::ostream& blink      (std::ostream &s) { s << "\033[5m" ; return s; }

/*reset colors to default*/
inline std::ostream& clearatt (std::ostream &s) { s << "\033[0m" ; return s; }

#endif

