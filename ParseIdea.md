##How parsing might be done
Written down do avoid later realizations of self incompetance
* The main file is turned into tokens.
* A main file automaticly uses itself. The source file uses the header after all.
* You start going through, Adding definitions along the way
* When you meet an #import:
 * Parse the file as includedFile
 * Add all the pub definitions of the includedFile to your imported defintions
* When you meet an #using:
 * Parse the file as includedFile
 * Add all the pub definitions of the includedFile to your used definitions
* The imports and uses are turned into definitions as well
* Inline code is never imported. It stays in it's file
  
#####How do includesFiles work?
* They have a list of pub definitions.
* When you parse a file it is first tokenized
* It goes through and adds pub definitions just as any other file
* If it meets #import, parse it as includedFile
 * If it's already parsing: Recursive importing is illegal
 * Add it's pub definitions to you
 
#### When adding definitions
Note to self: Should probably have an interface called "Type"
* If you are using a type, go through the list of imported definitions to find it.
* If it's a pointer, also go through the list of used definitions.
 * Add it to the list of used as pointer
  * This will be used to make class pre-definitions in the header
* Inside a scope, everyting is fair game. As long as it's defined somewhere

Will this work? I'd be so happy if it did. Right now it's 23:48 and I've got a social studies test for tomorrow. I feel prepared, but I'm still not sure what questions might be on the test. I'm sort of hoping I'll get back here someday, and wonder why I even worried. Either that or I do poorly on the test (by my standards, an A is poorly), and erase this line. The glory of git, though, it that it will never disapear. It will still be saved as a zlib compressed file with it's own hash as it's name, somewhere on my file system. Removing it would mean learning more of git, and I'm not to sure I want to ;)  
By now it's 23:52. I'm writing for no apparant reason, and my text is probably filled with spelling and grammatical errors. Will anyone ever even care? I dont know. Sometimes you just feel like writing for the sake of filling a blank document with text. In this case, it wasn't even blank. It already had clever ideas (at this point in time, they seem cunning). I'm just appending text onto the end of a document. Time passes by slowly when I write like this. I can't imagine my writing speed is any slower than if I had been give the task of copying a text from a piece of paper. That's how fast it's going.  
Now I've got 5 minutes to make the git commit of the day, to keep my github streak going. It's the small things in life, you know. ;)  
Right now I feel like writing a poem. Not that I'm good at it.  
Next to me, there are rings interwined.  
Four in one, the pattern's called. The most popular pattern in Europe.  
How is this a poem, you might ask?  
Well. Compared to the text above, I've given a lot more care to line breaks.  
Just a double tap away in Markdown.  
But you need to have a character there first.  
At least Gnome Builder requires that for it to highlight the line break.  
b  
b  
b  
b  
b  
b  
  
I'm not sure why I chose the letter b.  
I also left in some line containing only spaces.  
The previous line had four spaces following the full stop. This one has got none.
Even though many people like line breaks after the final line (I've written a lexer, I know why), I'm not going to. Instead I'm going to leave the end open (

#####Mate I just got so into writing I missed my commit
That's not gonna stop my streak, however!

That was easy, now I just have to set the time back to what it's supposed to be.  
Man, I like this keyboard
