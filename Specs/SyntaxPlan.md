### Syntax plan
This is the plan for the daf syntax. It is subject to change.

#### Modules and imports
###### Imports:
The file is written as **folder.folder.filename** without the daf extention. Relative to include paths. (and not current file)
When a module is imported and the name is kept, **module.function()** is required. When the name is removed, only **function()** is required.
If a file contains definitions outside of modules, they will allways be included in the import.
```
import <file>;    //import every module from the file, keeping their names
import <module>, <module> from <file>;    //imports the contents of the modules named, removing the names
import <module> as<name> from <file>;    //imports only the module named, with a custom name
import <module> as <is> from <file>;    //import the module keeping it's name
import <module>, <module> as are from <file>;    //import multiple modules, all keeping their names
import <*> from <file>;    //import everything from the file, without module names
import <module> from <this>;    //import a module from the same file (only when the file is being imported(?) )
```

###### Modules:
```
module String {
    def strlen:(&char):int;
};

module Other {
    import String from this;
    def something:(&mut char,&char,&char);
}
```
