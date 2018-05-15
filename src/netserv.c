// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// netserv.c
// ============================================================================

#include "libgame/platform.h"

#if defined(NETWORK_AVALIABLE)

#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#include "main.h"

#include "libgame/libgame.h"

#include "netserv.h"

static int clients = 0;
static int onceonly = 0;

struct NetworkServerPlayerInfo
{
  TCPsocket fd;
  char player_name[16];
  unsigned char number;
  struct NetworkServerPlayerInfo *next;
  char active;
  char introduced;
  unsigned char readbuffer[MAX_BUFFER_SIZE];
  unsigned char writbuffer[MAX_BUFFER_SIZE];
  int nread, nwrite;
  byte action;
  boolean action_received;
};

static struct NetworkServerPlayerInfo *first_player = NULL;

#define NEXT(player) ((player)->next ? (player)->next : first_player)

/* TODO: peer address */
static TCPsocket lfd;		/* listening socket */
static SDLNet_SocketSet fds;	/* socket set */

static unsigned char realbuffer[512], *buffer = realbuffer + 4;

static int interrupt;

static unsigned int ServerFrameCounter = 0;

static void addtobuffer(struct NetworkServerPlayerInfo *player,
			unsigned char *b, int len)
{
  if (player->nwrite + len >= MAX_BUFFER_SIZE)
    Error(ERR_EXIT_NETWORK_SERVER,
	  "internal error: network send buffer overflow");

  memcpy(player->writbuffer + player->nwrite, b, len);
  player->nwrite += len;
}

static void flushuser(struct NetworkServerPlayerInfo *player)
{
  if (player->nwrite)
  {
    SDLNet_TCP_Send(player->fd, player->writbuffer, player->nwrite);

    player->nwrite = 0;
  }
}

static void broadcast(struct NetworkServerPlayerInfo *except,
		      int len, int activeonly)
{
  struct NetworkServerPlayerInfo *player;

  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (unsigned char)len;
  for (player = first_player; player; player = player->next)
    if (player != except && player->introduced &&
	(player->active || !activeonly))
      addtobuffer(player, realbuffer, 4 + len);
}

static void sendtoone(struct NetworkServerPlayerInfo *to, int len)
{
  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (unsigned char)len;
  addtobuffer(to, realbuffer, 4 + len);
}

static void RemovePlayer(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "dropping client %d (%s)",
	  player->number, player->player_name);

  if (player == first_player)
    first_player = player->next;
  else
  {
    for (v = first_player; v; v = v->next)
    {
      if (v->next && v->next == player)
      {
	v->next = player->next;
	break;
      }
    }
  }

  SDLNet_TCP_DelSocket(fds, player->fd);
  SDLNet_TCP_Close(player->fd);

  if (player->introduced)
  {
    buffer[0] = player->number;
    buffer[1] = OP_PLAYER_DISCONNECTED;
    broadcast(player, 2, 0);
  }

  free(player);
  clients--;

  if (onceonly && clients == 0)
  {
    if (options.verbose)
    {
      Error(ERR_NETWORK_SERVER, "no clients left");
      Error(ERR_NETWORK_SERVER, "aborting");
    }
    exit(0);
  }
}

static void AddPlayer(TCPsocket fd)
{
  struct NetworkServerPlayerInfo *player, *v;
  unsigned char nxn;
  boolean again = TRUE;

  player = checked_malloc(sizeof (struct NetworkServerPlayerInfo));

  player->fd = fd;
  player->player_name[0] = 0;
  player->next = first_player;
  player->active = 0;
  player->nread = 0;
  player->nwrite = 0;
  player->introduced = 0;
  player->action = 0;
  player->action_received = FALSE;

  SDLNet_TCP_AddSocket(fds, fd);

  first_player = player;

  nxn = 1;

  while (again)
  {
    again = FALSE;
    v = player->next;

    while (v)
    {
      if (v->number == nxn)
      {
	nxn++;

	again = TRUE;
	break;
      }
      v = v->next;
    }
  }

  player->number = nxn;
  clients++;

  buffer[0] = 0;
  buffer[1] = OP_YOUR_NUMBER;
  buffer[2] = player->number;
  sendtoone(player, 3);
}

