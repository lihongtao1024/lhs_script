/*
for (var j = 0; j < 10; j = j + 1)
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
*/
/*
function test(aa)
{
    return aa
}
var aa = test(1) + 2 * 3 - -(test(4) + 5)
print(aa)

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
*/
function fibonacci(xx)
{
    if (xx < 2)
    {
        return 1
    }

    return fibonacci(xx - 1) + fibonacci(xx - 2)
}
