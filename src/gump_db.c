#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "gump_db.h"

GumpDB gmp_init_DB(char * file_name, int size_of_data) {
  GumpDB db = malloc(sizeof(GumpDBStruct));

  if (db != NULL) {
    db->size_of_data = size_of_data;
    strcpy(db->file_name, file_name);
  }

  return db;
}

bool gmp_connect(GumpDB db) {
  db->file = fopen(db->file_name, "r+b");
  return db->file != NULL;
}

bool gmp_disconnect(GumpDB db) {
  return fclose(db->file) == 0;
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
  char ctrl[1];

  while (!found) {
    fseek(db->file, position * (db->size_of_data + 1), SEEK_SET);
    read_bytes = fread(&ctrl, 1, 1, db->file);

    if (ctrl[0] != SPOT_IN_USE || read_bytes == 0) {
      /* Go back one byte, since we have already advance the cursor */
      fseek(db->file, position * (db->size_of_data + 1), SEEK_SET);

      found = true;
      ctrl[0] = SPOT_IN_USE;
      if (fwrite(&ctrl, 1, 1, db->file) != 1){
        /* If we can't write ctrl the byte, we won't store the record. */
        return -1;
      }

      if (fwrite(r, db->size_of_data, 1, db->file) == db->size_of_data) {
        /* If we can't write the data let's try to mark the spot as empty */

        fseek(db->file, position * (db->size_of_data + 1), SEEK_SET);
        ctrl[0] = SPOT_EMPTY;

        /* And if we can't mark the spot as empty,
         * we return that the DB has been corrupted */
        return fwrite(&ctrl, 1, 1, db->file) == 1 ? -1 : -2;
      }

      return position;
    }

    position++;
  }
  return -1;
}

int gmp_store(GumpDB db, void * r) {
  if (!gmp_connect(db)) { return -1; }

  int result = _gmp_store(db, r);

  gmp_disconnect(db);
  return result;
}

bool _gmp_retrieve(GumpDB db, int position, void * r) {
  fseek(db->file, position * (db->size_of_data + 1), SEEK_SET);

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
  if (!gmp_connect(db)) { return false; }

  bool result = _gmp_retrieve(db, id, r);

  gmp_disconnect(db);
  return result;
}
