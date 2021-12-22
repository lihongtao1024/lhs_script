
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

