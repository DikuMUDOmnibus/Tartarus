io.write("Sum of a table's components:\n");
sum = 0
for i = 1, #foo do
    print(i, foo[i])
    sum = sum + foo[i]
end
io.write("Returning the table's sum to C\n");
return sum