static void Handle_OP_PROTOCOL_VERSION(struct NetworkServerPlayerInfo *player,
				       unsigned int len)
{
  if (len != 5 ||
      buffer[2] != PROTOCOL_VERSION_1 ||
      buffer[3] != PROTOCOL_VERSION_2)
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) has wrong protocol version %d.%d.%d",
	    player->number, player->player_name, buffer[2], buffer[3], buffer[4]);

    buffer[0] = 0;
    buffer[1] = OP_BAD_PROTOCOL_VERSION;
    buffer[2] = PROTOCOL_VERSION_1;
    buffer[3] = PROTOCOL_VERSION_2;
    buffer[4] = PROTOCOL_VERSION_3;
    sendtoone(player, 5);
    flushuser(player);

    RemovePlayer(player);
    interrupt = 1;
  }
  else
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) uses protocol version %d.%d.%d",
	    player->number, player->player_name, buffer[2], buffer[3], buffer[4]);
  }
}

static void Handle_OP_NUMBER_WANTED(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  int client_nr = player->number;
  int nr_wanted = buffer[2];
  int nr_is_free = 1;

  if (options.verbose)
      Error(ERR_NETWORK_SERVER, "client %d (%s) wants to switch to # %d",
	    player->number, player->player_name, nr_wanted);

  for (v = first_player; v; v = v->next)
  {
    if (v->number == nr_wanted)
    {
      nr_is_free = 0;
      break;
    }
  }

  if (options.verbose)
  {
    if (nr_is_free)
      Error(ERR_NETWORK_SERVER, "client %d (%s) switches to # %d",
	    player->number, player->player_name, nr_wanted);
    else if (player->number == nr_wanted)
      Error(ERR_NETWORK_SERVER, "client %d (%s) already has # %d",
	    player->number, player->player_name, nr_wanted);
    else
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) cannot switch (client %d already exists)",
	    player->number, player->player_name, nr_wanted);
  }

  if (nr_is_free)
    player->number = nr_wanted;

  buffer[0] = client_nr;
  buffer[1] = OP_NUMBER_WANTED;
  buffer[2] = nr_wanted;
  buffer[3] = player->number;

  /*
  sendtoone(player, 4);
  */

  broadcast(NULL, 4, 0);
}

static void Handle_OP_PLAYER_NAME(struct NetworkServerPlayerInfo *player,
				  unsigned int len)
{
  struct NetworkServerPlayerInfo *v;
  int i;

  if (len>16)
    len=16;
  memcpy(player->player_name, &buffer[2], len-2);
  player->player_name[len-2] = 0;
  for (i = 0; i < len - 2; i++)
  {
    if (player->player_name[i] < ' ' || 
	((unsigned char)(player->player_name[i]) > 0x7e &&
	 (unsigned char)(player->player_name[i]) <= 0xa0))
    {
      player->player_name[i] = 0;
      break;
    }
  }

  if (!player->introduced)
  {
    buffer[0] = player->number;
    buffer[1] = OP_PLAYER_CONNECTED;
    broadcast(player, 2, 0);
  }
	      
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d calls itself \"%s\"",
	  player->number, player->player_name);
  buffer[1] = OP_PLAYER_NAME;
  broadcast(player, len, 0);

  if (!player->introduced)
  {
    for (v = first_player; v; v = v->next)
    {
      if (v != player && v->introduced)
      {
	buffer[0] = v->number;
	buffer[1] = OP_PLAYER_CONNECTED;
	sendtoone(player, 2);
	buffer[1] = OP_PLAYER_NAME;
	memcpy(&buffer[2], v->player_name, 14);
	sendtoone(player, 2+strlen(v->player_name));
      }
    }
  }

  player->introduced = 1;
}

