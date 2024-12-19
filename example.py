import daho

aho = daho.DAho(15)  # aho buckets count

aho.insert('hello', 1)
aho.insert('abc')
print(aho.request('hello world aabc'))  # 2

aho.insert('a')
print(aho.request('hello world aabc'))  # 4

aho.insert('a', -1)
print(aho.request('hello world aabc'))  # 2

print(aho.buckets)  # 15
