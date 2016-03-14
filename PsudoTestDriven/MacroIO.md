#####Macro Test 1:
```
#macro Ball {5}
printd(#Ball);
```
Evaluates to `printd(5);` The macro name is defined to **Ball**, and it's contents are **5**

##### Macro with parameter:
```
#macro ToString<#value>
{ #if #value == "0"
	0
  #else
  	#IntToString(value)
}
let a:=5;
printf("%s%s", #ToString<a>, #ToString<0>);
```
Evaluates to printf("%s%s", IntToString(a), 0); The macro name becomes ToString<#> and it's contents are as written.  
The <> may be replaced with any special characters, but they must both start and end the parameter lost. No Special characters inbetween (,except for # of cource).

