import hoge

print(hoge.__doc__)

print(hoge.add(1, 2))
print(hoge.What)

p = hoge.Pet("Molly")
print(p)
print(p.GetName())
print(p.name)
p.SetName("Charly")
print(p.GetName())
p.name = "Molly"
print(p.name)

b = hoge.Dog("Molly")
print(b.name)
print(b.Bark())

p2 = hoge.pet_store()
print(type(p2))
# p2.Back() # error

p3 = hoge.pet_store2()

print(type(p3))  # automatically downcast

p3.Bark()  # ok

# help(hoge.Pet)

p4 = hoge.Pet("Lucy", hoge.Pet.Kind.Cat)
print(p4.type)
print(int(p4.type))
