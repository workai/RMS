#ifndef GETOPT_H
#define GETOPT_H

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef __Win32__

extern char *optarg;
extern int optreset;
extern int optind;
extern int opterr;
extern int optopt;
int getopt(int argc, char* const *argv, const char *optstr);

//#endif /* WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* GETOPT_H */