static void Handle_OP_START_PLAYING(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v, *w;

  if (options.verbose)
    Error(ERR_NETWORK_SERVER,
	  "client %d (%s) starts game [level %d from leveldir %d (%s)]",
	  player->number, player->player_name,
	  (buffer[2] << 8) + buffer[3],
	  (buffer[4] << 8) + buffer[5],
	  &buffer[10]);

  for (w = first_player; w; w = w->next)
    if (w->introduced)
      w->active = 1;

  /* reset frame counter */
  ServerFrameCounter = 0;

  Error(ERR_NETWORK_SERVER, "resetting ServerFrameCounter to 0");

  /* reset player actions */
  for (v = first_player; v; v = v->next)
  {
    v->action = 0;
    v->action_received = FALSE;
  }

  broadcast(NULL, 10 + strlen((char *)&buffer[10])+1, 0);
}

static void Handle_OP_PAUSE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) pauses game",
	  player->number, player->player_name);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_CONTINUE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) continues game",
	  player->number, player->player_name);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_STOP_PLAYING(struct NetworkServerPlayerInfo *player)
{
  int cause_for_stopping = buffer[2];

  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) stops game [%d]",
	  player->number, player->player_name, cause_for_stopping);
  broadcast(NULL, 3, 0);
}

static void Handle_OP_MOVE_PLAYER(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  int last_client_nr = 0;
  int i;

  /* store player action */
  for (v = first_player; v; v = v->next)
  {
    if (v->number == player->number)
    {
      v->action = buffer[2];
      v->action_received = TRUE;
    }
  }

  /* check if server received action from each player */
  for (v = first_player; v; v = v->next)
  {
    if (!v->action_received)
      return;

    if (v->number > last_client_nr)
      last_client_nr = v->number;
  }

  /* initialize all player actions to zero */
  for (i = 0; i < last_client_nr; i++)
    buffer[6 + i] = 0;

  /* broadcast actions of all players to all players */
  for (v = first_player; v; v = v->next)
  {
    buffer[6 + v->number-1] = v->action;
    v->action = 0;
    v->action_received = FALSE;
  }

  buffer[2] = (unsigned char)((ServerFrameCounter >> 24) & 0xff);
  buffer[3] = (unsigned char)((ServerFrameCounter >> 16) & 0xff);
  buffer[4] = (unsigned char)((ServerFrameCounter >>  8) & 0xff);
  buffer[5] = (unsigned char)((ServerFrameCounter >>  0) & 0xff);

  broadcast(NULL, 6 + last_client_nr, 0);

  ServerFrameCounter++;
}

/* the following is not used for a standalone server;
   the pointer points to an integer containing the port-number */
int NetworkServerThread(void *ptr)
{
  NetworkServer(*((int *) ptr), 0);

  /* should never be reached */
  return 0;
}

