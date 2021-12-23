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
*/

var i = 100
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
