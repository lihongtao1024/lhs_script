var a
if (a)
{
    var c
    if (a)
    {
        var b
    }

    var b
}

set fn1 = getvalue(1), fn2 = getvalue()
set global0 = (-5-(1 + (-2))) * 3, global10
var global1, global11 = -6 % 3
var global2 = -global1 + 3.5 - (5 - ~1) * 6  + 2 / 2 - 1
var global3 = !global2+-1
--6
global0
var global4 = 1 * global0
global1 = 55
//global3 + 3 + 1.2 * 7 - 6
global2 = "abc"
//wer

var func1 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6

if (func1)
{
    var x = 0
}
else if (global1)
{
    var func2 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6
}
else if (global2)
{
    if (global3)
    {
        var func2 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6
    }

    var func2 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6
}
else
{
    var func2 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6
    if (func2)
    {

    }
    var func3 = getvalue(3, 5, global2 + 5 * -4 - (getvalue(1))) - 6
}

/*
pushc x

push local[1]
push 0
mov stack[-2], stack[-1]

push local[1]
push 5
l stack[-2], stack[-1]
jz 79
jmp 74

push local[1]
push local[1]
push 1
add stack[-2], stack[-1]
mov stack[-2], stack[-1]
jmp 61

push local[1]
call global[1], 1
jmp 67

popc

for (var i = 0; i < 5; i = i + 1)
{
    printf(i)
}

var i = 0
for (; i < 5; i = i + 1)
{

}

for (i = 1;;)

for (; i < 5;)
{

}

for (;;i = i + 2)
{

}

for (; ;)
{

}
*/

function (x, y, z)
{
    var x = 10

}
