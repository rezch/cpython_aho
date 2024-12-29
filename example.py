import daho

aho = daho.DAho(5)  # aho buckets count

aho.insert('hello', 2)  # add 2 strings 'hello' to the aho set
aho.insert('abc')  # add one string 'abc' to the aho set
print(aho.request(text='hello world aabc'))  # count the inclusions of strings from the aho set in a text
#> 3

aho.insert('a')
print(aho.request('hello world aabc'))
#> 5

aho.insert('a', -1)  # add "-1" string 'hello' to the aho set, еру same as delete 1 occurrence
print(aho.request('hello world aabc'))
#> 3

print(aho.buckets)
#> 5

aho.resize(15)  # changes the aho buckets count

print(aho.buckets)
#> 15
