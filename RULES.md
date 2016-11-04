## Rules for this repository
To make backtracking and bisecting using scripts as painless as possible, all these rules must be followed. Each rule is marked with a tag.
The tag points at the first commit where the rule is introduced.
If rule breaking problems are found, the tag must be moved.
The tag RULES\_SET marks the first commit where any rules are enforced.
Before this point, source folders are renamed, and unbuildable commits are pushed.  
At the time of RULES\_SET, all memory tests are showing no problems.

####Rules:
as of RULES_SET
1. The name of a source directory can't change
2. The name of a build directory may change
3. All commited code changes must be compilable, and run all its tests without errors
