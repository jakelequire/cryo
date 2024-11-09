

This is what the IR should look like:

```
define void @testFunc() {
entry:
  %intTest = alloca %Int, align 8                     ; Allocate struct
  call void @"Int::constructor"(ptr %intTest, i32 42) ; Call constructor
  %value = getelementptr %Int, ptr %intTest, i32 0, i32 0 ; Get field ptr
  %value.load = load i32, ptr %value                  ; Load field value
  call void @printInt(i32 %value.load)               ; Call print
  ret void
}
```
