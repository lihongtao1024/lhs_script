/*set d = 8 + -(5 - -7) - 6
//-10
set c = 4 / 8 * -(5 - -7) - 6
//-12
set b = 8 * -(5 - -7) - 6
//-102
set a = -(6)
//-6
set z = -(5 - -7)
//-12
set y = -(5 + 3)
//-8
set x = -5 + 6 * 7 - 4 / 8 * -(5 - -7) - 6
//37
set k = 9
//9
set l = -(-(-(1*3-2)-6))
//-7

print(d)
print(c)
print(b)
print(a)
print(z)
print(y)
print(x)
print(k)
print(l)
*/
/*
for (var i = 0; i < 10; i = i + 1)
{
    print(i)
}
*/
/*
push 0
movs l[?]

push l[?]
push 10
less
jz 54   //finish
jmp 50  //block

push l[?]
push 1
add
movs l[?]
jmp 38  //condition

push l[?]
call g[?], 1, -1
jmp 44  //iterate

exit
*/

function test(x, y, z)
{
    var a = 6, b = 7, c = 8
    x = x + a
    y = y + b
    z = z + c
    return x + y + z
}

print(test(1, 2, 3))