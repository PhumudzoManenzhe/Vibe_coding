num_subjects = int(input("How many subjects do you have? "))
total = 0

for i in range(num_subjects):
    mark = float(input(f"Enter mark for subject {i + 1}: "))
    total += mark

average = total / num_subjects

if average >= 75:
    grade = "A"
elif average >= 65:
    grade = "B"
elif average >= 50:
    grade = "C"
else:
    grade = "F"

print("\n--- Results ---")
print("Average:", average)
print("Grade:", grade)