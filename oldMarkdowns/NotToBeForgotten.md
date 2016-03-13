##These are things to not forget
There are certain features that might not be implemented in early versions of the language, 
that I still might want to add someday.
This file contains both possible features and small things that would be cool.

#### Important qirks of language
* In c++, the delete keyword only works on polymoripic pointers if the destructor is virtual!
* There is a difference between delete and delete[]
* The tokens for templates are **lower** **identifier** **greater**

#### Small things that would be cool
* Have scopes with only one statement log a "Suggestion" for you to remove the scope.
* Add **sizeof** and **typeof**

#### Possible features down the line
* The **elselse** keyword. When a set of "if"-"else if"-"else" has one of the ifs be true, and the else is skipped, run the elselse after the else instead.
* Call **elselse** something cooler
* The **retry** keyword. Just like *continue;*, only that the iterator isn't iterated