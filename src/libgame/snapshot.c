// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// snapshot.c
// ============================================================================

#include "snapshot.h"


#ifdef DEBUG
#define DEBUG_SNAPSHOTS			0
#endif

static ListNode *snapshot_single = NULL;
static ListNode *snapshot_list = NULL;
static ListNode *snapshot_current = NULL;

static int num_snapshots = 0;
static int num_snapshot_buffers = 0;
static int num_snapshot_bytes = 0;
static int next_snapshot_key = 0;


// -----------------------------------------------------------------------------
// functions for handling buffers for a single snapshot
// -----------------------------------------------------------------------------

void SaveSnapshotBuffer(ListNode **snapshot_buffers, void *buffer, int size)
{
  struct SnapshotNodeInfo *bi =
    checked_calloc(sizeof(struct SnapshotNodeInfo));

  bi->buffer_orig = buffer;
  bi->buffer_copy = checked_malloc(size);
  bi->size = size;

  memcpy(bi->buffer_copy, buffer, size);

  addNodeToList(snapshot_buffers, NULL, bi);

  num_snapshot_buffers++;
  num_snapshot_bytes += size;
}

static void LoadSnapshotBuffer(struct SnapshotNodeInfo *bi)
{
  memcpy(bi->buffer_orig, bi->buffer_copy, bi->size);
}

void LoadSnapshotBuffers(ListNode *snapshot_buffers)
{
  while (snapshot_buffers != NULL)
  {
    LoadSnapshotBuffer((struct SnapshotNodeInfo *)snapshot_buffers->content);

    snapshot_buffers = snapshot_buffers->next;
  }
}

static void FreeSnapshotBuffer(void *bi_raw)
{
  struct SnapshotNodeInfo *bi = (struct SnapshotNodeInfo *)bi_raw;

  num_snapshot_buffers--;
  num_snapshot_bytes -= bi->size;

  checked_free(bi->buffer_copy);
  checked_free(bi);
}

void FreeSnapshotBuffers(ListNode *snapshot_buffers)
{
  while (snapshot_buffers != NULL)
    deleteNodeFromList(&snapshot_buffers, NULL, FreeSnapshotBuffer);
}

// -----------------------------------------------------------------------------
// functions for handling single shapshot or list of snapshots
// -----------------------------------------------------------------------------

static void FreeSnapshot(void *snapshot_buffers_ptr)
{
  FreeSnapshotBuffers(snapshot_buffers_ptr);
}

void FreeSnapshotSingle()
{
  FreeSnapshotBuffers(snapshot_single);

  snapshot_single = NULL;
}

static void FreeSnapshotList_UpToNode(ListNode *node)
{
  while (snapshot_list != node)
  {
#if DEBUG_SNAPSHOTS
    printf("::: FreeSnapshotList_*() [%s, %d, %d]\n",
	   snapshot_list->key, num_snapshot_buffers, num_snapshot_bytes);
#endif

    deleteNodeFromList(&snapshot_list, snapshot_list->key, FreeSnapshot);

    num_snapshots--;
    next_snapshot_key = (snapshot_list ? atoi(snapshot_list->key) + 1 : 0);
  }
}

void FreeSnapshotList()
{
#if DEBUG_SNAPSHOTS
  printf("::: FreeSnapshotList()\n");
#endif

  FreeSnapshotList_UpToNode(NULL);

  num_snapshots = 0;
  num_snapshot_buffers = 0;
  num_snapshot_bytes = 0;
  next_snapshot_key = 0;

  snapshot_current = NULL;
}

void ReduceSnapshotList()
{
#if DEBUG_SNAPSHOTS
  printf("::: (Reducing number of snapshots from %d ",
	 num_snapshots);
#endif

  // maximum number of snapshots exceeded -- thin out list of snapshots
  ListNode *node = snapshot_list;
  int num_snapshots_to_skip = num_snapshots / 10;

  // do not remove the newest snapshots from the list
  while (node && num_snapshots_to_skip--)
    node = node->next;

  // remove every second snapshot from the remaining list
  while (node)
  {
    // never delete the first list node (snapshot at game start)
    if (node->next == NULL)
      break;

    // in alternation, delete one node from the list ...
    deleteNodeFromList(&node, node->key, FreeSnapshot);
    num_snapshots--;

    // ... and keep one node (which always exists here)
    node = node->next;
  }

#if DEBUG_SNAPSHOTS
  printf("to %d.)\n", num_snapshots);

#if 0
  node = snapshot_list;
  while (node)
  {
    printf("::: key: %s\n", node->key);
    node = node->next;
  }
#endif
#endif
}

void SaveSnapshotSingle(ListNode *snapshot_buffers)
{
  if (snapshot_single)
    FreeSnapshotSingle();

  snapshot_single = snapshot_buffers;
}

void SaveSnapshotToList(ListNode *snapshot_buffers)
{
  if (snapshot_current != snapshot_list)
    FreeSnapshotList_UpToNode(snapshot_current);

#if DEBUG_SNAPSHOTS
  printf("::: SaveSnapshotToList() [%d] [%d snapshots, %d buffers, %d bytes]\n",
	 next_snapshot_key, num_snapshots,
	 num_snapshot_buffers, num_snapshot_bytes);
#endif

  addNodeToList(&snapshot_list, i_to_a(next_snapshot_key),
		snapshot_buffers);

  snapshot_current = snapshot_list;

  num_snapshots++;
  next_snapshot_key++;

  if (num_snapshot_bytes > setup.engine_snapshot_memory)
    ReduceSnapshotList();
}

boolean LoadSnapshotSingle()
{
  if (snapshot_single)
  {
    LoadSnapshotBuffers(snapshot_single);

    return TRUE;
  }

  return FALSE;
}

boolean LoadSnapshotFromList_Older(int steps)
{
  if (snapshot_current && snapshot_current->next)
  {
    while (snapshot_current->next && steps--)
      snapshot_current = snapshot_current->next;

    LoadSnapshotBuffers(snapshot_current->content);

#if DEBUG_SNAPSHOTS
    printf("::: LoadSnapshotFromList_Older() [%s]\n", snapshot_current->key);
#endif

    return TRUE;
  }

  return FALSE;
}

boolean LoadSnapshotFromList_Newer(int steps)
{
  if (snapshot_current && snapshot_current->prev)
  {
    while (snapshot_current->prev && steps--)
      snapshot_current = snapshot_current->prev;

    LoadSnapshotBuffers(snapshot_current->content);

#if DEBUG_SNAPSHOTS
    printf("::: LoadSnapshotFromList_Newer() [%s]\n", snapshot_current->key);
#endif

    return TRUE;
  }

  return FALSE;
}

boolean CheckSnapshotList()
{
  return (snapshot_list ? TRUE : FALSE);
}
