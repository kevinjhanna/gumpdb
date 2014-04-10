#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "locks.h"
#include "gump_db.h"

GumpDB gmp_init_DB(char * file_name, int size_of_data) {
  GumpDB db = malloc(sizeof(GumpDBStruct));

  if (db != NULL) {
    db->size_of_data = size_of_data;
    strcpy(db->file_name, file_name);
  }

  return db;
}

void _gmp_goto_id(GumpDB db, int id) {
  fseek(db->file, id * (db->size_of_data + 1), SEEK_SET);
}

bool _gmp_connect(GumpDB db) {
  db->file = fopen(db->file_name, "r+b");
  return db->file != NULL;
}

bool _gmp_disconnect(GumpDB db) {
  return fclose(db->file) == 0;
}

/*
 * Sets a shared lock on the DB for reading.
 * If a conflicting lock is held on the DB, it waits for that lock
 * to be released.
 *
 * Since we set a lock for the whole file, Position parameter
 * is a dummy for now.
 */

bool _gmp_set_shared_lock(GumpDB db, int position) {
  return set_lock(fileno(db->file), F_SETLKW, F_RDLCK, 0, SEEK_SET, 0);
}

/*
 * Sets an exclusive lock on the DB for writing.
 * If a conflicting lock is held on the DB, it waits for that lock
 * to be released.
 *
 * Since we set a lock for the whole file, Position parameter
 * is a dummy for now.
 */

bool _gmp_set_exclusive_lock(GumpDB db, int position) {
  return set_lock(fileno(db->file), F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);
}

/*
 *  Think about our binary file that holds our DB as a garage
 *  with marked spots on the floor to leave fixed-size 'boxes'.
 *  And this spots are numbered from 0, upto the last box.
 *
 *  So, when we have a new box to store, we will start from the
 *  first spot, upto the last one, and see if there is any empty spot
 *  to leave our box.
 *
 *  Now, when we no longer want to store a box, instead of taking it out
 *  and doing such hard work, we can leave it there, but we mark it as if
 *  it wasn't there. Thus, when someone stores a new box, he will think of
 *  that spot where the box lies as an empty spot.
 *
 *  Let's get technical :)
 *  The contents of these boxes are bytes. And we can mark a box as 'empty'
 *  or 'in use' in one byte, but we can't touch the bytes from the box.
 *  As a result, we will have to add 1 byte to the spots where we leave
 *  the boxes.
 */

int _gmp_store(GumpDB db, void * r) {
  int position = 0;
  int read_bytes;
  bool found = false;
  char ctrl_char;

  while (!found) {
    _gmp_goto_id(db, position);

    read_bytes = fread(&ctrl_char, 1, 1, db->file);

    if (ctrl_char != SPOT_IN_USE || read_bytes == 0) {
      /* Go back one byte, since we have already advance the cursor */
      _gmp_goto_id(db, position);

      found = true;
      ctrl_char = SPOT_IN_USE;
      if (fwrite(&ctrl_char, 1, 1, db->file) != 1){
        /* If we can't write ctrl the byte, we won't store the record. */
        return -1;
      }

      if (fwrite(r, db->size_of_data, 1, db->file) == db->size_of_data) {
        /* If we can't write the data let's try to mark the spot as empty */

        _gmp_goto_id(db, position);
        ctrl_char = SPOT_EMPTY;

        /* And if we can't mark the spot as empty,
         * we return that the DB has been corrupted */
        return fwrite(&ctrl_char, 1, 1, db->file) == 1 ? -1 : -2;
      }

      return position;
    }

    position++;
  }
  return -1;
}

int gmp_store(GumpDB db, void * r) {
  if (!_gmp_connect(db)) { return -1; }

  _gmp_set_exclusive_lock(db, -1);
  int result = _gmp_store(db, r);

  _gmp_disconnect(db);
  return result;
}

bool _gmp_retrieve(GumpDB db, int position, void * r) {
  _gmp_goto_id(db, position);

  char ctrl_char;
  int result = fread(&ctrl_char, 1, 1, db->file);

  if (result != 1 || ctrl_char == SPOT_EMPTY) {
    /* Make sure we can read the first byte
     * and also that the byte does not represent the empty spot control char.
     */
    return false;
  }

  return fread(r, db->size_of_data, 1, db->file) == 1;
}

bool gmp_retrieve(GumpDB db, int id, void * r) {
  if (!_gmp_connect(db)) { return false; }

  _gmp_set_shared_lock(db, id);

  bool result = _gmp_retrieve(db, id, r);

  _gmp_disconnect(db);
  return result;
}

bool _gmp_delete(GumpDB db, int position) {
  _gmp_goto_id(db, position);

  char ctrl_char;
  int result = fread(&ctrl_char, 1, 1, db->file);

  if (result != 1 || ctrl_char == SPOT_EMPTY) {
    /* Make sure we can read the first byte
     * and also that the byte does not represent the empty spot control char.
     */
    return false;
  }

  /* Go back one byte, since we have already advance the cursor */
  _gmp_goto_id(db, position);

  ctrl_char = SPOT_EMPTY;
  if (fwrite(&ctrl_char, 1, 1, db->file) != 1){
    /* TODO: If we can't write the ctrl the byte we should set errno */
    return false;
  }

  return true;
}

bool gmp_delete(GumpDB db, int id) {
  if (!_gmp_connect(db)) { return false; }

  _gmp_set_exclusive_lock(db, id);

  bool result = _gmp_delete(db, id);

  _gmp_disconnect(db);
  return result;
}

bool _gmp_list(GumpDB db, void ** rs, int * count) {
  fseek(db->file, 0, SEEK_END);
  int file_size = ftell(db->file);
  fseek(db->file, 0, SEEK_SET);

  int positions = file_size / (db->size_of_data + 1);

  void **list;
  /*
   * We may be asking for more memory than we actually need.
   * That's because positions >= count
   */
  list = malloc(positions * sizeof(*list));

  char ctrl_char;
  int result;
  *count = 0;

  void * throw_away = malloc(db->size_of_data);

  for (int i = 0; i < positions; i++)
  {
      if(fread(&ctrl_char, 1, 1, db->file) != 1) {
       /* We couldn't read from the DB. TODO: Set errno.  */
        return false;
      }

      if (ctrl_char == SPOT_IN_USE) {
        list[*count] = malloc(db->size_of_data);

        if (fread(list[*count], db->size_of_data, 1, db->file) != 1) {
         /* We couldn't read from the DB. TODO: Set errno.  */
          return false;
        }
        (*count)++;
      } else {
        /* Just to move the cursor */
        fread(throw_away, db->size_of_data, 1, db->file);
      }
  }

  free(throw_away);

  *rs = list;

  return true;
}

bool gmp_list(GumpDB db, void * rs, int * count) {
  if (!_gmp_connect(db)) { return false; }

  _gmp_set_shared_lock(db, -1);

  bool result = _gmp_list(db, rs, count);

  _gmp_disconnect(db);
  return result;
}
