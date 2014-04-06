#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "minunit.h"
#include "../src/gump_db.h"

int tests_run = 0;

/* Global Setup */
struct record {
  char last_name[20];
  char first_name[15];
  int age;
};

typedef struct record person;

/* ------ */

/* Helpers */
void create_file(char * file_name) {
  fclose(fopen(file_name, "w"));
}

bool str_eql(char * str, char * other_str) {
  return strcmp(str, other_str) == 0;
}

/* ------ */

static char * test_init_db() {
  person a_person;
  GumpDB db = gmp_init_DB("people.db", 100);

  mu_assert("file name", str_eql(db->file_name, "people.db"));
  mu_assert("size of data", db->size_of_data == 100);
  return 0;
}

static char * test_store() {
  create_file("people_test");

  /* Initialize DB */
  GumpDB db = gmp_init_DB("people_test", sizeof(person));

  /* Let's create a record */
  person person_to_store;
  strcpy(person_to_store.first_name, "John");
  strcpy(person_to_store.last_name, "Doe");
  person_to_store.age = 39;

  /* Now we store it in our DB */
  int result = gmp_store(db, &person_to_store);
  mu_assert("should store it in file's first position", result == 0);

  /* Ok, let's store the same data in next available position */
  result = gmp_store(db, &person_to_store);
  mu_assert("should store it in file's second position", result == 1);

  /* And the last one */
  result = gmp_store(db, &person_to_store);
  mu_assert("should store it in file's third position", result == 2);

  return 0;
}

static char * test_retrieve() {
  create_file("people_test");

  /* Initialize DB */
  GumpDB db = gmp_init_DB("people_test", sizeof(person));

  /* Let's create three different records and store them in our DB*/
  person john;
  strcpy(john.first_name, "John");
  strcpy(john.last_name, "Doe");
  john.age = 39;

  person jane;
  strcpy(jane.first_name, "Jane");
  strcpy(jane.last_name, "Black");
  jane.age = 41;

  person paul;
  strcpy(paul.first_name, "Paul");
  strcpy(paul.last_name, "McGregor");
  paul.age = 40;

  gmp_store(db, &john); /* id = 0 */
  gmp_store(db, &jane); /* id = 1 */
  gmp_store(db, &paul); /* id = 2 */

  /* Now let's fetch them by id */

  person retrieved;
  bool result = gmp_retrieve(db, 0, &retrieved);
  mu_assert("retrieve result", result);
  mu_assert("first person name", str_eql(retrieved.first_name, "John"));
  mu_assert("first person last name", str_eql(retrieved.last_name, "Doe"));
  mu_assert("first person age", retrieved.age == 39);

  /* Skip to the last record */
  result = gmp_retrieve(db, 2, &retrieved);
  mu_assert("retrieve result", result);
  mu_assert("last person name", str_eql(retrieved.first_name, "Paul"));
  mu_assert("last person last name", str_eql(retrieved.last_name, "McGregor"));
  mu_assert("last person age", retrieved.age == 40);

  /* And back to the second record */
  result = gmp_retrieve(db, 1, &retrieved);
  mu_assert("retrieve result", result);
  mu_assert("second person name", str_eql(retrieved.first_name, "Jane"));
  mu_assert("second person last name", str_eql(retrieved.last_name, "Black"));
  mu_assert("second person age", retrieved.age == 41);

  return 0;
}

static char * test_delete() {
  create_file("people_test");

  /* Initialize DB */
  GumpDB db = gmp_init_DB("people_test", sizeof(person));

  person john;
  strcpy(john.first_name, "John");

  gmp_store(db, &john);

  /* Let's make sure we can fetch it id */
  person retrieved;
  bool result = gmp_retrieve(db, 0, &retrieved);
  mu_assert("retrieve result", result);
  mu_assert("first person name", str_eql(retrieved.first_name, "John"));

  /* Now we delete the record in the DB */
  result = gmp_delete(db, 0);
  mu_assert("delete record with id = 0", result);

  /* We shouldn't be able to retrieve the result now */
  person not_retrieved;
  result = gmp_retrieve(db, 0, &not_retrieved);
  mu_assert("don't retrieve result", !result);

  /* We shouldn't be able to delete a deleted record */
  result = gmp_delete(db, 0);
  mu_assert("don't delete record with id = 0", !result);

  /* Neither a non existent record */
  result = gmp_delete(db, 1);
  mu_assert("don't delete record with id = 1", !result);

  return 0;
}

static char * all_tests() {
   mu_run_test(test_init_db);
   mu_run_test(test_store);
   mu_run_test(test_retrieve);
   mu_run_test(test_delete);
   return 0;
}

int main(int argc, char **argv) {
   char *result = all_tests();
   if (result != 0) {
       printf("Error: %s\n", result);
   }
   else {
       printf("ALL TESTS PASSED\n");
   }
   printf("Tests run: %d\n", tests_run);

   return result != 0;
}