void NetworkServer(int port, int serveronly)
{
  int sl;
  struct NetworkServerPlayerInfo *player;
  int r; 
  unsigned int len;
  IPaddress ip;

#if defined(PLATFORM_UNIX) && !defined(PLATFORM_NEXT)
  struct sigaction sact;
#endif

  if (port == 0)
    port = DEFAULT_SERVER_PORT;

  if (!serveronly)
    onceonly = 1;

#if defined(PLATFORM_UNIX)
#if defined(PLATFORM_NEXT)
  signal(SIGPIPE, SIG_IGN);
#else
  sact.sa_handler = SIG_IGN;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sigaction(SIGPIPE, &sact, NULL);
#endif
#endif

  if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_ResolveHost() failed");

  lfd = SDLNet_TCP_Open(&ip);
  if (!lfd)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_Open() failed");

  fds = SDLNet_AllocSocketSet(MAX_PLAYERS+1);
  SDLNet_TCP_AddSocket(fds, lfd);

  if (options.verbose)
  {
    Error(ERR_NETWORK_SERVER, "started up, listening on port %d", port);
    Error(ERR_NETWORK_SERVER, "using protocol version %d.%d.%d",
	  PROTOCOL_VERSION_1, PROTOCOL_VERSION_2, PROTOCOL_VERSION_3);
  }

  while (1)
  {
    interrupt = 0;

    for (player = first_player; player; player = player->next)
      flushuser(player);

    if ((sl = SDLNet_CheckSockets(fds, 500000)) < 1)
    {
      Error(ERR_NETWORK_SERVER, "SDLNet_CheckSockets failed: %s",
	    SDLNet_GetError());
      perror("SDLNet_CheckSockets");
    }

    if (sl < 0)
      continue;
    
    if (sl == 0)
      continue;

    /* accept incoming connections */
    if (SDLNet_SocketReady(lfd))
    {
      TCPsocket newsock;

      newsock = SDLNet_TCP_Accept(lfd);

      if (newsock)
	AddPlayer(newsock);
    }

    player = first_player;

    do
    {
      if (SDLNet_SocketReady(player->fd))
      {
	/* read only 1 byte, because SDLNet blocks when we want more than is
	   in the buffer */
	r = SDLNet_TCP_Recv(player->fd, player->readbuffer + player->nread, 1);

	if (r <= 0)
	{
	  if (options.verbose)
	    Error(ERR_NETWORK_SERVER, "EOF from client %d (%s)",
		  player->number, player->player_name);
	  RemovePlayer(player);
	  interrupt = 1;
	  break;
	}
	player->nread += r;
	while (player->nread >= 4 && player->nread >= 4 + player->readbuffer[3])
	{
	  len = player->readbuffer[3];
	  if (player->readbuffer[0] || player->readbuffer[1] || player->readbuffer[2])
	  {
	    if (options.verbose)
	      Error(ERR_NETWORK_SERVER, "crap from client %d (%s)",
		    player->number, player->player_name);
	    RemovePlayer(player);
	    interrupt = 1;
	    break;
	  }
	  memcpy(buffer, &player->readbuffer[4], len);
	  player->nread -= 4 + len;
	  memmove(player->readbuffer, player->readbuffer + 4 + len, player->nread);

	  buffer[0] = player->number;
	  if (!player->introduced && buffer[1] != OP_PLAYER_NAME)
	  {
	    if (options.verbose)
	      Error(ERR_NETWORK_SERVER, "!(client %d)->introduced && buffer[1]==%d (expected OP_PLAYER_NAME)", buffer[0], buffer[1]);

	    RemovePlayer(player);
	    interrupt = 1;
	    break;
	  }

	  switch (buffer[1])
	  {
	    case OP_PLAYER_NAME:
	      Handle_OP_PLAYER_NAME(player, len);
	      break;

	    case OP_PROTOCOL_VERSION:
	      Handle_OP_PROTOCOL_VERSION(player, len);
	      break;

	    case OP_NUMBER_WANTED:
	      Handle_OP_NUMBER_WANTED(player);
	      break;

	    case OP_START_PLAYING:
	      Handle_OP_START_PLAYING(player);
	      break;

	    case OP_PAUSE_PLAYING:
	      Handle_OP_PAUSE_PLAYING(player);
	      break;

	    case OP_CONTINUE_PLAYING:
	      Handle_OP_CONTINUE_PLAYING(player);
	      break;

	    case OP_STOP_PLAYING:
	      Handle_OP_STOP_PLAYING(player);
	      break;

	    case OP_MOVE_PLAYER:
	      Handle_OP_MOVE_PLAYER(player);
	      break;

	    case OP_BROADCAST_MESSAGE:
	      buffer[len] = '\0';
	      if (options.verbose)
		Error(ERR_NETWORK_SERVER, "client %d (%s) sends message: %s",
		      player->number, player->player_name, &buffer[2]);
	      broadcast(player, len, 0);
	      break;
	    
	    default:
	      if (options.verbose)
		Error(ERR_NETWORK_SERVER,
		      "unknown opcode %d from client %d (%s)",
		      buffer[0], player->number, player->player_name);
	  }
	}
      }

      if (player && !interrupt)
	player = player->next;
    }
    while (player && !interrupt);
  }
}

#endif /* NETWORK_AVALIABLE */
