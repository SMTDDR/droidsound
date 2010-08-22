/* uadecontrol.c is a helper module to control uade core through IPC:

   Copyright (C) 2005 Heikki Orsila <heikki.orsila@iki.fi>

   This source code module is dual licensed under GPL and Public Domain.
   Hence you may use _this_ module (not another code module) in any way you
   want in your projects.
*/


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <uadecontrol.h>
#include <uadeipc.h>
#include <unixatomic.h>


static void subsong_control(int subsong, int command, struct uade_ipc *ipc);


void uade_change_subsong(int subsong, struct uade_ipc *ipc)
{
  subsong_control(subsong, UADE_COMMAND_CHANGE_SUBSONG, ipc);
}


int uade_read_request(struct uade_ipc *ipc)
{
  int left;
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;

  left = UADE_MAX_MESSAGE_SIZE - sizeof(*um);
  um->msgtype = UADE_COMMAND_READ;
  um->size = 4;
  * (uint32_t *) um->data = htonl(left);
  if (uade_send_message(um, ipc)) {
    fprintf(stderr, "\ncan not send read command\n");
    return 0;
  }

  return left;
}


void uade_send_filter_command(int filter_type, int filter_state,
			      int force_filter, struct uade_ipc *ipc)
{
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;

  /* Note that filter state is not normally forced */
  filter_state = force_filter ? (2 + (filter_state & 1)) : 0;

  *um = (struct uade_msg) {.msgtype = UADE_COMMAND_FILTER, .size = 8};
  ((uint32_t *) um->data)[0] = htonl(filter_type);
  ((uint32_t *) um->data)[1] = htonl(filter_state);
  if (uade_send_message(um, ipc)) {
    fprintf(stderr, "Can not setup filters.\n");
    exit(-1);
  }
}


void uade_send_interpolation_command(const char *mode, struct uade_ipc *ipc)
{
  if (mode != NULL) {
    if (strlen(mode) == 0) {
      fprintf(stderr, "Interpolation mode may not be empty.\n");
      exit(-1);
    }
    if (uade_send_string(UADE_COMMAND_SET_INTERPOLATION_MODE, mode, ipc)) {
      fprintf(stderr, "Can not set interpolation mode.\n");
      exit(-1);
    }
  }
}


static void subsong_control(int subsong, int command, struct uade_ipc *ipc)
{
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;

  assert(subsong >= 0 && subsong < 256);

  *um = (struct uade_msg) {.msgtype = command, .size = 4};
  * (uint32_t *) um->data = htonl(subsong);
  if (uade_send_message(um, ipc) < 0) {
    fprintf(stderr, "Could not changet subsong\n");
    exit(-1);
  }
}


void uade_set_subsong(int subsong, struct uade_ipc *ipc)
{
  subsong_control(subsong, UADE_COMMAND_SET_SUBSONG, ipc);
}


int uade_song_initialization(const char *scorename,
			     const char *playername,
			     const char *modulename,
			     struct uade_ipc *ipc)
{
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;

  if (uade_send_string(UADE_COMMAND_SCORE, scorename, ipc)) {
    fprintf(stderr, "Can not send score name.\n");
    goto cleanup;
  }

  if (uade_send_string(UADE_COMMAND_PLAYER, playername, ipc)) {
    fprintf(stderr, "Can not send player name.\n");
    goto cleanup;
  }

  if (uade_send_string(UADE_COMMAND_MODULE, modulename, ipc)) {
    fprintf(stderr, "Can not send module name.\n");
    goto cleanup;
  }

  if (uade_send_short_message(UADE_COMMAND_TOKEN, ipc)) {
    fprintf(stderr, "Can not send token after module.\n");
    goto cleanup;
  }

  if (uade_receive_message(um, sizeof(space), ipc) <= 0) {
    fprintf(stderr, "Can not receive acknowledgement.\n");
    goto cleanup;
  }

  if (um->msgtype == UADE_REPLY_CANT_PLAY) {
    if (uade_receive_short_message(UADE_COMMAND_TOKEN, ipc)) {
      fprintf(stderr, "Can not receive token in main loop.\n");
      exit(-1);
    }
    return UADECORE_CANT_PLAY;
  }

  if (um->msgtype != UADE_REPLY_CAN_PLAY) {
    fprintf(stderr, "Unexpected reply from uade: %d\n", um->msgtype);
    goto cleanup;
  }

  if (uade_receive_short_message(UADE_COMMAND_TOKEN, ipc) < 0) {
    fprintf(stderr, "Can not receive token after play ack.\n");
    goto cleanup;
  }

  return 0;

 cleanup:
    return UADECORE_INIT_ERROR;
}


void uade_spawn(struct uade_ipc *ipc, pid_t *uadepid, const char *uadename,
		const char *configname)
{
  int forwardfds[2];
  int backwardfds[2];

  if (pipe(forwardfds) != 0 || pipe(backwardfds) != 0) {
    fprintf(stderr, "Can not create pipes: %s\n", strerror(errno));
    exit(-1);
  }
 
  *uadepid = fork();
  if (*uadepid < 0) {
    fprintf(stderr, "Fork failed: %s\n", strerror(errno));
    exit(-1);
  }
  if (*uadepid == 0) {
    int fd;
    char instr[32], outstr[32];
    int maxfds;

    if ((maxfds = sysconf(_SC_OPEN_MAX)) < 0) {
      maxfds = 1024;
      fprintf(stderr, "Getting max fds failed. Using %d.\n", maxfds);
    }

    /* close everything else but stdin, stdout, stderr, and in/out fds */
    for (fd = 3; fd < maxfds; fd++) {
      if (fd != forwardfds[0] && fd != backwardfds[1])
	atomic_close(fd);
    }
    /* give in/out fds as command line parameters to the uade process */
    snprintf(instr, sizeof(instr), "fd://%d", forwardfds[0]);
    snprintf(outstr, sizeof(outstr), "fd://%d", backwardfds[1]);
    execlp(uadename, uadename, "-i", instr, "-o", outstr, (char *) NULL);
    fprintf(stderr, "Execlp failed: %s\n", strerror(errno));
    abort();
  }

  /* close fd that uade reads from and writes to */
  if (atomic_close(forwardfds[0]) < 0 || atomic_close(backwardfds[1]) < 0) {
    fprintf(stderr, "Could not close uade fds: %s\n", strerror(errno));
    kill (*uadepid, SIGTERM);
    *uadepid = 0;
    exit(-1);
  }

  do {
    char input[64], output[64];
    snprintf(output, sizeof output, "fd://%d", forwardfds[1]);
    snprintf(input, sizeof input, "fd://%d", backwardfds[0]);
    uade_set_peer(ipc, 1, input, output);
  } while (0);

  if (uade_send_string(UADE_COMMAND_CONFIG, configname, ipc)) {
    fprintf(stderr, "Can not send config name: %s\n", strerror(errno));
    kill(*uadepid, SIGTERM);
    *uadepid = 0;
    exit(-1);
  }
}

