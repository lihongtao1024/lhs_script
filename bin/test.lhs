print("hello world.\n")
main()

function main()
{
    for (var i = 0; i < 30; i = i + 1)
    {
        print(fibonacci(i), " ")
    }
}

function fibonacci(x)
{
    if (x < 2)
    {
        return 1
    }

    return fibonacci(x - 1) + fibonacci(x - 2)
}
