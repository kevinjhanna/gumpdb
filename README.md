# GumpDB

Simple, stupid and tiny DB to use in a school project.

> "He must be the stupidest son of a bitch alive, but he sure is fast!"

## Idea
GumpDB works similar to cloakrooms, where you leave your clothes and in exchange they
give you a number to later come back and fetch them.

GumpDB stores your records (C data structures) in a binary file and returns an ID
each time you store a record.

With that ID you can then look up the values of the record.

## Current functionality
* Insert a record - **O(n)**
* Retrieves a record by id (doesn't take it out) - **O(1)**

## Coming soon
* Delete a record - **O(1)**
* List records - **O(n)**
* Modify a record - **O(1)**


