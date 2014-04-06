# GumpDB

> "He must be the stupidest son of a bitch alive, but he sure is fast!"

## Idea
GumpDB works similar to cloakrooms, where you leave your clothes and in exchange they
give you a number to later come back and fetch them.

GumpDB stores your records (C data structures) in a binary file and returns an ID
each time you store a record.

With that ID you can then look up the values of the record.

## Operations
### Current
* Insert a record - **O(n)**
* Retrieves a record by id (doesn't take it out) - **O(1)**
* Delete a record - **O(1)**

### Coming soon
* List records - **O(n)**
* Modify a record - **O(1)**

## Locks
For simplicity's sake, Insert, Delete and Modify operations lock the **whole file** with an exclusive lock.
And Retrieve and List operations place a shared lock on the file.

In this way, GumpDB enables many processes accesing records for reading while no other process is changing its content.

### Maybe, think about, in the future:
Insert, Modify and Delete operations will lock its particular record bytes with an exclusive lock in the file.
Retrieve will set a shared lock on its particular record bytes.
And List operation will set a shared lock on the whole file.
