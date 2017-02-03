## Rules for this repository
To make backtracking and bisecting using scripts as painless as possible, all these rules must be followed. Each rule is marked with a tag.
The tag points at the first commit where the rule is introduced.
If rule breaking problems are found, the tag must be moved.
The tag RULES\_SET marks the first commit where any rules are enforced.
Before this point, source folders are renamed, and unbuildable commits are pushed.  
At the time of RULES\_SET, all memory tests are showing no problems.

####Rules:
as of RULES\_SET
1. The name of a source directory can't change
2. The name of a build directory may change
3. All commited code changes must be compilable, and run all its tests without errors
as of TEST\_SCRIPTS\_ADDED\_1
1. CompilerTests and its future sub-folders are for testing
3. You have got RunTests.py MakeCallgraph.py and CompileToTest.py
2. No test script functionality is removed
3. All tests must pass in a commit, including memory tests
4. .daf and .daf.test are tests
