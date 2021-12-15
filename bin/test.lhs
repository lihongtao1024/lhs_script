

print(test_print("aaaa", 11, 33))

function test_print(x, y, z)
{
    y = y + 2
    print(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    return 6
}

function test_print_ex(x, y, z)
{
    print(x, y, z)
    return y
}
