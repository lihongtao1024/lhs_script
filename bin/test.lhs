/*
for (var j = 0; j < 10000; j = j + 1)
{
    if (j > 1)
    {
        break
    }

    for (var i = 0; i < 100; i = i + 1)
    {
        print(i)
        if (i & 1)
        {
            continue
        }

        if (i > 50)
        {
            break
        }
    }
}

var i = 100
while (i)
{
    print(i)
    if (i > 100)
    {
        break
    }

    if (i == 50)
    {
        i = 200
        continue
    }

    i = i - 1
}


i = 100
do
{
    i = i - 1
    if (i < 50)
    {
        continue
    }

    print(i)
    break
} until (i > 0)

function test(aa)
{
    print(aa)
}

print(i)


function test(aa)
{
    return aa
}
var aa = (-test(7) - 2) * -5 + test(1) + 2 * 3 - -(test(4) + 5)
//aa = 61
//aa = -9 + 6 - -9
print("\n", aa, "\n")

set d = 8 + -(5 - -7) - 6
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

function fibonacci(xx)
{
    if (xx < 2)
    {
        return 1
    }

    return fibonacci(xx - 1) + fibonacci(xx - 2)
}


for (var ii = 0; ii < 10; ii = ii + 1)
{
    print("aa\n")
    print(fibonacci(ii))
    print("aa\n")
}

var aaa = 1 + 2 * 3 - -(4 + 5)

if (a + 1)
{

}
*/
var a1 = 1, a2 = 3
var b1 = a2 > a1 && a1 >= a1
var b2 = 3 > 1 && 1 >= 1
print(b1, b2)