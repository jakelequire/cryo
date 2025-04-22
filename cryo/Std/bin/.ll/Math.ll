; ModuleID = 'Math'
source_filename = "Math"

%class.Math = type {}

@Math = external global %class.Math

; Function Attrs: nounwind
define i32 @Math.sqrt(i32 %value) #0 {
entry:
  ret i32 0
}

; Function Attrs: nounwind
define i32 @Math.pow(i32 %exponent, i32 %0) #0 {
entry:
  ret i32 0
}

; Function Attrs: nounwind
define i32 @Math.abs(i32 %value) #0 {
entry:
  ret i32 0
}

; Function Attrs: nounwind
define i32 @Math.sin(i32 %value) #0 {
entry:
  ret i32 0
}

attributes #0 = { nounwind }
