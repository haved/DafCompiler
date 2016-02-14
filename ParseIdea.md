##How parsing might be done. Written down do avoid later realizations of self incompetance
* The main file is turned into tokens.
* One goes through the main file tokens and finds imports and uses.
* Find the tokens for all those files.
* Go through those files and find imports and uses
* Do this for every import and use in the project
* All of the uses and imports are collected in order?
* Does that even work??
* Profit
