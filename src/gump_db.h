#ifndef GUMP_DB_H
  #define GUMP_DB_H

  #define SPOT_EMPTY 'o'
  #define SPOT_IN_USE 'x'

  typedef struct GumpDBStruct {
    char file_name[50];
    int size_of_data;
    FILE *file;
  } GumpDBStruct;

  typedef GumpDBStruct * GumpDB;

  GumpDB gmp_init_DB(char * file_name, int size_of_data);

  bool gmp_connect(GumpDB db);

  bool gmp_disconnect(GumpDB db);

  /*
   * Stores a record in the first available position.
   * Returns the position (ID) of where it is stored
   * inside the file.
   *
   * Returns -1 if it fails to do so.
   *
   * Returns -2 if it fails and the DB has been corrupted.
   *
   */
  int gmp_store(GumpDB db, void * r);

  /*
   * Retrieves a record for the given position (ID) of the DB.
   * Fills the structure that the r pointer points to.
   *
   * Returns true if it succeeds to do so.
   *
   * Returns false if it couldn't open the DB, or the record has
   * been deleted, or the record never existed, or it failed to
   * retrieve the whole data structure.
   *
   */
  bool gmp_retrieve(GumpDB db, int position, void * r);

  /*
   * Deletes the record with given id.
   * Returns true if it successfully deleted the record.
   * Returns false if there was nothing to delete, or if it
   * couldn't delete the record.
   *
   * TODO: Should put tell with errno, if the DB was corrupted
   * by not being able to set the ctrl char representing a
   * deleted the record.
   */
  bool gmp_delete(GumpDB db, int position);

#endif
