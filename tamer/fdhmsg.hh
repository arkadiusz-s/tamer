#ifndef TAMER_FDHELP_HH
#define TAMER_FDHELP_HH 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <stdint.h>

#define FDH_CLONE 100
#define FDH_KILL  101
#define FDH_OPEN  102
#define FDH_STAT  103
#define FDH_READ  104
#define FDH_WRITE 105

/* clone
 * > msg: req
 * < msg: errno | pid; [anc: fd]
 *
 * kill
 * > msg: req
 *
 * open 
 *  > msg: req | int (flag) | mode | filename
 *  < msg: errno; [anc: fd]
 *
 * stat
 *  > msg: req, anc: fd
 *  < msg: errno | [stat]
 *
 * read
 *  > msg: req | size; anc: fd
 *  < sendfile: pipe, fd, NULL, size
 *
 * write
 *  > msg: req | size; anc: fd
 *  > write: pipe 
 *  < read: pipe -> write: fd
 */

union fdh_msg {
  struct {
    uint8_t req;
    union {
      size_t size;
      int flags;
    };
    mode_t mode;
  } query;
  struct {
    int err;
    pid_t pid;
  } reply;
};//stat and fname placed at the end

typedef union fdh_msg fdh_msg;

#define FDH_MSG_SIZE sizeof(union fdh_msg)

int fdh_send(int fd, int fd_to_send, char * buf, size_t len);
int fdh_recv(int fd, int * fd_to_recv, char * buf, size_t len);

#endif /*TAMER_FDHELP_HH*/
