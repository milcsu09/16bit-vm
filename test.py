
def move(position, velocity, margin):
    if position < velocity:
        velocity = position
    return velocity

for i in range(1, 10):
    print(i, '=>', move(i, 5, 2))

