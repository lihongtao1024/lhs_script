function test(x)
{
    return x
}

function test1()
{
    return {aa = {["1"] = 12}}
}

function fibonacci(x)
{
    if (x < 2)
    {
        return 1
    }

    return fibonacci(x - 1) + fibonacci(x - 2)
}

var t = {{4, aa = test1}, cc = {7, 99}, test="test1", 5.5, 89, aa="test11", [12] = 10, ["aa"] = 100, [test(55)] = 88, bb = 6 * 6 - 8, test, [test] = "ccc"}
print(t[0].aa().aa["1"] + t[55], "\n")
print(t[0].aa()["aa"]["1"] + t[55], "\n")
print(t[55], "\n")

for (var i = 0; i < 20; i = i + 1)
{
    print(fibonacci(i), " ")
}

print("\n")

var i = 0
while (true)
{
    if (i < 20)
    {
        print(fibonacci(i), " ")
        i = i + 1
        continue
    }

    break
}

print("\n")

i = 0
do
{
    print(fibonacci(i), " ")
    i = i + 1
} when (i < 20)

//print(t[1][2])
//print(t.test)
//print(t.aa)
//var tt = 4 + 5 * {} - -6
